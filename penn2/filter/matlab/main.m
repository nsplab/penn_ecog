%filter1.m
%this is the master filter for all Matlab filters
%it expects that a separate matlab function takes care of implementing
%the actual filter eqauations, but
% this file takes care of interfacing with the rest of the Penn2 C++ modules.
%
% this filter uses zmq to 
% receive
%    features from the feature module
% send - the following values as ASCII strings over ZMQ:
% (this is a known vulnerability that could slow down execution -
%   the preferred implementation is to send as a binary data)
%   timestamp
%   hand position
%   # of features
%   # of parameters
%   feature values
%   parameter values
%   parameter names

% Path for zmq MATLAB wrapper
addpath('../../libs/zeromq-matlab/')

% Get rid of any remaining zmq sockets
zmq( 'cleanup');

% read the main log file created by the launcher
% this is based on the INI file standard. the matlab function inifile
% reads the INI format, downloaded on 6/5/14 from http://www.mathworks.com/matlabcentral/fileexchange/2976-inifile/content/inifile.zip
%
%keys returns a 4-column cell array.
%   column 1 - section title (currently, the only section is   'experimentlog'
%   column 2 - subsection title (currently blank)
%   column 3- name of parameter as a string (ex. 'time', which is the wall clock date and time)
%   column 4 - value of the parameter
[keys,sections,subsections] = inifile('../../data/log.txt','readall');

% sampling rate index
samplingRateIdx = find(strcmp('samplingrate', keys(:,3)));
% sampling rate value
samplingRate = str2num(keys{samplingRateIdx, 4});

% get the index of the algorithm parameter
filterTypeIdx = find(strcmp('algorithm', keys(:,3)));
% get the algorithm selected by the user in the launcher GUI
filterType = keys{filterTypeIdx, 4};

if strcmp('Matlab 1', filterType),
    filter = Filter1;
    filterName = 'Matlab_1';
end
if strcmp('Matlab 2', filterType),
    filter = Filter2;
    filterName = 'Matlab_2';
end
if strcmp('Matlab 3', filterType),
    filter = Filter3;
    filterName = 'Matlab_3';
end
if strcmp('Matlab 4', filterType),
    filter = Filter4;
    filterName = 'Matlab_4';
end
if strcmp('Matlab JointRSE', filterType),
    dimensions = 1;
    filter = FilterJointRSE(dimensions, dimensions);
    filterName = 'Matlab_JointRSE';
end

[keysFilter,sectionsFilter,subsectionsFilter] = inifile('../../config/filter.cfg','readall');
dataPathIdx = find(strcmp('datapath', keysFilter(:,3)));
dataPath = keysFilter{dataPathIdx, 4};

%class(keysFilter)
selectedSessionIdx = find(strcmp('selectedsession', keysFilter(:,3)));
selectedSession = keysFilter{selectedSessionIdx, 4};
%if (~isempty(selectedSession))
if (~strcmp(selectedSession, '""'))
    filter.LoadParameters(selectedSession);
end

% get the value of the demo state from the config file
demoModeIdx = find(strcmp('demomode', keysFilter(:,3)));
demoMode = keysFilter{demoModeIdx, 4};
% set the demo flag of the filter
filter.demoMode = demoMode;

% current target postion
% current target radius
% current hand position in virtual environment
% filter type
% new feature values

% filter should know about the pause event
% filter to improve performance logs all the parameters as in the same log
% file arfe that generated wby the supervisor 
% plus a columns for pause and all the filter parameters

% 

% add the feedback from the supervisor tell whether the hand is hitting the
% target


% create a zmq subscriber ipc socket to receive features for a single
% iteration. assumes that under the /tmp, the file features.pipe exists.
% this file is defined from the feature extractor module for ipc
% communication of features.
% the function zmq is defined by /penn2/filter/matlab/zmq.mexa64
% which is a link to the the compiled version of the source code located at
% penn2/libs/zeromq-matlab/zmq.mexa64, which is based on the source code
%zmq.cc in that same directory, compiled with the Makefile in that
%directory
featurePipe = zmq( 'subscribe', 'ipc', 'features.pipe' );

% create a zmq publisher to send the commands to the supervisor module
supervisorPipe = zmq( 'request', 'ipc', 'supervisor.pipe' );

%define an empty byte array that receives features from the featurePipe
%sent by feature extractor
recvFeatures = [];

%loop condition 'exit' used to allow the user to kill the loop from the
%launcher
exit = false;

Imitator_pos = [];
%while the user hasn't asked the program to stop
while ~exit
    % receive features from the feature extractor module
    %disp('receive');
    % wait up to 1 seconds, if there is no data coming in then stop the
    % filter
    
    waitForData = true;
    tic
    while waitForData
        [recvData, hasMore] = zmq( 'receive', featurePipe );
        if hasMore == 0
            waitForData = false;
        end
        if toc > 15.0
            %disp('no data from feature_extractor');
            exit = true;
            break;
        end
    end
    if exit
        break;
    end

    %disp('received');
    % extract the time stamp from the first 8 bytes and typcast it into a
    % 64 bit integer which contains the time stamp
    timeStamp = typecast(recvData(1:8),'uint64');
    % extract the features
    recvdFeatures = typecast(recvData(9:end),'single');
    %size(recvData)
    imitator_pos = double(recvdFeatures(7))

%%%%%%%%%%%%%%%%%%%
% DEBUG CODE 7/7/14
filter.target(1:dimensions) = recvdFeatures(4); % update filter target position
filter.setHandPos(imitator_pos);
    Imitator_pos = [Imitator_pos imitator_pos];
    save('imitator.mat', 'Imitator_pos');
recvdFeatures
%%%%%%%%%%%%%%%%%%%

    %% Run filter for one time step (including predict step and update step) with new features
    filter.currentTimeStamp = timeStamp; %log the timestamp in the filter object
    recvdFeatures = recvdFeatures(1);    %extract the features for the timestep
    %TODO: log number of features / number of parameters
    control = filter.RunFilter(recvdFeatures); % run predict and update for filter
    filter.LogParameters(dataPath, filterName); % save parameters and features to file

    %supervisorData = uint8([num2str(timeStamp) ' ' num2str(control, '%f')]);
    %[num2str(timeStamp) ' ' num2str(control, '%f')]
    %asdknasd

    %% Send decoded velocity to supervisor; store this value in a string for sending through ZMQ
    control = [control 0 0];   %DEBUG CODE: specific hardcoded for 1D case
    controlX = control(1);     %x,y, and z decoded velocities stored separately
    controlY = control(2);
    controlZ = control(3);
    supervisorData = uint8([num2str(timeStamp) ' ' num2str(controlX) ' ' num2str(controlY) ' ' num2str(controlZ)]);
    
    %disp('going to send to supervisor');
    nbytes = zmq( 'send', supervisorPipe, supervisorData ); %command to zmq to send decoded velocity to supervisor
    %disp('sent to supervisor');
    
    %disp('receive from supervisor');
    
    % loop to receive data from supervisor about the game state (specifically the actual virtual arm position)
    waitForData = true;
        
    tic
    while waitForData
        [recvData, hasMore] = zmq( 'receive', supervisorPipe );
        %recvData
        if hasMore == 0   %if the zmq function indicates the data has been completely received,
            waitForData = false; %then set waitForData to break this while loop
        end
        if toc > 15.0 %if you've been waiting without data from supervisor for more than 15 seconds, then exit the matlab function, because this means that the supervisor module is not running, likely because the user has stopped the bci task
            disp('no data from supervisor');
            exit = true;
            break;
        end
    end
    
    %disp('received from supervisor');
    
    % extract from recvData: score, score/min
    value = str2double(strsplit(char(recvData')));
    target = value(1:3);
    filter.target(1:dimensions) = target(1:dimensions); % update filter target position
    hand = value(4:6);
    filter.setHandPos(hand);
    trial = value(7);
    triaining_test = value(8);
    attending = value(9);

end

zmq( 'cleanup');

