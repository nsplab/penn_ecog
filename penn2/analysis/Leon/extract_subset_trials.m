function [dataset, labels_dataset, trials] = extract_subset_trials(chan_power_mat, large_labels, rise_or_fall, fourier_sampling_rate, folds, fold_number)
%EXTRACT_SUBSET_TRIALS is a function that given a channel data, will extratct the
%data round the trials, disregarding inter trial eeg power. It will extract
%the desired trials given a number of folds and the fold to extract
%--------------------------------------------------------------------------
%input:
%-------------------------------------------------------------------------
%chan_power_mat: Feature vector of a single channel of size TxF
%large labels: array of 1's and 0s, where a one is declared where a set
%threshold for the force sensor is set.
%rise_or_fall: strin array indicating if we are interested in rising edges
%or falling edges of the data
%fourier_smapling_rate: is a value that represents the sampling rate of the
%power matrix and the large labels, usually reduced after calculating the
%spectrogram.
%fold: total number of folds to divide the data in
%fold_number: number of fold to be extracted
[aligned_channel, aligned_time] = align_data(large_labels', chan_power_mat, rise_or_fall, fourier_sampling_rate);
[aligned_force aligned_time] = align_data(large_labels', large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
[time, feat, trials] = size(aligned_channel);
%check for the number of trials per fold
trials_per_fold = trials/folds;
if round(trials_per_fold)~=trials_per_fold
    disp('There is not a precise division given the folds, will be approximated to lowest integer')
    trials_per_fold = floor(trials_per_fold);
end
if fold_number > folds
    error('The indicated fold number is larger than the number of folds')
end

aligned_channel = aligned_channel(:,:,(fold_number-1)*trials_per_fold+1:trials_per_fold*fold_number);
aligned_force = aligned_force(:,:,(fold_number-1)*trials_per_fold+1:trials_per_fold*fold_number);
[time, feat, trials] = size(aligned_channel);
dataset_perm=permute(aligned_channel,[1,3,2]);%this rearranges the indexes
dataset = reshape(dataset_perm,[],size(aligned_channel,2),1);%this does the reshaping so it has a sensible format
labels_perm = permute(aligned_force,[1,3,2]);
labels_dataset = reshape(labels_perm,[],size(aligned_force,2),1);
end