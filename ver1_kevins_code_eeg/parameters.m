function parMap = parameters()

parMap = containers.Map();

% -------- PARAMETERS FOR THE EXPERIMENT --------
% This is the number of parameter sets there are.
parMap('nParSets') = 5;

% This is the number of training runs per parameter set, in total.
parMap('nRunsPerParSet') = 50;

% -------- PARAMETERS FOR THE SYSTEM --------

% This determines whether the system uses open- or closed-loop control.
% Open-loop control is currently only supported for the LQR models of the
% human, not the TD model. Use 'openLoop' for open-loop control and
% 'closedLoop' for closed-loop.
parMap('systemControl') = 'closedLoop';

% This is the number of sets of trials that will be run.
parMap('nSets') = 1;

% This is the number of trials in each set of trials.
parMap('nTrials') = 50;

% This list of 0's and 1's determines which of the trials will use the
% cursor-only point process filter rather than the grand state point
% process filter. A 0 represents the grand state filter and a 1 represents
% the cursor-only filter. Currently, the cursor-only PPF only uses the
% random walk prior, though it could be modified to allow the reach state
% equaiton prior as well.
timeBetweenTrials = 5;
testTrials = repmat([zeros(1, timeBetweenTrials - 1) 1], ...
    [1 ceil(parMap('nTrials') / timeBetweenTrials)]);
parMap('testTrials') = testTrials(1:parMap('nTrials'));
% parMap('testTrials') = [1 testTrials(1:parMap('nTrials'))];


% This is the radius of the neighborhood of the target that the cursor must
% stay in.
parMap('targetRadius') = 0.05;

% This is the number of simulated neurons the point process filter will get
% spiking data from.
parMap('nNeurons') = 25;

% This is the length of each time step, in seconds.
parMap('timeBin') = 1 / 30; % This is the sampling rate of the Kinect.

parMap('maxKinectTimeBin') = 1.1 * parMap('timeBin');

% This is the max length in time steps (not seconds) of each trial.
parMap('maxTrialLength') = ceil(3 / parMap('timeBin'));

% This is the length of time, in time steps, that the cursor must be in the
% neighborhood of the target before the trial ends early.
parMap('holdLength') = ceil(0.5 / parMap('timeBin'));

% This is the list that determines when the human LQR becomes inattentive
% in each of the trials, if at all. For a trial number 'n',
%    inattentionTrials(n) = t if the human becomes inattentive starting at
%                             trial 't'. If 't' is greater than 
%                             'maxTrialLength', then the human never
%                             becomes inattentive.
parMap('inattentionTrials') = (parMap('maxTrialLength') + 1) * ...
    ones(1, parMap('nTrials'));

% This is the list that determines where the waypoint for the human LQR is
% located, if it has one. For a trial number 'n',
%     waypointTrials(:, n) = [x; y] where '[x; y]' is the position of the
%                                   waypoint.

% This is how far away the cursor will start from the target, in meters.
parMap('startMag') = 0.2;

% This is the target position the human wants to move towards. 
parMap('targetPos') = [0; 0];

% This is the target velocity the human wants to move towards. 
parMap('targetVel') = [0; 0];

% The target time, in seconds.
parMap('targetTime') = 2;

% Use 'directed' for the reach state equation prior, 'undirected' for the
% random walk prior, 'cursorGoal' for the cursorGoal prior, 'nolearning'
% for the point process filter to have fixed neuron parameter estimates,
% 'lockstepRSE' for the reach state equation prior with sequential decoding
% of neuron parameters and cursor kinematics (with a RSE prior), and
% 'lockstepRandomWalk' for the reach state equation prior with sequential
% decoding of neuron parameters and cursor kinematics (with a random walk
% prior).
parMap('computerPrior') = 'nolearning';

% Possible choices are 'TD', 'smartLQR', 'simpleLQR', and 'kinect'.
parMap('humanControlPolicy') = 'kinect';

% -------- PARAMETERS FOR THE POINT PROCESS FILTER --------
% Use 'cartesian' for cartesian neuron parameters and 'polar' for polar
% neuron parameters.
parMap('ppfCoordinateSystem') = 'cartesian';

% This determines how the PPF generates its initial estimates for the
% neuron parameters. For the direction parameter of each neuron, the PPF
% selects an element from the uniform distribution on [0, 2pi), and for the
% constant parameter and magnitude parameter, it depends on the value of
% 'magnitudeGeneration'.
%     'random': We select a base firing rate in spikes/second from the
%               uniform distribution on [10, 20], and a maximum firing rate
%               from [25, 40]. The constant parameter and magnitude
%               parameter are then chosen to produce the desired base and
%               max firing rates. Details are given in the documentation
%               for the function 'generateNeuralParameters'.
%     'mean'  : The constant and magintude parameters are set to their mean
%               values over the distribution generated by the 'random'
%               method.
%     'custom': Each base firing rate is set to '10 * baseFactor + 10' and
%               each max firing rate is set to '15 * maxFactor + 25'. The
%               parameters are then generated as in the 'random' method.
% In particular, note that 'baseFactor' and 'maxFactor' are only used if
% the parameter generation method is set to 'custom'.
parMap('magnitudeGeneration') = 'custom';
parMap('baseFactor') = 0.5;
parMap('maxFactor') = 0.5;
parMap('directionGeneration') = 'random';
parMap('dirList') = ones(parMap('nNeurons'), 3);

% This determines whether the covariance on the kinematic terms of the
% state are reset at every time step to their initial values. 
parMap('covReset') = 'yes';

% REACH STATE EQUATION PARAMETERS
% - Note that these are only used if the prior is set to 'directed'.

% This is the number of time steps in the reach.
parMap('reachTimeSteps') = ceil(parMap('targetTime') / parMap('timeBin'));

% This is the target for the reach.
reachTarget = [0; 0; 0; 0];

% This is the covariance matrix on the target.
targetCov = [ [1e-6 0 0 0];
              [0 1e-6 0 0];
              [0 0 1e-8 0];
              [0 0 0 1e-8] ];

% This is the covariance on the state evolution of the reach.
xIntentVar = 1e-3;
yIntentVar = xIntentVar;
xPosVar = 0;
yPosVar = xPosVar;
Q_reach = [ [xPosVar 0 0 0]; 
             [0 yPosVar 0 0];
             [0 0 xIntentVar 0];
             [0 0 0 yIntentVar] ];
         
% The unconstrained state evolution matrix. It advances the position by
% the intended velocity * timeBin.
F_reach = [ [1 0 parMap('timeBin') 0                ];
            [0 1 0                 parMap('timeBin')];
            [0 0 1                 0                ];
            [0 0 0                 1                ] ];
        
[f_prefix, new_inc_variance, phi_t_T] = ...
    reach_state_equation_parameters_test(parMap('reachTimeSteps'), F_reach, ...
    Q_reach, reachTarget, targetCov);

% OTHER PARAMETERS
% The initial covariance matrices for the parameters of each neuron.
neuronCov = cell(parMap('nNeurons'), 1);
for i = 1:parMap('nNeurons')
   neuronCov{i} = 1 * eye(3);
%    neuronCov{i} = [ [1 0 0];
%                     [0 1 0];
%                     [0 0 1]];
end
parMap('neuronCov') = neuronCov;

% The initial covariance matrix for the intended cursor velocity and cursor
% position.
cursorCov = [ [1e-7 0 0 0];
              [0 1e-7 0 0];
              [0 0 1e-7 / parMap('timeBin') 0];
              [0 0 0 1e-7 / parMap('timeBin')] ];
% cursorCov = [ [1e-7 0 0 0];
%               [0 1e-7 0 0];
%               [0 0 0.5 0];
%               [0 0 0 0.5] ];
parMap('cursorCov') = cursorCov;

% The neural parameter state evolution matrix, which evolves the parameter
% state '[a; b; c]'. It leaves all of the parameters constant.
parMap('F_neuron') = eye(3);

% The cursor state evolution matrix. It advances the position by the
% intended velocity * timeBin if the prior is 'undirected', or uses the
% appropriate matrices if the prior is 'directed'
parMap('F_cursorUndirected') = [ [1 0 parMap('timeBin') 0                ];
                                 [0 1 0                 parMap('timeBin')];
                                 [0 0 1                 0                ];
                                 [0 0 0                 1                ] ];
F_cursor = repmat(F_reach, [1 1 parMap('reachTimeSteps')]) - ...
    f_prefix(:, :, 2:end);
parMap('F_cursorDirected') = cat(3, F_cursor, repmat(F_reach, ...
    [1 1 parMap('maxTrialLength') - parMap('reachTimeSteps')]));
% The constant term in the cursor state evolution equation. If the prior is
% undirected, this term is 0.
%if(strcmp(parMap('computerPrior'), 'undirected'))
    parMap('b_cursorUndirected') = [0; 0; 0; 0];
%elseif(strcmp(parMap('computerPrior'), 'directed'))
    parMap('b_cursorDirected') = [0; 0; 0; 0];
    
    % This is the actual formula, but since the reach target is always
    % '[0; 0; 0; 0]', the above formula is easier to deal with.
    % parMap('b_cursor') = f_prefix(:, :, 2:end) .* phi_t_T * reachTarget;
%end


% The covariances on the noise terms in the state evolution equation for
% the neural parameters and the cursor.

constantParameterVar = 0;
xParameterVar = 0;
yParameterVar = 0;
Q_neuron = [ [xParameterVar 0 0]; 
             [0 yParameterVar 0];
             [0 0 constantParameterVar] ];
parMap('Q_neuron') = Q_neuron;

%if(strcmp(parMap('computerPrior'), 'undirected'))
    xIntentVar = 1e-3;
    yIntentVar = xIntentVar;
    xPosVar = 0;
    yPosVar = xPosVar;          
    Q_cursor = [ [xPosVar 0 0 0]; 
                 [0 yPosVar 0 0];
                 [0 0 xIntentVar 0];
                 [0 0 0 yIntentVar] ];
    parMap('Q_cursorUndirected') = Q_cursor;
%elseif(strcmp(parMap('computerPrior'), 'directed'))
    Q_cursor = new_inc_variance(:, :, 2:end);
    parMap('Q_cursorDirected') = cat(3, Q_cursor, repmat(Q_reach, ...
        [1 1 parMap('maxTrialLength') - parMap('reachTimeSteps')]));
%end

% PARAMETERS FOR THE DISCRETE STATE ESTIMATOR
stayAttentiveProb = 0.8;
stayInattentiveProb = 0.8;
parMap('discreteStateTransitions') = ...
    [ [stayInattentiveProb   1 - stayInattentiveProb];
      [1 - stayAttentiveProb stayAttentiveProb      ] ];
  
parMap('initialDiscreteStateProbDist') = [0.001; 0.999];

% -------- PARAMETERS FOR THE HUMAN CONTROLLER --------

% PARAMETERS FOR THE HUMAN LQR CONTROLLER
% The state evolution matrix.
parMap('lqrStateMat') = [ [1 0 parMap('timeBin') 0                 0];
                          [0 1 0                 parMap('timeBin') 0];
                          [0 0 1                 0                 0];
                          [0 0 0                 1                 0];
                          [0 0 0                 0                 1] ];
                    
% The control matrix.
parMap('lqrControlMat') = [ [0 0];
                            [0 0];
                            [1 0];
                            [0 1];
                            [0 0] ];
                        
% The cost matrix that penalizes based on the state.                          
lqrStateCostMat = zeros(5, 5, parMap('maxTrialLength') + 1);
for timeStep = 1:parMap('maxTrialLength') + 1
    if(timeStep <= parMap('reachTimeSteps') + parMap('holdLength'))
        reachPosCost = 2 * parMap('timeBin');
        reachVelCost = 0.25 * parMap('timeBin');
        lqrStateCostMat(:, :, timeStep) = [ [reachPosCost 0 0 0 0];
                                            [0 reachPosCost 0 0 0];
                                            [0 0 reachVelCost 0 0];
                                            [0 0 0 reachVelCost 0];
                                            [0 0 0 0            0] ];
    else
        afterPosCost = 5 * reachPosCost;
        afterVelCost = reachVelCost;
        lqrStateCostMat(:, :, timeStep) = [ [afterPosCost 0 0 0 0];
                                            [0 afterPosCost 0 0 0];
                                            [0 0 afterVelCost 0 0];
                                            [0 0 0 afterVelCost 0];
                                            [0 0 0 0            0] ];
    end
end

parMap('lqrStateCostMat') = lqrStateCostMat;


% The cost matrix that penalizes based on the control.
% controlCost = 5000 * parMap('timeBin');
% controlCost = 250 * parMap('timeBin');
% controlCost = 50 * parMap('timeBin');
controlCost = 0;
parMap('lqrControlCostMat') = [ [controlCost 0          ];
                                [0           controlCost] ];

% Specific to the smart LQR:

% The initial covariance matrix for the posterior density
% on the intended cursor velocity and cursor position, and
% the covariance on the noise in the state evolution
% process.
parMap('smartLqrInitialStateCov') = [ [0 0 0 0 0]; 
                                      [0 0 0 0 0];
                                      [0 0 0 0 0];
                                      [0 0 0 0 0];
                                      [0 0 0 0 0] ];
                                  
parMap('smartLqrStateNoiseCov') = [ [0 0 0    0    0]; 
                                    [0 0 0    0    0];
                                    [0 0 1e-4 0    0];
                                    [0 0 0    1e-4 0];
                                    [0 0 0    0    0] ];
                                
% Training parameters.

% The number of different angles we execute reaches from in training. These
% positions start at an angle of 0 and are evenly distributed around a
% circle of radius 'trainStartMag'.

parMap('trainNumAngles') = 8;

% The distance from the origin our training reaches start at.
parMap('trainStartMag') = parMap('startMag');

% The number of reaches we execute from each start angle.
parMap('trainRepsPerAngle') = 3;

% The number of time steps each reach will last.
parMap('trainTrialLength') = parMap('maxTrialLength');

% PARAMETERS FOR THE HUMAN TD CONTROLLER

% This is the basis for the function approximator that is used to
% parameterize the human's control policy. It is a cell array of function
% handles.
phi1 = @(x)[1; 0];
phi2 = @(x)[0; 1];
phi3 = @(x)[x(1); 0];
phi4 = @(x)[0; x(1)];
phi5 = @(x)[x(2); 0];
phi6 = @(x)[0; x(2)];
phi7 = @(x)[x(3); 0];
phi8 = @(x)[0; x(3)];
phi9 = @(x)[x(4); 0];
phi10 = @(x)[0; x(4)];
parMap('intentBasis') = {phi1, phi2, phi3, phi4, phi5, phi6, phi7, ...
    phi8, phi9, phi10};

% This is the initial parameterization of the human's control policy. We
% initialize it to values that take the form of a "dumb" LQR.

% The state evolution matrix.
A = [ [1 0 parMap('timeBin') 0                ];
      [0 1 0                 parMap('timeBin')];
      [0 0 1                 0                ];
      [0 0 0                 1                ] ];

% The control matrix.
B = [[0 0];
     [0 0];
     [1 0];
     [0 1] ];

% The cost matrix that penalizes based on the state.
posCost = 0.25 * parMap('timeBin');
velCost = 0.0025 * parMap('timeBin');
Q = [ [posCost 0       0       0      ];
      [0       posCost 0       0      ];
      [0       0       velCost 0      ];
      [0       0       0       velCost] ];

% The cost matrix that penalizes based on the control.
controlCost = 0.0025 * parMap('timeBin');
R = [ [controlCost 0          ];
      [0           controlCost] ];
  
% NOTE: For some reason, Matlab's dlqr function returns the negative of the
% actual control matrix you should use. 
[L, K, e] = dlqr(A, B, Q, R);
L = -L;
  
% For use with the general basis.
% parMap('initialW') = [0; 0; L(1, 1); L(2, 1); L(1, 2); L(2, 2); L(1, 3); ...
%     L(2, 3); L(1, 4); L(2, 4)];

% For use with the linear basis.
parMap('initialW') = L;

% This is the variance on the noise added to the policy at each timestep.
parMap('zVar') = 0.0001;

% This is the learning rate on the human's control parameters.
parMap('wLearningFactor') = 1; %0.1;

% This is the basis for the function approximator that is used to
% parameterize the human's value function. It is a cell array of function
% handles.
psi1 = @(x)1;
psi2 = @(x)x(1);
psi3 = @(x)x(2);
psi4 = @(x)x(3);
psi5 = @(x)x(4);
parMap('valueBasis') = {psi1, psi2, psi3, psi4, psi5};

% This is the initial parameterization of the human's value function.
parMap('initialV') = zeros(size(parMap('valueBasis')))';

% This is the learning rate on the human's value parameters.
parMap('vLearningFactor') = 0.01;

% This is the human's cost function. It penalizes him based on the squared
% Euclidean distance to a target point.
posFactor = 10 * parMap('timeBin');
controlFactor = .0025 * parMap('timeBin');

cost = @(xHistory, uHistory) ...
    (posFactor * (xHistory{end}(1)^2 + xHistory{end}(2)^2) + ...
    controlFactor * sum(cellfun(@(u)sum(u.^2), uHistory)));
parMap('cost') = cost;

% This is the human's initial eligibility trace.
parMap('eligibilityTrace') = 0.01 * ones(size(parMap('initialW')));

% This is the human's discount factor on its previous estimates for the
% value function, as well as for the eligibility trace.
parMap('discountFactor') = 0.7;

% -------- PARAMETERS FOR THE HUMAN KINECT CONTROLLER --------

% This is the length of time (in seconds) that the cursor is displayed on
% the screen for before the trial begins.
parMap('timeBetweenTrials') = 2;

% This is the length of time (in seconds) of each of the long breaks the
% user is forced to take.
parMap('breakTime') = 10 * 60;

% This list of 0's and 1's determines the trial sets after which the user
% is forced to take a long break. A '0' in index 'i' of this list indicates
% that there will be no long break after trial set 'i' and a '1' indicates
% that there will be one.
timeBetweenBreaks = 6;
breakList = repmat([zeros(1, timeBetweenBreaks - 1) 1], [1 6]);
breakList(end) = 0;
parMap('breakList') = breakList;

% Which hand the human uses to control the cursor.
parMap('humanHandedness') = 'right';

% The ID used to identify each subject in the data files.
parMap('humanID') = 'simClosed';

% This is the factor that the actual hand velocity is multiplied by to get
% the intended velocity. Since the position coordinates returned by
% 'mxNiSkeleton' are in millimeters, we multiply by 0.001 to get meters.
parMap('kinectVelocityScaling') = 0.001;

% -------- PARAMETERS FOR GRAPHING --------
% This is the number of bootstrap samples used to calculate each continuous
% parameter in graph().
parMap('nBootstrapSamples') = 1000;
end


