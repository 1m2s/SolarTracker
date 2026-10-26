function [delta] = Deklination(d)
%Abweichung zwischen Magnetfeld und Geographie
Js    = 360.0*d/365.0;       % Umrechnung des Tages in Laufvariable in rad
a1    = grad2rad(Js+9.1); 
a2    = grad2rad(2*Js+5.4);
a3    = grad2rad(3*Js+26);
delta = 0.3948-23.2559*cos(a1)-0.391*cos(a2)-0.1764*cos(a3); % delta ist hier in °
end