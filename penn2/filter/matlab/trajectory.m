function [ A, B, Q, R, L, K ] = trajectory(dimensions, delta, target)
% Parameters
%   - dimensions: number of dimensions
%   - delta: time step
%   - target: target state (column vector of dimension (2 * dimensions) - positions and then velocities)
% Return Values:
%   - A: matrix to update state
%   - B: matrix to include control into state
%   - Q: hard-coded cost on kinematics (positon and velocity)
%   - R: hard-coded cost on control

% state is [position(1);
%             ...
%           position(dimension);
%           velocity(1);
%             ...
%           velocity(dimension)]

  A = eye(2 * dimensions);
  for i = 1:dimensions
    %A(i, i) = 1; % keep old position
    A(i, i + dimensions) = delta; % add in multiple of velocity
  end

  B = zeros(2 * dimensions, dimensions);
  for i = 1:dimensions
    B(i + dimensions, i) = delta; % input is acceleration
  end

  %Q = eye(2 * dimensions); % penalty on position / velocity
  Q = blkdiag(eye(dimensions), 25 * eye(dimensions));
  R = 25 * eye(dimensions); % penalty on acceleration (control)

  [L, K] = solve_ricatti(A, B, Q, R);

  % Optimal control is
  % u_k = A x_k + B = A(x_k - target)
  %A = L;
  %B = -L * target;
end

