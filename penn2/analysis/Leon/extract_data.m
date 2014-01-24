function [dataset, labels_dataset] = extract_data(power_matrix, large_labels, rise_or_fall, fourier_sampling_rate, F, frequency_range, channel_id)
%EXTRACT_DATA extract the data for a specific channel at a specific
%frequency from a dataset that contains all the frequency features from
%matlab spectrogram function. The data excludes any inter trial time, only
%limiting itself to a set time before and after the squeeze.
%The setup time is set in the get_variables file under time_right and time
%left
%--------------------------------------------------------------------------
%input:
%--------------------------------------------------------------------------
%channel_id: id of the channel to analyze (1-60)
%F: array with all the Frequencies represented, usually the output of
%spectrogram function in matlab
%power_matrix: The feature vector of TxD, where T is the time sample and D
%is the F features of each channel, 
%large labels: array of 1's and 0s, where a one is declared where a set
%threshold for the force sensor is set.
%frequency_range: array that contains the frequency range to use in the
%classification.
%fourier_smapling_rate: is a value that represents the sampling rate of the
%power matrix and the large labels, usually reduced after calculating the
%spectrogram.
%rise_or_fall: defines if we want to extract the trials based on the rise
%of the que or the fall
chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F)); %extract the info for the channel_id
[F_vect chan_power_mat] = extract_frequency_reg(chan_power_mat, F, frequency_range, 'average'); %extract the frequencies that we want
[dataset, labels_dataset, trials] = extract_trials(chan_power_mat, large_labels, rise_or_fall, fourier_sampling_rate);
end

