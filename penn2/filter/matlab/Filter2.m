classdef Filter2 < FilterClass
    properties
    end
    methods
        % class constructor
        function filter = Filter1(varargin)
        end
        % function that is called every iteration when new
        % feature values are received from the feature extractor
        function [ control ] = RunFilter(filter, recvdFeatures)
            control = recvdFeatures;
            control
        end
    end
end
