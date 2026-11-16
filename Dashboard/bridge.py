"""Bridge between the left Nano (USB serial) and programs on this laptop (TCP).

Opens the serial port once and keeps it open. Every full line the Nano prints
is sent to all connected programs. Every full line a program sends is passed
on to the Nano. Only reachable from this laptop (127.0.0.1).

Run:   python bridge.py /dev/cu.wchusbserial1140
Stop:  Ctrl+C
"""

import os
import socket
import sys
import threading

import serial
from serial.tools import list_ports

HOST = '127.0.0.1'
TCP_PORT = 5555
BAUD = 115200

clients = []
clients_lock = threading.Lock()
serial_write_lock = threading.Lock()


def send_to_clients(line: bytes) -> None:
    """Send one Nano line to every connected program; drop the ones that left."""
    with clients_lock:
        for client in list(clients):
            try:
                client.sendall(line)
            except OSError:
                clients.remove(client)
                client.close()


def read_nano(nano: serial.Serial) -> None:
    """Collect bytes from the Nano into lines and pass each line on."""
    buffer = b''
    while True:
        try:
            buffer += nano.read(nano.in_waiting or 1)
        except (serial.SerialException, OSError):
            print('bridge: lost the Nano (USB unplugged?) -> stopped')
            os._exit(1)
        while b'\n' in buffer:
            line, buffer = buffer.split(b'\n', 1)
            line += b'\n'
            print('Nano:', line.decode(errors='replace').rstrip())
            send_to_clients(line)


def read_client(client: socket.socket, nano: serial.Serial) -> None:
    """Collect bytes from one program into lines and pass each line to the Nano."""
    buffer = b''
    try:
        while True:
            data = client.recv(1024)
            if not data:
                break
            buffer += data
            while b'\n' in buffer:
                line, buffer = buffer.split(b'\n', 1)
                with serial_write_lock:
                    nano.write(line + b'\n')
    except OSError:
        pass
    with clients_lock:
        if client in clients:
            clients.remove(client)
    client.close()
    print('bridge: a program disconnected')


def main() -> None:
    if len(sys.argv) != 2:
        print('usage: python bridge.py <serial port>')
        print('ports found:')
        for port in list_ports.comports():
            print('  ', port.device)
        sys.exit(1)

    try:
        nano = serial.Serial(sys.argv[1], BAUD, timeout=0.2, exclusive=True)
    except serial.SerialException as error:
        print('bridge: cannot open the port:', error)
        sys.exit(1)
    print(f'bridge: opened {sys.argv[1]} (the Nano may restart now)')
    threading.Thread(target=read_nano, args=(nano,), daemon=True).start()

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, TCP_PORT))
    server.listen()
    print(f'bridge: waiting for programs on {HOST}:{TCP_PORT}')

    try:
        while True:
            client, _ = server.accept()
            with clients_lock:
                clients.append(client)
            print('bridge: a program connected')
            threading.Thread(target=read_client, args=(client, nano), daemon=True).start()
    except KeyboardInterrupt:
        print('\nbridge: stopped')
    finally:
        server.close()



if __name__ == '__main__':
    main()