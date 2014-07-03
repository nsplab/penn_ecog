% Class definition for Filter1
% Implements Gaussian Time-Invariant version of JointRSE
% Authors: Bryan, Ram
% First Edited: 7/2/2014 5:44 PM

classdef Filter1 < FilterClass
    properties
        imitatorBaseline = 50;
        imitatorAmplifier = 2;
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
        function [ control ] = RunFilter(filter, recvdFeatures)
            filter.speed = filter.speed + 1;
            filter.parameterValues{1,1} = filter.speed;
            filter.parameterValues{1,2} = filter.speed + 1;

            %control = (recvdFeatures / filter.imitatorAmplifier) .^ 2 - filter.imitatorBaseline;

            controlX = recvdFeatures(1) / filter.imitatorAmplifier;
            controlX = controlX^2;
            controlX = controlX - filter.imitatorBaseline;
            
            controlY = recvdFeatures(2) / filter.imitatorAmplifier;
            controlY = controlY^2;
            controlY = controlY - filter.imitatorBaseline;
            
            controlZ = recvdFeatures(3) / filter.imitatorAmplifier;
            controlZ = controlZ^2;
            controlZ = controlZ - filter.imitatorBaseline;

            control = [ controlX controlY controlZ ];

        end
    end
end
