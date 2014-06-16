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

[keysFilter,sectionsFilter,subsectionsFilter] = inifile('../../config/filter.cfg','readall');
dataPathIdx = find(strcmp('datapath', keysFilter(:,3)));
dataPath = keysFilter{dataPathIdx, 4};


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

%while the user hasn't asked the program to stop
while ~exit
    % receive features from the feature extractor module
    disp('receive');
    % wait up to 1 seconds, if there is no data coming in then stop the
    % filter
    
    waitForData = true;
    tic
    while waitForData
        [recvData, hasMore] = zmq( 'receive', featurePipe );
        if hasMore == 0
            waitForData = false;
        end
        if toc > 1.0
            disp('no darta from feature_extractor');
            exit = true;
            break;
        end
    end
    
    disp('received');
    % extract the time stamp from the first 8 bytes and typcast it into a
    % 64 bit integer which contains the time stamp
    timeStamp = typecast(recvData(1:8),'uint64');
    % extract the features
    recvdFeatures = typecast(recvData(9:end),'single');
    recvdFeatures
    filter.currentFeatures = recvdFeatures;
    filter.currentTimeStamp = timeStamp;
    filter.RunFilter();
    filter.LogParameters(dataPath, filterName)
    
    imitatorBaseline = 50;
    imitatorAmplifier = 2;
    
    controlX = recvdFeatures(1) / imitatorAmplifier;
    controlX = controlX^2;
    controlX = controlX - imitatorBaseline;
    
    controlY = recvdFeatures(2) / imitatorAmplifier;
    controlY = controlY^2;
    controlY = controlY - imitatorBaseline;

    controlZ = recvdFeatures(3) / imitatorAmplifier;
    controlZ = controlZ^2;
    controlZ = controlZ - imitatorBaseline;

    supervisorData = uint8([num2str(timeStamp) ' ' num2str(controlX) ' ' num2str(controlY) ' ' num2str(controlZ)])';
    
    disp('going to send to supervisor');
    nbytes = zmq( 'send', supervisorPipe, supervisorData );
    disp('sent to supervisor');
    
    disp('receive from supervisor');
    
    waitForData = true;
        
    tic
    while waitForData
        [recvData, hasMore] = zmq( 'receive', supervisorPipe );
        recvData
        if hasMore == 0
            waitForData = false;
        end
        if toc > 1.0
            disp('no darta from supervisor');
            exit = true;
            break;
        end
    end
    
    disp('received from supervisor');
    
    % extract from recvData: score, score/min
end

zmq( 'cleanup');

