%Goodness of fit for different delay times
%desired_frequencies = [12:42];
clear y_value r_squared
desired_chan = 3:11;
plot_low = 1000;
plot_high = 1100;
%desired_reg_frequencies = get_variables('beta'); %Get the desired frequencies to analyze in the regression
%desired_reg_frequencies = get_variables('High Gamma'); %Get the desired frequencies to analyze in the regression
frequency_matrix = zeros(size(desired_chan), length(T_axis)); %matrix that has channels in the rows and time in the X, to plot averaged frequencies
    for chan_idx = desired_chan
        chan_power_mat = large_power_matrix(:,(chan_idx-1)*length(F)+1:chan_idx*length(F)); %extract the info for the current channel
        channel_frequency = extract_frequency(chan_power_mat, F, desired_frequencies, 'average'); %extract the frequencies that we want
        frequency_matrix(chan_idx,:) = channel_frequency;%assigns those frequencies to the channel and generate a new features vector
    end
count_idx = 1;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%This plots the normal regression effects of
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%the offset
count_idx = 1;
for offset_time = 0.0:0.01:1 %iterate over seconds
    [reg_large_labels, reg_frequency_matrix] = offset_label(frequency_matrix', large_labels, offset_time, 'backward');%add an offset to the labels
    [b,dev,stats] = glmfit(reg_frequency_matrix ,reg_large_labels'); % Logistic regression
    deviance(count_idx) = dev;
    y_value(count_idx) = offset_time;
    count_idx = count_idx + 1;
end
fig1 = figure('visible','off');
hold on
plot(y_value, deviance, '-bo', 'color', [0.5,0.5,0.5],'LineWidth', 2, 'MarkerSize', 4, 'MarkerEdgeColor', 'k','MarkerFaceColor', [0.8,0.8,0.8])
ylim([plot_low, plot_high])
xlabel('Offset Time [s]')
ylabel('Deviance value')
title('Backward Goodness of fit for different values of offset between the labels and the features')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],[num2str(desired_frequencies(1)) '_to_' num2str(desired_frequencies(end))  'regression_r_value_forward.png'])
count_idx = 1;
%%%This plots the forward offsets with the labels
for offset_time = 0.0:0.01:1 %iterate over seconds
    [reg_large_labels, reg_frequency_matrix] = offset_label(frequency_matrix', large_labels, offset_time, 'forward');%add an offset to the labels
    [b,dev,stats] = glmfit(reg_frequency_matrix ,reg_large_labels'); % Logistic regression
    deviance(count_idx) = dev;
    y_value(count_idx) = offset_time;
    count_idx = count_idx + 1;
end
fig2 = figure('visible','off');
plot(y_value, deviance, '-bo', 'color', [0.5,0.5,0.5],'LineWidth', 2, 'MarkerSize', 4, 'MarkerEdgeColor', 'k','MarkerFaceColor', [0.8,0.8,0.8])
xlabel('Offset Time [s]')
ylabel('Deviance value')
ylim([plot_low, plot_high])
title('Forward Goodness of fit for different values of offset between the labels and the features')
set(gca,'XDir','Reverse')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],[num2str(desired_frequencies(1)) '_to_' num2str(desired_frequencies(end)) 'regression_r_value_backward.png'])
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%This plots the PCA regression effects in the offset
count_idx = 1;
for offset_time = 0.0:0.01:1 %iterate over seconds
    [reg_large_labels, reg_pca_data] = offset_label(pca_data, large_labels, offset_time, 'backward');%add an offset to the labels
    [b,dev,stats] = glmfit(reg_pca_data ,reg_large_labels'); % Logistic regression
    deviance(count_idx) = dev;
    y_value(count_idx) = offset_time;
    count_idx = count_idx + 1;
end
fig3 = figure('visible','off');
plot(y_value, deviance, '-bo', 'color', [0.5,0.5,0.5],'LineWidth', 2, 'MarkerSize', 4, 'MarkerEdgeColor', 'k','MarkerFaceColor', [0.8,0.8,0.8])
xlabel('Offset Time [s]')
ylabel('Deviance value')
ylim([plot_low, plot_high])
title('Backward Goodness of fit for different values of offset between the labels and the PCA components')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],[num2str(desired_frequencies(1)) '_to_' num2str(desired_frequencies(end))  'pca_regression_r_value_forward.png'])

count_idx = 1;
%%%This plots the forward offsets with the labels
for offset_time = 0.0:0.01:1 %iterate over seconds
    [reg_large_labels, reg_pca_data] = offset_label(pca_data, large_labels, offset_time, 'forward');%add an offset to the labels
    [b,dev,stats] = glmfit(reg_pca_data ,reg_large_labels'); % Logistic regression
    deviance(count_idx) =dev;
    y_value(count_idx) = offset_time;
    count_idx = count_idx + 1;
end
fig4 = figure('visible','off');
plot(y_value, deviance, '-bo', 'color', [0.5,0.5,0.5],'LineWidth', 2, 'MarkerSize', 4, 'MarkerEdgeColor', 'k','MarkerFaceColor', [0.8,0.8,0.8])
xlabel('Offset Time [s]')
ylabel('Deviance value')
ylim([plot_low, plot_high])
set(gca,'XDir','Reverse')
title('Forward Goodness of fit for different values of offset between the labels and the PCA components')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],[num2str(desired_frequencies(1)) '_to_' num2str(desired_frequencies(end)) 'pca_regression_r_value_backward.png'])
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%This plots the PLS regression effects in the offset
count_idx = 1;
for offset_time = 0.0:0.01:1 %iterate over seconds
    [reg_large_labels, reg_pls_data] = offset_label(XS, large_labels, offset_time, 'backward');%add an offset to the labels
    [b,dev,stats] = glmfit(reg_pls_data ,reg_large_labels'); % Logistic regression
    deviance(count_idx) = dev;
    y_value(count_idx) = offset_time;
    count_idx = count_idx + 1;
end
fig5 = figure('visible','off');
plot(y_value, deviance, '-bo', 'color', [0.5,0.5,0.5],'LineWidth', 2, 'MarkerSize', 4, 'MarkerEdgeColor', 'k','MarkerFaceColor', [0.8,0.8,0.8])
xlabel('Offset Time [s]')
ylabel('Deviance value')
ylim([plot_low, plot_high])
title('Backward Goodness of fit for different values of offset between the labels and the PLS components')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],[num2str(desired_frequencies(1)) '_to_' num2str(desired_frequencies(end)) 'pls_regression_r_value_forward.png'])

count_idx = 1;
%%%This plots the forward offsets with the labels
for offset_time = 0.0:0.01:1 %iterate over seconds
    [reg_large_labels, reg_pls_data] = offset_label(XS, large_labels, offset_time, 'forward');%add an offset to the labels
    [b,dev,stats] = glmfit(reg_pls_data ,reg_large_labels'); % Logistic regression
    deviance(count_idx) = dev;
    y_value(count_idx) = offset_time;
    count_idx = count_idx + 1;
end
fig6 = figure('visible','off');
plot(y_value, deviance, '-bo', 'color', [0.5,0.5,0.5],'LineWidth', 2, 'MarkerSize', 4, 'MarkerEdgeColor', 'k','MarkerFaceColor', [0.8,0.8,0.8])
xlabel('Offset Time [s]')
ylabel('Deviance value')
set(gca,'XDir','Reverse')
ylim([plot_low, plot_high])
title('Forward Goodness of fit for different values of offset between the labels and the PLS components')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],[num2str(desired_frequencies(1)) '_to_' num2str(desired_frequencies(end)) 'pls_regression_r_value_backward.png'])



