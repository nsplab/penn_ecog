function [F_vect group] = groupLassoAnalysis(channel_id, F, power_matrix, large_labels, frequency_range, fourier_sampling_rate, group_perc, norm_flag, log_flag)
%LASSO_REGRESSION uses the library detailed before to run the group lasso
%on a desired set of frequencies. It uses a set percentage to create the
%division among groups.

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
%group_perc: percentage of each group (0.1,0.2)
%norm_flag: 1:normalized, 0 unnormalized
%log_flag: 1: get the log(power), 0: no processing.

%Example Use
%groupLassoAnalysis(3, F, large_power_matrix, large_labels, [0:100],30, 0.10, 1, 0)


%Test simpe group Lasso in the dataset
%Code used from:
%http://www.cs.cmu.edu/~gunhee/software.html
%Simple implementation based on the paper:
%http://webdocs.cs.ualberta.ca/~mahdavif/ReadingGroup/Papers/tr1095.pdf

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% run the group lasso
%
% 1. Input
% Y:        responses ([n x 1], n: # of data)
% X:        predictor variables ([n x p], p: dimension) 
% group:    group indicatior ([1 x J] cell, J: # of groups)
%           Each cell contains the column indices of X which are in the same group.
% lambda:   parameter
%
% 2. Output
% beta:         coefficients ([p x 1])
% beta_hist:    beta history
% lasso parameter
lambda = 0.3;
%set the precentage of the groups
rise_or_fall = 'rise';%variable that controls whether t align data tgo rising edges or falling edges
chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F)); %extract the info for the channel_id
[F_vect chan_power_mat] = extract_frequency_reg(chan_power_mat, F, frequency_range, 'single'); %extract the frequencies that we want
%we need to build the cells containing the groups.
[group group_name] = create_groups_lasso(F_vect, group_perc);
if norm_flag ==1
    chan_power_mat = zscore(chan_power_mat);
    plot_title = ['lasso_normalized_analysis_single_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
    
elseif log_flag ==1
    chan_power_mat = log(chan_power_mat);
    plot_title = ['lasso_logarithmic_scale_analysis_single_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
else 
    plot_title = ['lasso_analysis_single_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
end
[aligned_channel, aligned_time] = align_data(large_labels', chan_power_mat, rise_or_fall, fourier_sampling_rate);
size(aligned_channel)
%Extract only the are around succesfull trials to prune the data
[aligned_force aligned_time] = align_data(large_labels', large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
[time, feat, trials] = size(aligned_channel);
dataset_perm=permute(aligned_channel,[1,3,2]);%this rearranges the indexes
dataset = reshape(dataset_perm,[],size(aligned_channel,2),1);%this does the reshaping so it has a sensible format in accordance with TxN
labels_perm = permute(aligned_force,[1,3,2]);
labels_dataset = reshape(labels_perm,[],size(aligned_force,2),1);
disp('Dataset size is')
size(dataset)
%here the X is the variable called dataset and the Y is the variable called
%labels_dataset
[beta, beta_hist] = LassoShootingGroup(dataset, labels_dataset, group, lambda) ;
stem(F_vect, beta, 'Marker', 'None')
hold on
p1 = plot(F_vect, beta, 'ko','MarkerSize',7, 'MarkerEdgeColor','k',...
    'MarkerFaceColor',[0,0,0]);
freq_groups = group_perc*length(frequency_range);
title_text = ['Grouped Lasso with frequency groups of ' num2str(freq_groups) 'Hz'];
if log_flag == 1
    main_title = ['Trials: ' num2str(trials) ', Window Size = 0.5 [s], Log(powers) features'];
elseif norm_flag==1
    main_title = ['Trials: ' num2str(trials) ', Window Size = 0.5 [s], Normalized features'];
else
    main_title = ['Trials: ' num2str(trials) ', Window Size = 0.5 [s]']; 
end
main_title = [main_title ', Channel: ' num2str(channel_id+64)];
title([title_text ,', ',  main_title])
if(min(beta)<max(beta))
    ylim([min(beta) max(beta)])
else
    ylim([-1,1])
end
group_ticks = floor(cellfun(@(v)v(1),group_name)); %this operates over every member of the cell and gets the first element of each array
group_ticks = floor([group_ticks  group_name{end}(end)]) %get the last tick
xlimit=get(gca,'XLim');
increment = xlimit(2)*0.05;
set(gca, 'XMinorTick','on')
set(gca,'XTick',[group_ticks])
set(gca,'XTickLabel',[group_ticks])
%set(gca,'XTick',[xlimit(1):increment:xlimit(2)])
%set(gca,'XTickLabel',[xlimit(1):increment:xlimit(2)])
figureHandle = gcf;
set(gca, 'fontsize', 18)
set(findall(figureHandle,'type','text'),'fontSize',22,'fontWeight','bold')



