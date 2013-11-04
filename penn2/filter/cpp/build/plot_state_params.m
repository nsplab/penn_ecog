state = load('/home/user/code/penn2/penn/penn2/filter/cpp/build/state.txt');

set(0, 'DefaultFigureRenderer', 'OpenGL');
set(gcf,'Visible','on');
set(0,'defaultAxesFontName', 'Arial')
set(0,'defaultaxesfontSize',12)
set(gcf,'color','w');

hold off;

plot(state(:,1));

hold on;

plot(state(:,2), 'k');

plot(state(:,4), 'r');

myaa([6 4], 'channel_params.png')
