classdef Filter2 < FilterClass
    properties
    end
    methods
        % class constructor
        function filter = Filter2(varargin)
            filter.speed = 2;
            filter.ssave = struct();
            filter.parameterNames = {'alpha', 'beta'};
        end
        % function that is called every iteration when new
        % feature values are received from the feature extractor
        function filter=RunFilter(filter, val)
            filter.speed = filter.speed + 1;
             filter.parameters{1,1} = filter.speed;
             filter.parameters{1,2} = filter.speed + val;
        end
    end
end
