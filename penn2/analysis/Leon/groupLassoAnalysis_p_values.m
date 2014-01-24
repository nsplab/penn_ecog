function groupLassoAnalysis_p_values(grouping_idx, channel_id, F, power_matrix, large_labels, frequency_range, fourier_sampling_rate, norm_flag, log_flag)
%GROUPLASSOANALYSIS_P_VALUES plots the grouped lasso groups along with the
%weight obtained from a group lasso analysis.

%--------------------------------------------------------------------------
%input:
%-------------------------------------------------------------------------
%grouping_idx = percentage of grouping of the frequencies
%channel_id: channel to work with
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

[F_feat group] = groupLassoAnalysis(channel_id, F, power_matrix, large_labels, frequency_range,fourier_sampling_rate, grouping_idx, norm_flag, log_flag);
cc=jet(length(group));
ylimit=get(gca,'YLim');
set(gca,'layer','top')
for group_idx = 1:length(group)
    ntp = length(F_feat(group{group_idx}));
    B=area(F_feat(group{group_idx}), ylimit(2)*ones(ntp,1));
    ch = get(B,'child');
    set(ch, 'facea', 0.3,'edgea',0.3,'FaceColor', cc(group_idx,:), 'EdgeColor', cc(group_idx,:));%set the bar to be equal to the stem
    B = area(F_feat(group{group_idx}), ylimit(1)*ones(ntp,1));
    ch = get(B,'child');
    set(ch, 'facea', 0.3,'edgea',0.3,'FaceColor', cc(group_idx,:), 'EdgeColor', cc(group_idx,:));%set the bar to be equal to the stem
end
xlim([frequency_range(1) frequency_range(end)])
title(['Grouped Lasso with groups of ' num2str(length(group{1})) ' frequency bands']);
ylabel('Coefficients')
xlabel('Frequency [Hz]')
grid on
end