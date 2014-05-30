% change cwd to data direcotry
cd '../../supervisor/elam3/';

% using gui ask user to select data log file
[filename, pathname] = uigetfile('*.txt','Select the supervisor log file');

% read the data file and skip the first row (name of columns)
data = dlmread([pathname filename], ' ', 1, 0);

% plot performance per minute vs. time 
subplot(4,1,1)
plot((data(:,2) - data(1,2))/60, data(:,11));
xlabel('time (min)');
ylabel('Performance/min');

% plot feature vs. time 
subplot(4,1,2)
plot((data(:,2) - data(1,2))/60, data(:,21));
xlabel('time (min)');
ylabel('Feature');

% plot parameter 1 vs. time 
subplot(4,1,3)
plot((data(:,2) - data(1,2))/60, data(:,22));
xlabel('time (min)');
ylabel('Param1');

% plot parameter 2 vs. time 
subplot(4,1,4)
plot((data(:,2) - data(1,2))/60, data(:,23));
xlabel('time (min)');
ylabel('Param2');
