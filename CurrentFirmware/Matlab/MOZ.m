function [Min_MOZ] = MOZ(Min_MEZ,lambda)
%lambda entspricht dem Längengrad
Min_MOZ    = Min_MEZ-4*(15-lambda);  % Min_MEZ Minuten seit 0:00 Uhr
end