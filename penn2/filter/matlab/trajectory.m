function [ A, B, Q, R, L, K ] = trajectory(dimensions, delta, target)
% dimensions: number of dimensions
% delta: time step

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

  Q = eye(2 * dimensions); % penalty on position / velocity
  R = eye(dimensions); % penalty on acceleration

  [L, K] = solve_ricatti(A, B, Q, R);

  % Optimal control is
  % u_k = A x_k + B = A(x_k - target)
  %A = L;
  %B = -L * target;
end

