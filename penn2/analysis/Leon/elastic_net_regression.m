function beta = elastic_net_regression(power_matrix, large_labels, F, channel_id, fourier_sampling_rate, frequency_range, norm_flag, log_flag)

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
%power_matrix = zscore(power_matrix);

%First, we extract the frequencies for the channel that we desire to study
%and concatenate all of the channels together
num_chan = get_variables('number_of_channels');
%chan_power_mat = smooth_features(power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F)),100);
%chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F));
chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F));
if norm_flag ==1
    chan_power_mat = zscore(chan_power_mat);
    plot_title = ['lasso_normalized_analysis_single_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
    
elseif log_flag ==1
    chan_power_mat = log(chan_power_mat);
    plot_title = ['elastic_net_logarithmic_scale_analysis_single_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
else 
    plot_title = ['elastic_net_analysis_single_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
end
size(chan_power_mat)
[F_val chan_power_mat] = extract_frequency_reg(chan_power_mat, F, frequency_range, 'single'); %extract the frequencies that we want
[aligned_channel, aligned_time] = align_data(large_labels', chan_power_mat, rise_or_fall, fourier_sampling_rate);
size(aligned_channel)
[aligned_force aligned_time] = align_data(large_labels', large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
[time, feat, trials] = size(aligned_channel);
dataset_perm=permute(aligned_channel,[1,3,2]);%this rearranges the indexes
dataset = reshape(dataset_perm,[],size(aligned_channel,2),1);%this does the reshaping so it has a sensible format
labels_perm = permute(aligned_force,[1,3,2]);
labels_dataset = reshape(labels_perm,[],size(aligned_force,2),1);
[beta fitinfo] =lasso(dataset, labels_dataset,'CV', 3);

stem(F_val, beta(:,fitinfo.IndexMinMSE), 'ko','MarkerSize',10, 'MarkerEdgeColor','k',...
    'MarkerFaceColor',[0.8,0.8,0.8])
grid on
grid minor
frequency_range
xlim([frequency_range(1) frequency_range(end)])
xlabel('Frequency')
ylabel('B-values')
xlimit=get(gca,'XLim');
increment = xlimit(2)*0.05;
set(gca, 'XMinorTick','on')
set(gca,'XTick',[xlimit(1):increment:xlimit(2)])
set(gca,'XTickLabel',[xlimit(1):increment:xlimit(2)])
set(gca, 'XMinorTick','on')
if norm_flag == 1
    title(['B-Values, Normalized features, Trials: ' num2str(trials) ', Window Size = 0.5 [s], Lambda = ' num2str(fitinfo.LambdaMinMSE)]);
elseif log_flag ==1
    title(['B-Values, log(power) features, Trials: ' num2str(trials) ', Window Size = 0.5 [s], Lambda = ' num2str(fitinfo.LambdaMinMSE)]);
else
    title(['B-Values, Trials: ' num2str(trials) ', Window Size = 0.5 [s], Lambda = ' num2str(fitinfo.LambdaMinMSE)]);
end
set(gca, 'fontsize', 20)
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',24,'fontWeight','bold')

