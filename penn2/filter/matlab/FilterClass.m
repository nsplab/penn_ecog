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
        intialTime = '';
        demoMode = 0;
    end
    methods
        % class constructor
        
        function filter = FilterClass(varargin)
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
            %data = struct('value', filter.speed);
            %filter.ssave.('varname') = data;
            filter.speed = filter.speed + 1;
            filter.ssave = struct();
            if filter.firstrun 
                filter.opt = {};
                filter.firstrun = false;
                filter.numberOfFilterParameters = length(filter.parameterNames);
                filter.parameterNames(end+1) = {'score'};
                filter.parameterNames(end+1) = {'timestamp'};
                filter.parameterNames(end+1) = {'epoch_seconds'};
                filter.parameterNames(end+1) = {'is_at_target'};
                filter.parameterNames(end+1) = {'target_x'};
                filter.parameterNames(end+1) = {'target_y'};
                filter.parameterNames(end+1) = {'target_z'};filter.parameterNames(end+1) = {'hand_z'};
                filter.parameterNames(end+1) = {'hand_x'};
                filter.parameterNames(end+1) = {'hand_y'};
                filter.parameterNames(end+1) = {'hand_z'};
                
                for i=1:length(filter.currentFeatures),
                    columnName = sprintf('feature_%i', i);
                    filter.parameterNames(end+1) = {columnName};
                    filter.intialTime = datestr(now,'mm_dd_yyyy_HH:MM');
                end
                
                filter.ssave.('parameter_names') = filter.parameterNames;
            else
                filter.opt = {'-append'};
            end
                filter.parameterValues{1,filter.numberOfFilterParameters+1} = filter.currentScore;
                filter.parameterValues{1,filter.numberOfFilterParameters+2} = filter.currentTimeStamp;
                filter.parameterValues{1,filter.numberOfFilterParameters+3} = filter.epochSeconds;
                filter.parameterValues{1,filter.numberOfFilterParameters+4} = filter.isAtTarget;
                filter.parameterValues{1,filter.numberOfFilterParameters+5} = filter.targetX;
                filter.parameterValues{1,filter.numberOfFilterParameters+6} = filter.targetY;
                filter.parameterValues{1,filter.numberOfFilterParameters+7} = filter.targetZ;
                filter.parameterValues{1,filter.numberOfFilterParameters+8} = filter.handX;
                filter.parameterValues{1,filter.numberOfFilterParameters+9} = filter.handY;
                filter.parameterValues{1,filter.numberOfFilterParameters+10} = filter.handZ;
                for i=1:length(filter.currentFeatures),
                    filter.parameterValues{1,filter.numberOfFilterParameters+10+i} = filter.currentFeatures(i);
                end

                vname = sprintf('parameters_timestamp_%i', filter.currentTimeStamp);
                filter.ssave.(vname) = filter.parameterValues;
                tstruct = filter.ssave;
                save([dataPath '/filter_log_' filterName '_' filter.intialTime '.mat'], '-struct', 'tstruct', filter.opt{:});
        end
    end
end
