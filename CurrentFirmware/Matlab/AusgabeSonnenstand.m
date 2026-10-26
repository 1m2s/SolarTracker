%
% Gebe den aktuellen Sonnenstand an die PV-Nachführung aus
%
clear all
% master und slave konfigurieren
ar = arduino('/dev/cu.usbserial-AC013FMP','Nano3','Libraries','I2C');     % COM# über Geräte-Manager herausfinden
slave = device(ar,'I2CAddress','0x19','bitrate',100000);

% Standort angeben
Laenge= 9.66;
Breite= 47.81;

while 1
[c,tf] = clock;
Jahr  = c(1);
Monat = c(2);
Tag   = c(3);
Stunde = c(4);
Minute = c(5);

[a,g] = SonnenStand(Breite,Laenge,Tag,Monat,Stunde,Minute,tf)

% Azimut/2, da nur 0-255 ausgegeben werden kann. Wird vom slave wieder
% verdoppelt.
a = a / 2;

% Datenformat anpassen, um Werte über I2C-Bus ausgeben zu können.
a = uint8(a);
g = uint8(g);

% Werte ausgeben, dabei Registernummer (254/255) angeben, damit der slave
% die Werte zuordnen kann.
writeRegister(slave,254,a,'uint8')
writeRegister(slave,255,g,'uint8')
pause(30)
end