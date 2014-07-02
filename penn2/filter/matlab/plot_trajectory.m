function [ f ] = plot_trajectory(delta, target, radius, number)
  % plots two dimensions
  f = figure;
  theta = 0:0.01:2*pi;
  x = target(1) + radius * cos(theta);
  y = target(2) + radius * sin(theta);
  plot(x, y, 'LineWidth', 3, 'Color', 'black');
  hold on;

  [A,B,Q,R,L,K] = trajectory(2, delta, target);

  for iteration = 1:number
    theta = 2 * pi * rand();
    start = [radius * cos(theta);
             radius * sin(theta);
             0;
             0];
    x = start;
    X = [];
    Y = [];
    for i = 1:100
      X = [X x(1)];
      Y = [Y x(2)];
      x = A * x + B * (L * (x - target)) + normrnd(0, 0.02, [4, 1]);
    end
    scatter(X, Y);
  end

  xlim(target(1) + 1.1 * radius * [-1 1]);
  ylim(target(2) + 1.1 * radius * [-1 1]);
  axis square;
end

