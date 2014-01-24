function [beta, beta_hist, mse_error] = LassoShootingGroup(X, y, Group, lambda, param)
% run the group lasso
%
%   1. Input
%   Y:        responses ([n x 1], n: # of data)
%   X:        predictor variables ([n x p], p: dimension) 
%   group:    group indicatior ([1 x J] cell, J: # of groups)
%             Each cell contains the column indices of X which are in the same group.
%   lambda:   lasso parameter
%
%   2. Output
%   beta:        optimized coefficients ([p x 1])
%   beta_hist:   beta history
%
%   Copyright, Gunhee Kim (gunhee@cs (dot) cmu (dot) edu)
%   Computer Science Department, Carnegie Mellon University, 
%   January 14 2010
%


% Optimization parameters
if nargin < 5,
    param.maxIter = 10000;
    param.tol = 1e-6;
end

[n p] = size(X);
J = length(Group) ;
beta_hist = zeros(p, param.maxIter) ;

% Make mean 0 to eliminate intercept
X = X - repmat(mean(X,1), [size(X,1) 1]) ;
y = y - repmat(mean(y,1), [size(y,1) 1]) ;

% Orthonormalization
for i=1:J
    X(:,Group{i}) = GSOrth(X(:,Group{i})) ;
end


% This is not necessary.
% Any initialization would be fine. 
% Start from the Least Squares solution
beta = (X'*X + lambda*eye(p))\(X'*y);

% Main loop
iter = 0;
while iter < param.maxIter
    
    beta_prev = beta;
    
    % See the eq.(2.4) of Yuan & Lin's paper (2006). 
    for j = 1:J
        % \beta_{-j}
        beta_tilt = beta ; 
        beta_tilt(Group{j}) = 0 ;
        
        % Sj
        Sj = X(:,Group{j})' * (y - (X*beta_tilt)) ;
        tmp_beta = (1-lambda*sqrt(length(Group{j}))/norm(Sj)) ;
        if tmp_beta<0, tmp_beta= 0 ; end
        
        % new beta
        beta(Group{j}) = tmp_beta * Sj ;
        
    end
    iter = iter + 1;
    beta_hist(:,iter) = beta;
    
%     disp(['iteration: ' num2str(iter, '%.4d'), ', error = ' num2str(sum((X*beta-y).^2)+lambda*sum(abs(beta))), ...
%         ', diff_beta:' num2str(sum(abs(beta-beta_prev))) ]) ;

    % Check termination
    if sum(abs(beta-beta_prev)) < param.tol
        break;
    end
end

disp(['Done! Number of iterations: ' num2str(iter) '.']);
mse_error = sum((X*beta-y).^2)+lambda*sum(abs(beta)); 
beta_hist = beta_hist(:,1:iter) ;

