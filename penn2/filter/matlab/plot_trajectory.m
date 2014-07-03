function [ f ] = plot_trajectory(dimensions, delta, target, radius, number)
% Run
% >> plot_trajectory(1, 0.1, [0;0], 1, 10)
% >> plot_trajectory(2, 0.1, [0;0;0;0], 1, 10)
% >> plot_trajectory(3, 0.1, [0;0;0;0;0;0], 1, 10)
  % plots two dimensions
  f = figure;
  if (dimensions == 1)
    hold on;
  elseif (dimensions == 2)
    theta = 0:0.01:2*pi;
    x = target(1) + radius * cos(theta);
    y = target(2) + radius * sin(theta);
    plot(x, y, 'LineWidth', 3, 'Color', 'black');
    hold on;
  elseif (dimensions == 3)
    [x y z] = sphere(1024);
    h = surfl(x, y, z); 
    set(h, 'FaceAlpha', 1.0)
    shading interp
  else
    assert(false);
  end

  [A,B,Q,R,L,K] = trajectory(dimensions, delta, target);

  for iteration = 1:number
    if (dimensions == 1)
      x0 = [1;
            0];
    elseif (dimensions == 2)
      theta = 2 * pi * rand();
      x0 = [radius * cos(theta);
            radius * sin(theta);
            0;
            0];
    elseif (dimensions == 3)
      x0 = [normrnd(0,1,[3,1]);
            zeros(3,1)];
      x0 = x0 / norm(x0);
    end
    x = x0;
    X = [];
    for i = 1:100
      X = [X x(1:dimensions)];
      x = A * x + B * (L * (x - target)) + normrnd(0, 0.02, [2 * dimensions, 1]);
    end
    if (dimensions == 1)
      scatter(1:100,X(1,:));
    elseif (dimensions == 2)
      scatter(X(1,:), X(2,:));
    elseif (dimensions == 3)
      scatter3(X(1,:), X(2,:), X(3,:));
      hold on;
    end
  end

  if (dimensions == 1)
    title('1D Trajectories');
    xlabel('Time Step');
    ylabel('x');
  elseif (dimensions == 2)
    xlim(target(1) + 1.1 * radius * [-1 1]);
    ylim(target(2) + 1.1 * radius * [-1 1]);
    axis square;
    xlabel('x');
    ylabel('y');
    title('2D Trajectories');
  elseif (dimensions == 3)
    xlim(target(1) + 1.1 * radius * [-1 1]);
    ylim(target(2) + 1.1 * radius * [-1 1]);
    zlim(target(2) + 1.1 * radius * [-1 1]);
    axis square;
    xlabel('x');
    ylabel('y');
    zlabel('z');
    title('3D Trajectories');
    grid on
  end
end

