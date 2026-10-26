function [alpha,gamma] = SonnenStand(Breite,Laenge,Tag,Monat,Stunde,Minute,tf)
%
Min_MEZ = MEZ(Stunde,Minute,tf);
Min_MOZ = MOZ(Min_MEZ,Laenge);
%
d       = JahresTag(Monat,Tag);
Jg      = d/365*360;
zgl     = Zeitgleichung(Jg);
Min_WOZ = Min_MOZ+zgl;
%
delta   = Deklination(d);
alpha   = Azimut(d,Breite,Min_WOZ);
beta    = StundenWinkel(Min_WOZ);
gamma   = HoehenWinkel(beta,Breite,delta);
%
end