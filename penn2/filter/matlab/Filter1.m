classdef Filter1 < FilterClass
    properties
    end
        methods
        % class constructor
        function filter = Filter1(varargin)
            filter.speed = 2;
            filter.ssave = struct();
            filter.parameterNames = {'alpha', 'beta'};
        end
        % function that is called every iteration when new
        % feature values are received from the feature extractor
        function filter=RunFilter(filter)
            filter.speed = filter.speed + 1;
             filter.parameterValues{1,1} = filter.speed;
             filter.parameterValues{1,2} = filter.speed + 1;
        end
    end
end