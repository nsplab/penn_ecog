% Class definition for FilterJointRSE
% Implements Gaussian Time-Invariant version of JointRSE
% Authors: Bryan, Ram
% First Edited: 7/2/2014 5:44 PM

classdef FilterJointRSE < FilterClass
    properties
        % This block is used to declare fields in the class
        L = [];
        imitatorBaseline = 50;
        imitatorAmplifier = 2;
        dimensions;
        features;
        A;
        B;
        target;
        covariance;
        innovation;
    end
    methods
        % class constructor
        function filter = FilterJointRSE(dimensions, features)
            fprintf('FilterJointRSE\n');
            filter.dimensions = dimensions;
            filter.features = features;
            %% Set parameter names
            % For a velocity-dependent affine observation model of the form
            % observation = alpha * velocity + beta + gaussian noise
            % in the code below, parameters are named (for example, in 2 dimension, 2 features)
            % f1b1, f2b1, f1b2, f2b2, f1b3, f2b3
            % and the observation model is
            % observation1 = f1b1 * velocity1 + f1b2 * velocity2 + f1b3
            % observation2 = f2b1 * velocity1 + f2b2 * velocity2 + f2b3
            % f1b1, f2b1, f1b2, f2b2 correspond to alpha
            % f1b3, f2b3 correspond to beta
            filter.parameterNames = cell(1, (dimensions + 1) * features + 2 * dimensions); % extra dimension for affine term + position + velocity
            filter.numberOfFilterParameters = numel(filter.parameterNames);
            for i = 1:(dimensions + 1)
                for j = 1:features
                    filter.parameterNames{(i - 1) * features + j} = ['f' int2str(j) 'b' int2str(i)];
                end
            end
            for i = 1:dimensions
                filter.parameterNames{(dimensions + 1) * features + i} = ['position' int2str(i)];
                filter.parameterNames{(dimensions + 1) * features + dimensions + i} = ['velocity' int2str(i)];
            end
            %% Initialize State Vector with Parameters
            filter.parameterValues = cell(size(filter.parameterNames));
            for i = 1:numel(filter.parameterValues)
                filter.parameterValues{1, i} = 0;
            end

            %% Compute matrices for Time-Invariant Trajectories
            delta = 1;
            target = zeros(2 * dimensions, 1);
            [ A, B, Q, R, L, K ] = trajectory(dimensions, delta, target); % TODO: delta and target?
            filter.A = A;
            filter.B = B;
            filter.target = target;
            filter.L = L;
            filter.covariance = eye((dimensions + 1) * features + 2 * dimensions); % TODO: more systematic choice than identity?
            filter.extra_parameter_names = {'innovation'};
        end
        % function that is called every iteration when new
        % feature values are received from the feature extractor
        function [ control ] = RunFilter(filter, recvdFeatures)
            obs = (recvdFeatures / filter.imitatorAmplifier) .^ 2 - filter.imitatorBaseline;

            dimensions = filter.dimensions;
            features = filter.features;
            A = filter.A;
            B = filter.B;
            L = filter.L;
            target = filter.target;
            %% Prediction Step
            x = cell2mat(filter.parameterValues)';
            %x
            %x = [1;1;1;0;0;0];
            %control = L * (x((dimensions + 1) * features + 1:end) - target)
            %state = x((dimensions + 1) * features + 1:end)
            %target
            %err = state - target
            %return

            F = eye((dimensions + 1) * features + 2 * dimensions);
            F(((dimensions + 1) * features + 1):end,((dimensions + 1) * features + 1):end) = (A + B * L);
            Q = zeros((dimensions + 1) * features + 2 * dimensions);
            Q(((dimensions + 1) * features + 1):end,((dimensions + 1) * features + 1):end) = eye(2 * dimensions); % TODO: better choice than identity?
            b = [zeros((dimensions + 1) * features, 1);-B * L * target];
            pred_x = F * x + b;
            %x
            %pred_x
            pred_cov = F * filter.covariance * F' + Q;
            %for i = 1:100000
            %    pred_cov = F * pred_cov * F' + Q;
            %end
            %pred_cov

            %% Update Step
            pred_uHistory = [pred_x((dimensions + 1) * features + dimensions + 1:end); 1]; % extract predicted velocity and append affine term
            channelParameters = cell2mat(reshape(filter.parameterValues(1:(dimensions + 1) * features), features, dimensions + 1));
            %pred_uHistory
            channelParameters
            estimated_obs = channelParameters * pred_uHistory;
            %estimated_obs
            %obs

            % derivative of the observation vector with respect to the state, evaluated at estimated_obs.
            D_obs = zeros(size(pred_x, 1), features);
            %filter.parameterNames
            %size(D_obs)
            for c = 1:features
                % velocity terms
                D_obs(((c - 1) * dimensions + 1):(c * dimensions), c) = pred_uHistory(1:dimensions, 1);
                %D_obs(((c - 1) * dimensions + 1):(c * dimensions), c) = 123;
%((c - 1) * dimensions + 1):(c * dimensions)
                for i = 1:dimensions
                    D_obs(size(D_obs, 1) - dimensions + i, c) = pred_x((c - 1) * dimensions + i, 1);
                    %D_obs(size(D_obs, 1) - dimensions + i, c) = 234;
                end
                % affine terms
                D_obs(features * dimensions + c, c) = 1;
            end
            %D_obs
            %size(D_obs)

            % double derivative of the observation vector with respect to the state, evaluated at estimated_obs.
            DD_obs = zeros(size(pred_x, 1), size(pred_x, 1), features);
            for c = 1:features
                for i = 1:dimensions
                    % Velocity terms
                    DD_obs((c - 1) * dimensions + i, size(DD_obs, 2) - dimensions + i, c) = 1;
                    DD_obs(size(DD_obs, 2) - dimensions + i, (c - 1) * dimensions + i, c) = 1;
                    % Affine terms
                    % Nothing to do
                    % Taking two derivatives will always result in a 0 (regardless of which two are selected)
                end
            end
            %DD_obs

            baseVariance = 1.0;
            channelVariances = baseVariance * ones(features, 1);
            cov_adjust = zeros(size(pred_cov));
            for c = 1:features
                if (~isnan(estimated_obs(c, 1)))
                    cov_adjust = cov_adjust + 1 / channelVariances(c) * ...
                                              (D_obs(:, c) * D_obs(:, c)' + ...
                                              (estimated_obs(c, 1) - obs(c)) * DD_obs(:, :, c));
                end
            end
            new_cov_inv = inv(pred_cov) + cov_adjust;
            new_cov = inv(new_cov_inv);

            x_adjust = zeros(size(pred_x));
            innovation = zeros(features, 1);
            for c = 1:features
                if(~isnan(estimated_obs(c, 1)))
                    x_adjust = x_adjust + 1 / channelVariances(c) * (obs(c) - estimated_obs(c, 1)) * D_obs(:, c);
                    innovation(c, 1) = (obs(c) - estimated_obs(c, 1)) / channelVariances(c);
                end
                %fprintf(['novel_' int2str(c) ': ' num2str(obs(c) -  estimated_obs(c, 1)) '\n']);
            end
            filter.innovation = innovation;



            new_x = pred_x + new_cov * x_adjust;
            filter.parameterValues = mat2cell(new_x, ones((dimensions + 1) * features + 2 * dimensions, 1), 1)';

            control = new_x((dimensions + 1) * features + dimensions+1:end); % grab velocity
            assert(numel(control) == dimensions);
            %target
            %control(end-5:end)-target
            control
            %pause(0.5);
            %reshape(filter.parameterNames,10,4)

        end
        function [ ] = setHandPos(filter, hand)
            for i = 1:3
                filter.parameterValues{(filter.dimensions + 1) * filter.features + i} = single(hand(i));
            end
        end
    end
end

