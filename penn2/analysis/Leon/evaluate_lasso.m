function evaluate_lasso(power_matrix, large_labels, F, channel_id, fourier_sampling_rate, frequency_range, norm_flag, log_flag)
%EVALUATE_LASSO runs over a lasso regression using MAtlab's lasso function
%it takes a set of trails as trianing data and then runs some tests on
%testing data

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
chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F));
if norm_flag ==1
    chan_power_mat = zscore(chan_power_mat);
elseif log_flag ==1
    chan_power_mat = log(chan_power_mat);
end
[F_val chan_power_mat] = extract_frequency_reg(chan_power_mat, F, frequency_range, 'single'); %extract the frequencies that we want
[aligned_channel, aligned_time] = align_data(large_labels', chan_power_mat, rise_or_fall, fourier_sampling_rate);
[aligned_force aligned_time] = align_data(large_labels', large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
cell_data_test = {};
cell_label_test = {};
cell_trials = {};
fold_number = 2;
for fold_idx = 1:fold_number
    %now we generate the datasets that we want to use for training and
    %testing
    [cell_data_test{fold_idx}, cell_label_test{fold_idx}, cell_trials{fold_idx}] = extract_subset_trials(chan_power_mat, large_labels, rise_or_fall, fourier_sampling_rate, fold_number, fold_idx);
end
training_folds = [1];
testing_folds = [2];
b_vect = [];
for freq_idx=1:length(frequency_range)
    for training_idx = training_folds
        [b,dev,stats] = glmfit(cell_data_test{training_idx}(:, freq_idx), cell_label_test{training_idx}); % Linear regression
    end
    predicted_values = glmval(cell_data_test{2}(:, freq_idx), cell_label_test{2}, 'link','identity');
    ssres = sum((cell_label_test{2}-predicted_values).^2)
end
%sst=sum((cell_label_test{2}-mean(cell_label_test{2})).^2);
%ssr=sum((predicted_values - mean(cell_label_test{2})).^2);

rsq = 1 - ssres/sst;
end

