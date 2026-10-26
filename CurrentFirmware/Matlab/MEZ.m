function [Min_MEZ] = MEZ(Stunde,Minute,tf)
%Ist Sommerzeit?
if tf == 1
    MEZ_Stunde = Stunde -1;
elseif tf == 0
    MEZ_Stunde = Stunde;
end
%
MEZ_Minute = Minute;
Min_MEZ    = MEZ_Minute+MEZ_Stunde*60;
%
end