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
    [recvData, hasMore] = zmq( 'receive', featurePipe );
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
    
    % send commands to the supervisor module
    %data1 = uint8('hello world!')';
    % send time stamp
    %num2str()
    % send number of features 
    % send number of parameters
    % send features
    % send parameters
    % send parameters names
    
    supervisorData = uint8([num2str(timeStamp) ' ' num2str(0.1) ' ' num2str(0.0) ' ' num2str(0.0)])';
    
    nbytes = zmq( 'send', supervisorPipe, supervisorData );
    
    [recvData, hasMore] = zmq( 'receive', supervisorPipe );

    
    
end