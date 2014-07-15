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
        Position = [];
        channelVariances;
        obs;
        estimated_obs;
    end
    methods
        % class constructor
        function filter = FilterJointRSE(dimensions, features)
            %% Copy number of features/dimensions
            % features: number of channels in the signal (number of observations)
            % dimensions: number of dimensions in graphics/task (typically 1-3)
            filter.dimensions = dimensions;
            filter.features = features;

            %% Set parameter names
            % For a velocity-dependent affine observation model of the form
            % observation = alpha * velocity + beta + gaussian noise
            % in the code below, parameters are named (for example, in 2 dimension, 2 features)
            % f1b1, f2b1, f1b2, f2b2, f1b3, f2b3
            % and the observation model is
            % {observation1 = f1b1 * velocity1 + f1b2 * velocity2 + f1b3 + gaussian noise
            % {observation2 = f2b1 * velocity1 + f2b2 * velocity2 + f2b3 + gaussian noise
            % f1b1, f2b1, f1b2, f2b2 correspond to alpha
            %   alpha = [f1b1 f1b2]
            %           [f1b2 f2b2]
            % f1b3, f2b3 correspond to beta
            %   beta = [f1b3]
            %          [f2b3]

            % (dimensions + 1) * features: corresponds to parameter values, one parameter for each dimension-feature pair (extra dimension for affine term)
            % 2 * dimensions: corresponds to position and velocity
            filter.parameterNames = cell(1, (dimensions + 1) * features + 2 * dimensions);
            filter.numberOfFilterParameters = numel(filter.parameterNames); % compute number of filter parameters from the list size

            % Generate the parameter names corresponding to parameters
            for i = 1:(dimensions + 1)
                for j = 1:features
                    filter.parameterNames{(i - 1) * features + j} = ['f' int2str(j) 'b' int2str(i)]; % parameter names
                end
            end

            % Generate the parameter names corresponding to position and velocity
            for i = 1:dimensions
                filter.parameterNames{(dimensions + 1) * features + i} = ['position' int2str(i)]; % position names
                filter.parameterNames{(dimensions + 1) * features + dimensions + i} = ['velocity' int2str(i)]; % velocity names
            end

            %% Initialize State Vector with Parameters
            % Parameters, position, and velocity are set to 0
            filter.parameterValues = cell(size(filter.parameterNames));
            for i = 1:numel(filter.parameterValues)
                filter.parameterValues{1, i} = 0.0;
            end

            %% Compute matrices for Time-Invariant Trajectories
            % Constants
            delta = 0.033; % time step (seconds)
            target = zeros(2 * dimensions, 1); % target: indices (1:dimensions) correspond to position, indices ((dimensions+1):2*dimensions) are velocity
            filter.target = target; 

            [ A, B, Q, R, L, K ] = trajectory(dimensions, delta, target);
            % Plant is
            % x_{k+1} = A * x_{k} + B * u_{k}
            filter.A = A; % Constant matrix from trajectory
            filter.B = B; % Constant matrix from trajectory
            % Given quadratic kinematic (position and velocity) cost Q and quadratic control cost R, optimal control policy is
            % u_{k} = L * x_{k}
            filter.L = L;

            %% Initial Covariance Matrix
            % Covariance matrix of neural parameters
            % Currently multiple of identity
            % Could be changed to have higher/lower variance for affine term
            parameter_variance = 10;
            covariance_neural = parameter_variance * eye((dimensions + 1) * features); % TODO: more systematic choice than identity?
            % Covariance matrix for kinematics
            position_var = 1e-7;
            velocity_var = 1e-7;
            covariance_cursor = blkdiag(position_var * eye(dimensions), velocity_var * eye(dimensions));
            % Neural parameters and kinematics have zero covariance between each other
            filter.covariance = blkdiag(covariance_neural, covariance_cursor);

            % Placeholder channel variance
            baseVariance = 1.0; % Variance of channels (should load later from calibration file)
            filter.channelVariances = baseVariance * ones(features, 1);

            %% Extra values to log
            filter.extra_parameter_names = {'innovation', 'obs', 'estimated_obs'};
            % TODO: hand position, target, score
        end

        % function that is called every iteration when new
        % feature values are received from the feature extractor
        function [ control ] = RunFilter(filter, recvdFeatures)
            %% Conversion from imitator mapping
            %obs = (recvdFeatures / filter.imitatorAmplifier) .^ 2 - filter.imitatorBaseline;
            obs = recvdFeatures;

            %% Copy filter variables for convenience
            dimensions = filter.dimensions;
            features = filter.features;
            A = filter.A;
            B = filter.B;
            L = filter.L;
            target = filter.target;

            %% Prediction Step
            x = cell2mat(filter.parameterValues)'; % convert neural parameters, position, velocity to mat from cell

            % Log filter position
            filter_position = x(((dimensions + 1) * features) + (1:dimensions));
            filter.Position = [filter.Position filter_position];
            filter_position = filter.Position;
            save('filter.mat', 'filter_position');

            F_neural = eye((dimensions + 1) * features); % State update model for neural parameters (identity - do not change parameters)
            Q_neural = zeros((dimensions + 1) * features); % Zero increment covariance for neural parameters

            F_cursor = A + B * L; % State update model for neural parameters when training
            delta_p = 1e-3; % increment variance for position
            delta_v = 1e-3; % increment variance for velocity
            Q_cursor = blkdiag(delta_p * eye(dimensions), delta_v * eye(dimensions)); % increment uncertainty in velocity

            % Combine state update model and increment covariance for neural parameters and (position and velocity)
            F = blkdiag(F_neural, F_cursor);
            Q = blkdiag(Q_neural, Q_cursor);

            % Linear shift for target when training
            b = [zeros((dimensions + 1) * features, 1);-B * L * target];

            pred_x = F * x + b; % predict the neural parameters, position, and velocity in next time step

            % DEBUG For Control Policy
            %err = target - x(((dimensions + 1) * features + 1):end);
            %vel = err(1:dimensions) / 10;
            %pred_x((end-dimensions+1):end) = vel;
            % END DEBUG

            % Reset covariance related to cursor kinematics to zero
            filter.covariance(((dimensions + 1) * features + 1):end, :) = 0;
            filter.covariance(:, ((dimensions + 1) * features + 1):end) = 0;
            pred_cov = F * filter.covariance * F' + Q; % Predict covariance matrix at next time step

            %% Update Step
            pred_uHistory = [pred_x((dimensions + 1) * features + dimensions + 1:end); 1]; % extract predicted velocity and append affine term
            channelParameters = cell2mat(reshape(filter.parameterValues(1:(dimensions + 1) * features), features, dimensions + 1)); % extract neural parameters
channelParameters
            estimated_obs = channelParameters * pred_uHistory; % affine observation equation
            filter.estimated_obs = estimated_obs;

            % derivative of the observation vector with respect to the state, evaluated at estimated_obs.
            D_obs = zeros(size(pred_x, 1), features);
            for c = 1:features
                % velocity terms
                for i = 1:dimensions
                    % derivative in terms of neural parameter is velocity
                    D_obs((i - 1) * features + c, c) = pred_uHistory(i, 1);
                    % derivative in terms of velocity is neural parameter
                    D_obs(size(D_obs, 1) - dimensions + i, c) = pred_x((i - 1) * features + c, 1);
                end

                % affine terms
                D_obs(features * dimensions + c, c) = 1;
            end

            % double derivative of the observation vector with respect to the state, evaluated at estimated_obs.
            DD_obs = zeros(size(pred_x, 1), size(pred_x, 1), features);
            for c = 1:features
                for i = 1:dimensions
                    % Velocity terms
                    % derivative in terms of neural parameter and then velocity is 1
                    DD_obs((i - 1) * features + c, size(DD_obs, 2) - dimensions + i, c) = 1;
                    % same for swapping order of derivatves
                    DD_obs(size(DD_obs, 2) - dimensions + i, (i - 1) * features + c, c) = 1;

                    % Affine terms
                    % Nothing to do
                    % Taking two derivatives will always result in a 0 (regardless of which two are selected)
                end
            end

            channelVariances = filter.channelVariances;

            %% compute summation for updating covariance
            cov_adjust = zeros(size(pred_cov));
            for c = 1:features
                if (~isnan(estimated_obs(c, 1)))
                    cov_adjust = cov_adjust + 1 / channelVariances(c) * ...
                                              (D_obs(:, c) * D_obs(:, c)' + ...
                                              (estimated_obs(c, 1) - obs(c)) * DD_obs(:, :, c));
                else
                    assert(false)
                end
            end
            new_cov_inv = inv(pred_cov) + cov_adjust; % compute inverse of updated covariance matrix
            new_cov = inv(new_cov_inv); % compute updated covariance matrix
            filter.covariance = new_cov; % Store new covariance matrix

            %% compute summation for updating state
            x_adjust = zeros(size(pred_x));
            innovation = zeros(features, 1);
            for c = 1:features
                if(~isnan(estimated_obs(c, 1)))
                    innovation(c, 1) = (obs(c) - estimated_obs(c, 1));
                    x_adjust = x_adjust + 1 / channelVariances(c) * innovation(c, 1) * D_obs(:, c);
                end
            end
            filter.innovation = innovation; % store innovation
            filter.obs = obs;
            new_x = pred_x + new_cov * x_adjust; % compute updated state
            filter.parameterValues = mat2cell(double(new_x), ones((dimensions + 1) * features + 2 * dimensions, 1), 1)'; % store updated state

            %% grab predicted velocity as control signal
            control = new_x((dimensions + 1) * features + dimensions+1:end);
            assert(numel(control) == dimensions);
        end

        function [ ] = setHandPos(filter, hand)
            % Copy new hand position for each dimension
            for i = 1:filter.dimensions
                filter.parameterValues{(filter.dimensions + 1) * filter.features + i} = hand(i);
            end
        end

        function [] = load_baseline(filter)
            baseline = load('../../feature_extraction/feature_extract_cpp/build/baseline.txt');
            parameter_mean = baseline(:, 1);
            parameter_variance = baseline(:, 2);
            % Copy mean into channel parameters
            for i = 1:filter.features
                filter.parameterValues{filter.dimensions * filter.features + i} = parameter_mean(i);
            end
            filter.channelVariances = parameter_variance;
        end
    end
end

