function [L, K, iterations] = solve_ricatti(A, B, Q, R)
% From Dynamic Programming and Optimal Control, Bertsekas
% Chapter 4.1

  K = Q;
  prev_K = Inf(size(K));
  iterations = 0;
  while sum(sum((K - prev_K) .^ 2)) >= 10e-10
    prev_K = K;
    K = A' * (K - K * B * inv(B' * K * B + R) * B' * K) * A + Q; % eq 4.5
    iterations = iterations + 1;
  end
  L = -inv(B' * K * B + R) * B' * K * A; % eq 4.6

end

