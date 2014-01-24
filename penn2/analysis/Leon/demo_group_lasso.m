%
%    Demo code for group lasso using shooting algorithm 
%    (i.e., coordinate descent)
%    
%    Copyright, Gunhee Kim (gunhee@cs (dot) cmu (dot) edu)
%    Computer Science Department, Carnegie Mellon University, 
%    January 14 2010
%
%    Please email me if you find any bugs in the code. 
%

% load data
load data.mat;

% lasso parameter
lambda = 10;

% run the group lasso
%
% 1. Input
% Y:        responses ([n x 1], n: # of data)
% X:        predictor variables ([n x p], p: dimension) 
% group:    group indicatior ([1 x J] cell, J: # of groups)
%           Each cell contains the column indices of X which are in the same group.
% lambda:   parameter
%
% 2. Output
% beta:         coefficients ([p x 1])
% beta_hist:    beta history
[beta, beta_hist] = LassoShootingGroup(X, Y, group, lambda) ;

beta