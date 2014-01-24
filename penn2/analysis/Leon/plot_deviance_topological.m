function plot_deviance_topological(power_matrix, large_labels, F, frequency_range, single_joint, fourier_sampling_rate, norm_flag, log_flag)
%PLOT_DEVIANCE_TOPOLOGICAL plots the averaged deviance of each channel.
%The averaged deviance may come from using each channel separately and
%averaging them, or using every feature in the channel

%--------------------------------------------------------------------------
%input:
%-------------------------------------------------------------------------
%F: array with all the Frequencies represented, usually the output of
%spectrogram function in matlab
%power_matrix: The feature vector of TxD, where T is the time sample and D
%is the F features of each channel, 
%large labels: array of 1's and 0s, where a one is declared where a set
%threshold for the force sensor is set.
%limit_frequency: frequency limit to analyze
%fourier_smapling_rate: is a value that represents the sampling rate of the
%power matrix and the large labels, usually reduced after calculating the
%spectrogram.
%single_joint: string value (single, joint) that specifies whether we use a
%single feature regression or a joint feature regression.
%norm_flag: 1:normalized, 0 unnormalized
%log_flag: 1: get the log(power), 0: no processing.

rise_or_fall = 'rise';%variable that controls whether t align data tgo rising edges or falling edges
deviance_channel = [];
channels_to_use = [1:60];
x_size = ceil(length(channels_to_use)/5);
y_size = length(channels_to_use)/x_size;
for channel_id = channels_to_use;
    disp(['Processing Channel: ' num2str(channel_id)])
    chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F)); %extract the info for the channel_id
    [F_val chan_power_mat] = extract_frequency_reg(chan_power_mat, F, frequency_range, 'single'); %extract the frequencies that we want
    if norm_flag ==1
        chan_power_mat = zscore(chan_power_mat);%normalize using zscore(data-mean)/std
        plot_title = ['normalized_analysis_electrode_heatmap' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
    end
    if log_flag ==1
        chan_power_mat = log(chan_power_mat); %use the log transform
        plot_title = ['logarithmic_analysis_electrode_heatmap' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
    end
    disp('Passed the normalization')
    %extract the data around the trials
    [dataset, labels_dataset, trials] = extract_trials(chan_power_mat, large_labels, rise_or_fall, fourier_sampling_rate);
    %set the place holders for the b and p vectors
    
    dev_vect = [];
    disp('Running the regression')
    switch single_joint
        case 'single'
            for freq_idx=1:length(F_val)
                [b,dev,stats] = glmfit(dataset(:, freq_idx), labels_dataset); % Linear regression
                dev_vect = [dev_vect dev];
            end
            deviance_channel(channel_id) = mean(dev_vect);
        case 'joint'
           [b,dev,stats] = glmfit(dataset, labels_dataset); % Linear regression
           deviance_channel(channel_id) = dev;
    end
end
figure('visible','off');
imagesc(reshape(deviance_channel,y_size,x_size)')
colorbar
colormap(copper)
set(gca,'xtick',[])
set(gca,'ytick',[])
chann_count = 1;
main_title = ['Heat Map of the electrodes, ' num2str(frequency_range(1)) ' to ' num2str(frequency_range(end)) '[Hz], Trials: ' num2str(trials) ', Window Size = 0.5 [s], Overlap:' num2str(get_variables('overlap_percentage'))];
if norm_flag == 1
    main_title = ['Heat Map of the electrodes, '  num2str(frequency_range(1)) ' to ' num2str(frequency_range(end))  '[Hz], Trials: ' num2str(trials) ', Window Size = 0.5 [s], Normalized, Overlap:' num2str(get_variables('overlap_percentage'))];
end
if log_flag == 1
    main_title = ['Heat Map of the electrodes, '  num2str(frequency_range(1)) ' to ' num2str(frequency_range(end)) '[Hz], Trials: ' num2str(trials) ', Window Size = 0.5 [s], Log(power) features, Overlap:' num2str(get_variables('overlap_percentage'))];
end
for y_text = 1:x_size
    for x_text = 1:y_size
        text(x_text, y_text, num2str(chann_count+64))
        chann_count = chann_count + 1;
    end
end
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',24,'fontWeight','bold', 'color', [1,0,0])
title(main_title, 'color','k','FontSize',24)
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
set(gcf,'outerposition',[1 1 1920 1200])
myaa([4 2],plot_title)
