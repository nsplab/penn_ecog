rsePath = load('/home/user/code/penn2/penn/penn2/filter/cpp/build/rseTrajectory.txt');
speed=sqrt(rsePath(:,4).^2+rsePath(:,5).^2+rsePath(:,6).^2);
speeds = reshape(speed, 300, 100);
meanSpeed = mean(speeds,2);
confSpeed = bootci(500,{@mean, speeds'});

set(0, 'DefaultFigureRenderer', 'OpenGL');
set(gcf,'Visible','on');
set(0,'defaultAxesFontName', 'Arial')
set(0,'defaultaxesfontSize',12)
set(gcf,'color','w');

hold off;
h1=plot(speeds, '-G');
hold on;
h2=plot(confSpeed(1,:), '-r');
h3=plot(confSpeed(2,:), '-r');
h4=plot(meanSpeed);

xlabel('time (steps)');
ylabel('speed');

legend([h1(1),h4,h3],'100 instances','mean','95% CI')

myaa([6 4], 'speed_rse.png')

hold off;

figure;

set(0, 'DefaultFigureRenderer', 'OpenGL');
set(gcf,'Visible','on');
set(0,'defaultAxesFontName', 'Arial')
set(0,'defaultaxesfontSize',12)
set(gcf,'color','w');

xlabel('X');
ylabel('Y');
zlabel('Z');

title('Trajectories');

hold on;

N=0;
for i=1:100,
    plot3(rsePath(1+N:300+N,1),rsePath(1+N:300+N,2),rsePath(1+N:300+N,3));
    N=N+300;
end
daspect([1 1 1]);
pbaspect([1 1 1]);

myaa([6 4], 'rse_trajectory.png')

view([90 0 0])

myaa([6 4], 'rse_trajectory_yz.png')
