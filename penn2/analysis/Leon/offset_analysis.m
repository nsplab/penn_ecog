%Goodness of fit for different delay times
desired_reg_frequencies = [15:40];
clear y_value r_squared
%desired_reg_frequencies = get_variables('beta'); %Get the desired frequencies to analyze in the regression
%desired_reg_frequencies = get_variables('High Gamma'); %Get the desired frequencies to analyze in the regression
frequency_matrix = zeros(num_chan, length(T_axis)); %matrix that has channels in the rows and time in the X, to plot averaged frequencies
    for chan_idx = 1:num_chan
        chan_power_mat = large_power_matrix(:,(chan_idx-1)*length(F)+1:chan_idx*length(F)); %extract the info for the current channel
        channel_frequency = extract_frequency(chan_power_mat, F, desired_reg_frequencies, 'average'); %extract the frequencies that we want
        frequency_matrix(chan_idx,:) = channel_frequency;%assigns those frequencies to the channel and generate a new features vector
    end
count_idx = 1;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%This plots the normal regression effects of
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%the offset
count_idx = 1;
for offset_time = 0.01:0.01:1 %iterate over seconds
    [reg_large_labels, reg_frequency_matrix] = offset_label(frequency_matrix', large_labels, offset_time, 'backward');%add an offset to the labels
    [b,dev,stats] = glmfit(reg_frequency_matrix ,reg_large_labels'); % Logistic regression
    r_squared(count_idx) = 1 - sum(stats.resid.^2) / sum((reg_large_labels-mean(reg_large_labels)).^2);
    y_value(count_idx) = offset_time;
    count_idx = count_idx + 1;
end
fig1 = figure('visible','off');
plot(y_value, r_squared, '-bo')
xlabel('Offset Time [s]')
ylabel('r^2 value')
title('Backward Goodness of fit for different values of offset between the labels and the features')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'regression_r_value_forward.png')

count_idx = 1;
%%%This plots the forward offsets with the labels
for offset_time = 0.01:0.01:1 %iterate over seconds
    [reg_large_labels, reg_frequency_matrix] = offset_label(frequency_matrix', large_labels, offset_time, 'forward');%add an offset to the labels
    [b,dev,stats] = glmfit(reg_frequency_matrix ,reg_large_labels'); % Logistic regression
    r_squared(count_idx) = 1 - sum(stats.resid.^2) / sum((reg_large_labels-mean(reg_large_labels)).^2);
    y_value(count_idx) = offset_time;
    count_idx = count_idx + 1;
end
fig2 = figure('visible','off');
plot(y_value, r_squared, '-bo')
xlabel('Offset Time [s]')
ylabel('r^2 value')
title('Forward Goodness of fit for different values of offset between the labels and the features')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'regression_r_value_backward.png')
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%This plots the PCA regression effects in the offset
count_idx = 1;
for offset_time = 0.01:0.01:1 %iterate over seconds
    [reg_large_labels, reg_pca_data] = offset_label(pca_data, large_labels, offset_time, 'backward');%add an offset to the labels
    [b,dev,stats] = glmfit(reg_pca_data ,reg_large_labels'); % Logistic regression
    r_squared(count_idx) = 1 - sum(stats.resid.^2) / sum((reg_large_labels-mean(reg_large_labels)).^2);
    y_value(count_idx) = offset_time;
    count_idx = count_idx + 1;
end
fig3 = figure('visible','off');
plot(y_value, r_squared, '-bo')
xlabel('Offset Time [s]')
ylabel('r^2 value')
title('Backward Goodness of fit for different values of offset between the labels and the PCA components')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'pca_regression_r_value_forward.png')

count_idx = 1;
%%%This plots the forward offsets with the labels
for offset_time = 0.01:0.01:1 %iterate over seconds
    [reg_large_labels, reg_pca_data] = offset_label(pca_data, large_labels, offset_time, 'forward');%add an offset to the labels
    [b,dev,stats] = glmfit(reg_pca_data ,reg_large_labels'); % Logistic regression
    r_squared(count_idx) = 1 - sum(stats.resid.^2) / sum((reg_large_labels-mean(reg_large_labels)).^2);
    y_value(count_idx) = offset_time;
    count_idx = count_idx + 1;
end
fig4 = figure('visible','off');
plot(y_value, r_squared, '-bo')
xlabel('Offset Time [s]')
ylabel('r^2 value')
title('Forward Goodness of fit for different values of offset between the labels and the PCA components')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'pca_regression_r_value_backward.png')
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%This plots the PLS regression effects in the offset
count_idx = 1;
for offset_time = 0.01:0.01:1 %iterate over seconds
    [reg_large_labels, reg_pls_data] = offset_label(XS, large_labels, offset_time, 'backward');%add an offset to the labels
    [b,dev,stats] = glmfit(reg_pls_data ,reg_large_labels'); % Logistic regression
    r_squared(count_idx) = 1 - sum(stats.resid.^2) / sum((reg_large_labels-mean(reg_large_labels)).^2);
    y_value(count_idx) = offset_time;
    count_idx = count_idx + 1;
end
fig5 = figure('visible','off');
plot(y_value, r_squared, '-bo')
xlabel('Offset Time [s]')
ylabel('r^2 value')
title('Backward Goodness of fit for different values of offset between the labels and the PLS components')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'pls_regression_r_value_forward.png')

count_idx = 1;
%%%This plots the forward offsets with the labels
for offset_time = 0.01:0.01:1 %iterate over seconds
    [reg_large_labels, reg_pls_data] = offset_label(XS, large_labels, offset_time, 'forward');%add an offset to the labels
    [b,dev,stats] = glmfit(reg_pls_data ,reg_large_labels'); % Logistic regression
    r_squared(count_idx) = 1 - sum(stats.resid.^2) / sum((reg_large_labels-mean(reg_large_labels)).^2);
    y_value(count_idx) = offset_time;
    count_idx = count_idx + 1;
end
fig6 = figure('visible','off');
plot(y_value, r_squared, '-bo')
xlabel('Offset Time [s]')
ylabel('r^2 value')
title('Forward Goodness of fit for different values of offset between the labels and the PLS components')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'pls_regression_r_value_backward.png')



