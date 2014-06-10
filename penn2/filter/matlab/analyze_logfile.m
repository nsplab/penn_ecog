% change cwd to data direcotry
cd '../../data/';

% using gui ask user to select data log file
[filename, pathname] = uigetfile('*.mat','Select the supervisor log file');

rawData = load([pathname filename]);