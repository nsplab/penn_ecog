function [dataset, labels_dataset, trials] = extract_trials(chan_power_mat, large_labels, rise_or_fall, fourier_sampling_rate)
%EXTRACT_TRIALS is a function that given a channel data, will extratct the
%data round the trials, disregarding inter trial eeg power.
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
[aligned_channel, aligned_time] = align_data(large_labels', chan_power_mat, rise_or_fall, fourier_sampling_rate);
[aligned_force aligned_time] = align_data(large_labels', large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force

[time, feat, trials] = size(aligned_channel);
dataset_perm=permute(aligned_channel,[1,3,2]);%this rearranges the indexes
dataset = reshape(dataset_perm,[],size(aligned_channel,2),1);%this does the reshaping so it has a sensible format
labels_perm = permute(aligned_force,[1,3,2]);
labels_dataset = reshape(labels_perm,[],size(aligned_force,2),1);
end

