function [f] = plot_parameters(filter_log_file, parameter_name)
  % plot_parameters('/home/bryanhe/penn_ecog/penn2/data/subject_8_@_07_03_2014/filter_log_Matlab_1_07_03_2014_12:34.mat', 'alpha')
  % plot_parameters('/home/bryanhe/penn_ecog/penn2/data/subject_8_@_07_03_2014/filter_log_Matlab_1_07_03_2014_12:54.mat', 'alpha')
  data = load(filter_log_file);
  c = struct2cell(data);
  names = c{1};
  %find(names, parameter_name)
  % TODO: search for parameter_name in names
  index = 1;
  x = 1:numel(c);
  y = zeros(1, numel(c) - 1);
  for i = 2:numel(c)
    y(i) = c{i}{index};
  end
  f = figure;
  scatter(x, y);
end
