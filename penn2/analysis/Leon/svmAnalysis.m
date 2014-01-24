function [searchmin fval] = svmAnalysis(channel_id, F, power_matrix, large_labels, frequency_range, fourier_sampling_rate, norm_flag, log_flag)
%SVMANALYSIS does an analysis using matlab svmtrain fucntion on an input
%dataset and a set of labels.
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
%norm_flag: 1:normalized, 0 unnormalized
%log_flag: 1: get the log(power), 0: no processing.


%svmAnalysis(3, F, large_power_matrix, large_labels, [0:100],30, 0.10, 1, 0)

rise_or_fall = 'rise';%variable that controls whether t align data tgo rising edges or falling edges
chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F)); %extract the info for the channel_id
[F_vect chan_power_mat] = extract_frequency_reg(chan_power_mat, F, frequency_range, 'single'); %extract the frequencies that we want
if norm_flag ==1
    chan_power_mat = zscore(chan_power_mat);%normalize using zscore (data-mean)/std
    plot_title = ['svm_normalized_analysis_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
    
elseif log_flag ==1
    chan_power_mat = log(chan_power_mat);
    plot_title = ['svm_logarithmic_scale_analysis_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
else 
    plot_title = ['svm_analysis_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
end
[aligned_channel, aligned_time] = align_data(large_labels', chan_power_mat, rise_or_fall, fourier_sampling_rate);
size(aligned_channel)
%Extract only the are around succesfull trials to prune the data
[aligned_force aligned_time] = align_data(large_labels', large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
[time, feat, trials] = size(aligned_channel);
dataset_perm=permute(aligned_channel,[1,3,2]);%this rearranges the indexes
dataset = reshape(dataset_perm,[],size(aligned_channel,2),1);%this does the reshaping so it has a sensible format in accordance with TxN
labels_perm = permute(aligned_force,[1,3,2]);%rearrages the index for the later reshape
labels_dataset = 2*reshape(labels_perm,[],size(aligned_force,2),1)-1;%we need to rescale to 1,-1 for the SVM library
c = cvpartition(time*trials,'kfold',3);
minfn = @(z)crossval('mcr',dataset,labels_dataset,'Predfun', ...
    @(xtrain,ytrain,xtest)crossfun(xtrain,ytrain,...
    xtest,exp(z(1)),exp(z(2))),'partition',c);
opts = optimset('TolX',5e-4,'TolFun',5e-4);
[searchmin fval] = patternsearch(minfn,randn(2,1));

