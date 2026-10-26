function [beta] = StundenWinkel(Min_WOZ)
WOZ_h = Min_WOZ/60;        % WOZ ist hier in Minuten übergeben, wird in Stunden mit Dezimalbruch umgerechnet
beta  = (12-WOZ_h)*15;     % beta in °, 15° entsprechen einer Stunde
end