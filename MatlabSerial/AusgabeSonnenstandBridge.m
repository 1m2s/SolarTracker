%
% AusgabeSonnenstandBridge.m
% Sends the sun position to the left Nano as text (MATLAB mode),
% through the dashboard's bridge instead of the USB port.
% Same loop as AusgabeSonnenstandSerial.m, only the connection is different.
% Use this one while Dashboard/bridge.py is running: the bridge holds the
% USB port, so MATLAB and the dashboard can talk to the Nano together.
%
% How to use:
%   1) Start the bridge in a terminal, inside Dashboard/:
%        python bridge.py /dev/cu.wchusbserial...
%   2) Tracker switch on MATLAB.
%   3) Run this script. Stop with Ctrl+C, then type: clear s
%
% Needs MATLAB R2020b or newer (readline and writeline on tcpclient).
% Without the bridge use AusgabeSonnenstandSerial.m.
% Never run the old AusgabeSonnenstand.m on this Nano:
% arduino() would replace our firmware.
%

clear                                   % also closes an old connection "s"

% ---------- settings ----------
host   = "127.0.0.1";                   % this laptop, as a number (see note 2)
port   = 5555;                          % TCP_PORT in Dashboard/bridge.py
Laenge = 9.66;                          % Standort (same as the original)
Breite = 47.81;

% Use the original sun functions, unchanged
here = fileparts(mfilename("fullpath"));
addpath(fullfile(here, "..", "CurrentFirmware", "Matlab"));

% ---------- connect ----------
try
    s = tcpclient(host, port);          % the Nano does NOT restart (see note 3)
catch connectError
    error("Could not connect to the bridge at %s:%d. Is bridge.py running?\n(MATLAB said: %s)", ...
          host, port, connectError.message);
end
configureTerminator(s, "CR/LF", "LF");  % Nano answers with "\r\n", we send "\n"
s.Timeout = 2;                          % wait max 2 s for one answer line
fprintf("Connected to the bridge (%s:%d)\n", host, port);

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
% 1) Why the bridge?
%    Only one program can open the USB port. bridge.py opens it once and
%    passes the lines on: every line from the Nano goes to all connected
%    programs (dashboard, this script), every line we send goes to the Nano.
%
% 2) Why "127.0.0.1" and not "localhost"?
%    MATLAB turns a name like "localhost" into an IPv6 address.
%    The bridge only listens on the IPv4 address 127.0.0.1.
%
% 3) The Nano does not restart when this script connects. Only opening
%    the USB port restarts it, and the bridge already did that.
%    So there is no pause(2) and no start text here, and the start
%    sequence does not run again.
%
% 4) Through the bridge a telemetry line ("T:...") arrives every second.
%    printNano skips them. They are for the dashboard.
%
% 5) The Nano's answer to each line also shows up in the dashboard,
%    in the Messages box. SUN and SEL in the Values row are what we sent.
%
% 6) real(a), the dot in sprintf, "ignored: switch on AUTO" and the
%    0.2 s check are the same as in AusgabeSonnenstandSerial.m
%    (notes 3 to 6 there).
%
% 7) Stop: Ctrl+C. The Nano keeps the last position.
%    After 180 s without a line, its LED blinks fast (timeout) and the
%    dashboard shows the problem line.
%    Type "clear s" to close the connection. The bridge keeps running.

% Prints every line the Nano has sent so far.
% Lines starting with "T:" are telemetry for the dashboard and are skipped.
function printNano(s)
    while s.NumBytesAvailable > 0
        reply = readline(s);
        if ~startsWith(reply, "T:")
            fprintf("    Nano: %s\n", reply);
        end
    end
end