function evaluate_glmfit(power_matrix, large_labels, F, channel_id, fourier_sampling_rate, frequency_range, norm_flag, log_flag, fold_number, testing_folds)
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
for fold_idx = 1:fold_number
    %now we generate the datasets that we want to use for training and
    %testing
    [cell_data_test{fold_idx}, cell_label_test{fold_idx}, cell_trials{fold_idx}] = extract_subset_trials(chan_power_mat, large_labels, rise_or_fall, fourier_sampling_rate, fold_number, fold_idx);
end
trials = sum([cell_trials{:}]);
training_folds = [1:fold_number];
training_folds(ismember(training_folds,testing_folds)) = []; %remove the testing fold
mse_vect = [];
training_data = [];
training_labels = [];
for training_idx = training_folds
        %aggregate the training data
        training_data = [training_data; cell_data_test{training_idx}];
        training_labels = [training_labels; cell_label_test{training_idx}];
   
end
testing_data = [];
testing_labels = [];
for testing_idx = testing_folds
        %aggregate the training data
        testing_data = [testing_data; cell_data_test{testing_idx}];
        testing_labels = [testing_labels; cell_label_test{testing_idx}];
   
end
for freq_idx=1:length(F_val)
    [b,dev,stats] = glmfit(training_data(:, freq_idx), training_labels); % Linear regression
    predicted_values = glmval(b, testing_data(:, freq_idx), 'identity');
    ssres = sum((testing_labels-predicted_values).^2);
    mse_vect = [mse_vect ssres];
end
stem(F_val,mse_vect, 'ko','MarkerSize',7, 'MarkerEdgeColor','k',...
'MarkerFaceColor',[0.8,0.8,0.8])
grid on
grid minor;
xlim([0, frequency_range(end)])
xlabel('Frequency')
ylabel('MSE')
title(['MSE']);
set(gca, 'fontsize', 20)
xlimit=get(gca,'XLim');
%Code to format the minor ticks
increment = xlimit(2)*0.05;
%Format the minor ticks
set(gca,'XTick',[xlimit(1):increment:xlimit(2)])
set(gca,'XTickLabel',[xlimit(1):increment:xlimit(2)])
title(['MSE scores for individually regressed frequencies (glmfit) ' num2str(length(testing_folds)/length([1:fold_number])*100) ' % Testing' ])
%sst=sum((cell_label_test{2}-mean(cell_label_test{2})).^2);
%ssr=sum((predicted_values - mean(cell_label_test{2})).^2);

%rsq = 1 - ssres/sst;
end

