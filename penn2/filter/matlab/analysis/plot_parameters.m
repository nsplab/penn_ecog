function [f] = plot_parameters(filter_log_file, parameter_name, a, b, c, d)
  % for i = 1:3; for j = 1:4; ['f' int2str(i) 'b' int2str(j)], plot_parameters('../../data/filter_log_JointRSE_07_07_2014_22:47.mat', ['f' int2str(i) 'b' int2str(j)], 3, 4, i, j); end; end
  % plot_parameters('/home/bryanhe/penn_ecog/penn2/data/subject_8_@_07_03_2014/filter_log_Matlab_1_07_03_2014_12:34.mat', 'alpha')
  % plot_parameters('/home/bryanhe/penn_ecog/penn2/data/subject_8_@_07_03_2014/filter_log_Matlab_1_07_03_2014_12:54.mat', 'alpha')
  % plot_parameters({'/home/bryanhe/penn_ecog/penn2/data/subject_8_@_07_03_2014/filter_log_Matlab_1_07_03_2014_16:49.mat','/home/bryanhe/penn_ecog/penn2/data/subject_8_@_07_03_2014/filter_log_Matlab_1_07_03_2014_16:50.mat','/home/bryanhe/penn_ecog/penn2/data/subject_8_@_07_03_2014/filter_log_Matlab_1_07_03_2014_16:53.mat'}, 'alpha')
  % plot_parameters({'/home/bryanhe/penn_ecog/penn2/data/subject_8_@_07_03_2014/filter_log_Matlab_1_07_03_2014_18:59.mat','/home/bryanhe/penn_ecog/penn2/data/subject_8_@_07_03_2014/filter_log_Matlab_1_07_03_2014_19:00.mat','/home/bryanhe/penn_ecog/penn2/data/subject_8_@_07_03_2014/filter_log_Matlab_1_07_03_2014_19:01.mat'}, 'alpha')
  if strcmp(class(filter_log_file), 'char')
    filter_log_file = {filter_log_file};
  end
  %f = figure;
  subplot(a, b, (c - 1) * b + d);
  title(parameter_name);
  xlabel('iteration')
  ylabel('parameter value')
  start = zeros(1, numel(filter_log_file) + 1)
  for i = 1:numel(filter_log_file)
    %i
    data = load(filter_log_file{i});
    %data
    c = struct2cell(data);
    %c
    %c{1}
    names = c{1}.parameter_names;
    index = -1;
    for j = 1:numel(names)
      if (strcmp(names(j), parameter_name))
        index = j;
        break;
      end
    end
    index
    x = 1:numel(c);
    y = zeros(1, numel(c) - 1);
    for j = 2:numel(c)
      %c{j}
      y(j) = c{j}.parameter_values{index}(1)
    end
    scatter(start(i) + x, y);
    hold on;
    start(i + 1) = start(i) + numel(c);
  end

  yl = ylim;
  for i = 2:numel(filter_log_file)
    plot([start(i) start(i)], yl, 'LineWidth', 3, 'Color', 'black');
  end
  start;
end

