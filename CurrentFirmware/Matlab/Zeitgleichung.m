function [zgl] = Zeitgleichung(Jg)
a1 = grad2rad(Jg+85.9);
a2 = grad2rad(2*Jg+108.9);
a3 = grad2rad(3*Jg+105.2);
zgl =0.0066+7.3525*cos(a1)+9.9359*cos(a2)+0.3387*cos(a3);
end