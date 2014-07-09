%FilterClass.m
%This file defines a Parent Class for all Matlab-based filters that work
%with the Penn2 C++ platform
%
%Actual filters that you implement will inherit from this parent class.
%an example of a filter that inherits FilterClass is given in 
%/penn2/filter/matlab/filter1.m
%
%uses Matlab's Object-Oriented Programming, detailed at:
%http://www.mathworks.com/help/pdf_doc/matlab/matlab_oop.pdf


% reload the data later
% s = load('myfile.mat');
% c = struct2cell(s)

classdef FilterClass < handle
    properties
        speed = 0;
        parameterValues = {}; %cell array (list in {}) of parameter values
        parameterNames = {}; %cell array (list in {}) same length as parameterValues, providing names for each parameter as a string (eg. {'theta_p','theta_q'})
        % structure used to keep track the data, not a cell
        ssave = struct();
        firstrun = true;
        opt = {};
        currentTimeStamp = 0;
        currentFeatures;
        currentScore = 0;
        isAtTarget = false;
        epochSeconds = 0;
        targetX = 0;
        targetY = 0;
        targetZ = 0;
        handX = 0;
        handY = 0;
        handZ = 0;
        numberOfFilterParameters = 0;
        initialTime = '';
        demoMode = 0;
        extra_parameter_names = {};
        game_state_names = ...%{'score', 'timestamp', 'epoch_seconds', 'is_at_target', 'target_x', 'target_y', 'target_z', 'hand_x', 'hand_y', 'hand_z'};
                           {'currentScore', 'currentTimeStamp', 'epochSeconds', 'isAtTarget', 'targetX', 'targetY', 'targetZ', 'handX', 'handY', 'handZ'};
    end
    methods
        % class constructor
        
        function filter = FilterClass(varargin)
            %fprintf('FilterClass\n');
            filter.speed = 1;
        end
        % function that is called every iteration when new
        % feature values are received from the feature extractor
        function filter=RunFilter(filter, val)
            filter.speed = val * filter.speed;
        end
        
        function sobj = saveobj(obj)
            sobj = obj.ssave;
        end
        
        function filter=LogParameters(filter, dataPath, filterName)
            % TODO: Save order: game state, extra debug values, number of parameters, number of features, parameters, faetures
           
            %data = struct('value', filter.speed);
            %filter.ssave.('varname') = data;
            filter.ssave = struct();
            if filter.firstrun 
                filter.initialTime = datestr(now,'mm_dd_yyyy_HH:MM');
                filter.opt = {};
                filter.firstrun = false;

                filter.ssave.parameter_names = filter.parameterNames; % cell array of observation model parameter names
                filter.ssave.game_state_names = filter.game_state_names; % cell array of game state names {'currentScore', 'currentTimeStamp', 'epochSeconds', 'isAtTarget', 'targetX', 'targetY', 'targetZ', 'handX', 'handY', 'handZ'}
                filter.ssave.extra_parameter_names = filter.extra_parameter_names; % cell array of extra parameters (innovation)
                filter.ssave.feature_names = cell(1, length(filter.currentFeatures)); % cell array of feature names
                filter.ssave.number_of_parameters = numel(filter.ssave.parameter_names);
                filter.ssave.number_of_features = numel(filter.ssave.feature_names);

                for i=1:length(filter.currentFeatures),
                    columnName = sprintf('feature_%i', i);
                    filter.feature_names{i} = columnName;
                end

            else
                filter.opt = {'-append'};
            end

            filter.ssave.parameter_values = filter.parameterValues;
            filter.ssave.game_state_values = cell(1, numel(filter.game_state_names));
            for i = 1:numel(filter.game_state_names)
                filter.ssave.game_state_values{1,i} = filter.(filter.game_state_names{i});
            end
            filter.ssave.extra_parameter_values = cell(1, numel(filter.extra_parameter_names));
            for i = 1:numel(filter.extra_parameter_names)
                filter.ssave.extra_parameter_values{1,i} = filter.(filter.extra_parameter_names{i});
            end

            filter.ssave.currentFeatures = filter.currentFeatures;

            vname = sprintf('timestamp_%i', filter.currentTimeStamp);
            %ssave = filter.ssave
            tstruct.(vname) = filter.ssave;
            if (exist([dataPath '/filter_log_' filterName '_' filter.initialTime '.mat']))
                filter.opt = {'-append'};
            end
            save([dataPath '/filter_log_' filterName '_' filter.initialTime '.mat'], '-struct', 'tstruct', filter.opt{:});
        end

        function filter=LoadParameters(filter, selectedSession)
            tstruct = load(selectedSession);
            c = struct2cell(tstruct);
            filter.ssave = c{end};

            filter.parameterNames = c{1}.parameter_names;
            filter.game_state_names = c{1}.game_state_names;
            filter.extra_parameter_names = c{1}.extra_parameter_names;

            filter.parameterValues = filter.ssave.parameter_values;
            for i = 1:numel(filter.game_state_names)
                filter.(filter.game_state_names{i}) = filter.ssave.game_state_values{1,i};
            end
            filter.currentFeatures = filter.ssave.currentFeatures;

            filter.firstrun = true;
            filter.numberOfFilterParameters = length(filter.parameterNames);
            %filter.parameterNames = c{1};
            %filter.ssave = struct();
            %filter.ssave.('parameter_names') = filter.parameterNames;
            filter.initialTime = datestr(now,'mm_dd_yyyy_HH:MM');
        end
    end
end

