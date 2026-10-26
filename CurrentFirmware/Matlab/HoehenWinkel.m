function [gamma] = HoehenWinkel(beta,phi,delta)
beta_r = grad2rad(beta);
phi_r  = grad2rad(phi);
delta_r= grad2rad(delta);
gamma = asin(cos(beta_r)*cos(phi_r)*cos(delta_r)+sin(phi_r)*sin(delta_r));
gamma = rad2grad(gamma);
end