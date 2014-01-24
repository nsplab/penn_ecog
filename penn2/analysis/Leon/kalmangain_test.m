%Test the steady state covariance
F=1;
Q=0.24218176;
k=0.43162217;
H=1;
est_2 = 0;
for i=1:100
    est_1 =  F*est_2+Q;
    est_2 = (1-k*H)*est_1
    plot(i,est_2)
    hold on
end

((1-k*H)*Q)/(1-(1-k*H)*F)