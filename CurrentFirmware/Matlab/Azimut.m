function [alpha] = Azimut(d,phi,WOZ)
rphi  = grad2rad(phi);
beta  = StundenWinkel(WOZ);
rbeta = grad2rad(beta);
delta = Deklination(d);
rdelta= grad2rad(delta);
rgamma= asin(cos(rbeta)*cos(rphi)*cos(rdelta)+sin(rphi)*sin(rdelta));
gamma = rad2grad(rgamma); 
for i = 1:length(WOZ)
    if (WOZ(i) < 12*60)
        alpha(i) = pi - acos((sin(rgamma(i)).*sin(rphi)-sin(rdelta))./(cos(rgamma(i)).*cos(rphi)));
    else
        alpha(i) = pi + acos((sin(rgamma(i))*sin(rphi)-sin(rdelta))/(cos(rgamma(i)).*cos(rphi)));
    end 
end
alpha = rad2grad(alpha);
end