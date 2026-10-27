% generate_reference.m
% Prints the expected values for test/test_sonnenstand/test_main.cpp.
% Uses the ORIGINAL MATLAB files in CurrentFirmware/Matlab (not edited).
% Run it in MATLAB: cd into AutoFirmware/test, then type generate_reference

here = fileparts(mfilename('fullpath'));
addpath(fullfile(here, '..', '..', 'CurrentFirmware', 'Matlab'));
format long

Breite = 47.81;
Laenge = 9.66;

fprintf('--- small functions ---\n');
fprintf('grad2rad(180)              = %.7f\n', grad2rad(180));
fprintf('rad2grad(1)                = %.7f\n', rad2grad(1));
fprintf('JahresTag(1,1)             = %d\n',   JahresTag(1,1));
fprintf('JahresTag(6,21)            = %d\n',   JahresTag(6,21));
fprintf('JahresTag(10,27)           = %d\n',   JahresTag(10,27));
fprintf('JahresTag(12,21)           = %d\n',   JahresTag(12,21));
fprintf('MEZ(12,0,0)                = %d\n',   MEZ(12,0,0));
fprintf('MEZ(13,0,1)                = %d\n',   MEZ(13,0,1));
fprintf('MOZ(720,Laenge)            = %.4f\n', MOZ(720,Laenge));
fprintf('StundenWinkel(720)         = %.4f\n', StundenWinkel(720));
fprintf('StundenWinkel(600)         = %.4f\n', StundenWinkel(600));
fprintf('Zeitgleichung(0)           = %.4f\n', Zeitgleichung(0));
fprintf('Zeitgleichung(300/365*360) = %.4f\n', Zeitgleichung(300/365*360));
fprintf('Deklination(172)           = %.4f\n', Deklination(172));
fprintf('Deklination(355)           = %.4f\n', Deklination(355));
fprintf('Deklination(300)           = %.4f\n', Deklination(300));
fprintf('HoehenWinkel(0,Breite,0)   = %.4f\n', HoehenWinkel(0,Breite,0));
fprintf('Azimut(172,Breite,720)     = %.4f\n', real(Azimut(172,Breite,720)));

fprintf('--- SonnenStand: Tag Monat Stunde Minute tf -> alpha gamma ---\n');
% [Tag Monat Stunde Minute tf]
cases = [26 10 12 0 0;
         26 10 15 0 0;
         27 10 12 0 0;
         21  6 13 0 1];
for k = 1:size(cases,1)
    c = cases(k,:);
    [a,g] = SonnenStand(Breite,Laenge,c(1),c(2),c(3),c(4),c(5));
    fprintf('%2d.%2d. %2d:%02d tf=%d  ->  %.4f  %.4f\n', c, real(a), real(g));
end
