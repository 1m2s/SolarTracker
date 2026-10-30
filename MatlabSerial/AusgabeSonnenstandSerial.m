%
% AusgabeSonnenstandSerial.m
% Sends the sun position to the left Nano as text over USB (MATLAB mode).
% Same idea as CurrentFirmware/Matlab/AusgabeSonnenstand.m,
% but for our firmware (AutoFirmware): no arduino(), no I2C from MATLAB.
% The Nano does targetBytes + I2C itself, so it also parks at night.
%
% How to use:
%   1) Tracker switch on MATLAB.
%   2) Close the PlatformIO monitor (only one program can use the port).
%   3) Set "port" below. Find it with: serialportlist("available")
%   4) Run this script. Stop with Ctrl+C, then type: clear s
%
% Needs MATLAB R2019b or newer (serialport).
% Never run the old AusgabeSonnenstand.m on this Nano:
% arduino() would replace our firmware.
%

clear                                   % also closes an old port "s"

% ---------- settings ----------
port   = "/dev/cu.wchusbserial1140";    % changes with the USB socket
Laenge = 9.66;                          % Standort (same as the original)
Breite = 47.81;

% Use the original sun functions, unchanged
here = fileparts(mfilename("fullpath"));
addpath(fullfile(here, "..", "CurrentFirmware", "Matlab"));

% ---------- connect ----------
s = serialport(port, 115200);           % same speed as the Nano
configureTerminator(s, "CR/LF", "LF");  % Nano answers with "\r\n", we send "\n"
s.Timeout = 2;                          % wait max 2 s for one answer line
pause(2)                                % opening the port restarts the Nano
printNano(s)                            % shows the Nano's start text

% ---------- send every 30 s (like the original) ----------
while true
    [c, tf] = clock;                    % Mac time + summer time flag
    [a, g] = SonnenStand(Breite, Laenge, c(3), c(2), c(4), c(5), tf);
    a = real(a);                        % near noon acos can give a complex number
    g = real(g);
    msg = sprintf("%.4f %.4f", a, g);   % e.g. "245.4100 3.8600"
    writeline(s, msg);                  % sends msg + "\n"
    fprintf("%02d:%02d:%02d  sent: %s\n", c(4), c(5), floor(c(6)), msg);
    t0 = tic;                           % wait 30 s, but show Nano messages right away
    while toc(t0) < 30
        pause(0.2)
        printNano(s)
    end
end

% Notes
% 1) Why text and not arduino()?
%    arduino() uploads its own firmware to the Nano. Then our firmware
%    (AUTO mode, parking, switch, LED) is gone. Text over USB keeps it.
%
% 2) The Nano restarts when MATLAB opens the port (DTR line).
%    That is why we wait 2 s. The start text "Hybrid firmware ..."
%    in the output proves the restart.
%
% 3) real(a): near noon, Azimut.m can call acos with a value a tiny bit
%    above 1. MATLAB then gives a complex number, e.g. 0 + 0.0004i.
%    The real part is 0, the same as the clamp in our C++ code.
%    Check: acos(1.0000001)
%
% 4) sprintf always uses a dot (245.41), never a comma.
%    The Nano only accepts the dot.
%
% 5) If the switch is on AUTO, the Nano answers
%    "ignored: switch on AUTO". Nothing moves. The switch always wins.
%
% 6) While waiting the 30 s, the script checks every 0.2 s for
%    messages from the Nano. So a switch flip shows up right away.
%
% 7) Stop: Ctrl+C. The Nano keeps the last position.
%    After 180 s without a line, its LED blinks fast (timeout).
%    Type "clear s" to free the port for PlatformIO.

% Prints every line the Nano has sent so far
function printNano(s)
    while s.NumBytesAvailable > 0
        fprintf("    Nano: %s\n", readline(s));
    end
end