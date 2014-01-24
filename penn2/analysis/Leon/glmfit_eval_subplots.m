function glmfit_eval_subplots(power_matrix, large_labels, F, channel_id, fourier_sampling_rate, freq_range, norm_flag, log_flag)
%GLMFIT_EVAL_SUBPLOTS creates multiple folds to evaluate the different
%single regressions
%--------------------------------------------------------------------------
%input:
%-------------------------------------------------------------------------
%F: array with all the Frequencies represented, usually the output of
%spectrogram function in matlab
%power_matrix: The feature vector of TxD, where T is the time sample and D
%is the F features of each channel, 
%large labels: array of 1's and 0s, where a one is declared where a set
%threshold for the force sensor is set.
%limit_freq: frequency limit to analyze
%fourier_sampling_rate: is a value that represents the sampling rate of the
%power matrix and the large labels, usually reduced after calculating the
%spectrogram.
%normalized_flag: 1:normalized, 0 unnormalized
%log_flag: 1: get the log(power), 0: no processing.
rise_or_fall = 'rise';
[dataset, labels_dataset, trials] = extract_subset_trials(power_matrix, large_labels, rise_or_fall, fourier_sampling_rate, 1, 1);
main_title = ['Trials: ' num2str(trials) ', Window Size = ' num2str(get_variables('Window_Size')) ' [s], Overlap:' num2str(100*get_variables('overlap_percentage')) '%, Channel:' num2str(channel_id+64)];
if norm_flag == 1
    main_title = ['Trials: ' num2str(trials) ', Window Size = ' num2str(get_variables('Window_Size')) ' [s], Normalized, Overlap:' num2str(100*get_variables('overlap_percentage')) '%, Channel:' num2str(channel_id+64)];
end
if log_flag == 1
    main_title = ['Trials: ' num2str(trials) ', Window Size = ' num2str(get_variables('Window_Size')) ' [s], Logarithmic-scaled features, Overlap:' num2str(100*get_variables('overlap_percentage')) '%, Channel: ' num2str(channel_id+64)];
end
plot_title = ['mse_evaluation' num2str(freq_range(1)) '_to_' num2str(freq_range(end)) '.png'];
if norm_flag ==1
    plot_title = ['normalized_mse_evaluation' num2str(round(freq_range(1))) '_to_' num2str(round(freq_range(end))) '.png'];
    
end
if log_flag ==1
  
    plot_title = ['log_mse_evaluation' num2str(round(freq_range(1))) '_to_' num2str(round(freq_range(end))) '.png'];
    
end
figure
subplot(4,1,1)
evaluate_glmfit(power_matrix, large_labels, F, channel_id, fourier_sampling_rate, freq_range, 1, 0, 10, 10)
subplot(4,1,2)
evaluate_glmfit(power_matrix, large_labels, F, channel_id, fourier_sampling_rate, freq_range, 1, 0, 10, [9:10])
subplot(4,1,3)
evaluate_glmfit(power_matrix, large_labels, F, channel_id, fourier_sampling_rate, freq_range, 1, 0, 10, [8:10])
subplot(4,1,4)
evaluate_glmfit(power_matrix, large_labels, F, channel_id, fourier_sampling_rate, freq_range, 1, 0, 10, [7:10])
ha = axes('Position',[0 0 1 1],'Xlim',[0 1],'Ylim',[0 1],'Box','off','Visible','off','Units','normalized', 'clipping' , 'off');
text(0.5, 1,main_title, 'HorizontalAlignment','center','VerticalAlignment', 'top')
text(0.1, 1, get_variables('date_str'),'HorizontalAlignment','center','VerticalAlignment', 'top')
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',24,'fontWeight','bold')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
set(gcf,'outerposition',[1 1 1920 1200])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],['channel_' num2str(channel_id+64) '_' plot_title])
end

