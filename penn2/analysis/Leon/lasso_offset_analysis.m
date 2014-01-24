function lasso_offset_analysis(chan_id, F, large_power_matrix, large_labels, frequency_range ,fourier_sampling_rate, lasso_or_elastic)
%LASSO_OFFSET_ANALYSIS function will do an offset analysis using either eleastic nets or
%lasso regression.
count_idx = 1;
rise_or_fall = 'rise';
differential = 0.2
for offset_time = 1.0:-differential:0 %iterate over seconds
    chan_power_mat = large_power_matrix(:,(chan_id-1)*length(F)+1:chan_id*length(F)); %extract the info for the current channel
    [F_val chan_power_mat] = extract_frequency_reg(chan_power_mat, F, frequency_range, 'single'); %extract the frequencies that we want
    [off_large_labels, chan_power_mat] = offset_label(chan_power_mat, large_labels, offset_time, 'backward', fourier_sampling_rate);%add an offset to the labels
    [aligned_channel, aligned_time] = align_data(off_large_labels', chan_power_mat, rise_or_fall, fourier_sampling_rate);
    [aligned_force aligned_time] = align_data(off_large_labels', off_large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
    [time, feat, trials] = size(aligned_channel);
    dataset_perm=permute(aligned_channel,[1,3,2]);%this rearranges the indexes
    dataset = reshape(dataset_perm,[],size(aligned_channel,2),1);%this does the reshaping so it has a sensible format
    labels_perm = permute(aligned_force,[1,3,2]);
    labels_dataset = reshape(labels_perm,[],size(aligned_force,2),1);
    switch lasso_or_elastic
        case 'lasso'
            [b, fitinfo] = lasso(dataset ,labels_dataset, 'CV', 3); % Lasso regression
        case 'elastic_nets'
            [b, fitinfo] = lasso(dataset ,labels_dataset, 'CV', 3, 'Alpha', 0.5); % Elastic Net regression regression
    end
    MSE_score(count_idx) = fitinfo.MSE(fitinfo.IndexMinMSE);
    y_value(count_idx) = -offset_time;
    count_idx = count_idx + 1;
end
for offset_time = differential:differential:1 %iterate over seconds
    chan_power_mat = large_power_matrix(:,(chan_id-1)*length(F)+1:chan_id*length(F)); %extract the info for the current channel
    [F_val chan_power_mat] = extract_frequency_reg(chan_power_mat, F, frequency_range, 'single'); %extract the frequencies that we want
    [off_large_labels, chan_power_mat] = offset_label(chan_power_mat, large_labels, offset_time, 'forward', fourier_sampling_rate);%add an offset to the labels
    [aligned_channel, aligned_time] = align_data(off_large_labels', chan_power_mat, rise_or_fall, fourier_sampling_rate);
    [aligned_force aligned_time] = align_data(off_large_labels', off_large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
    [time, feat, trials] = size(aligned_channel);
    dataset_perm=permute(aligned_channel,[1,3,2]);%this rearranges the indexes
    dataset = reshape(dataset_perm,[],size(aligned_channel,2),1);%this does the reshaping so it has a sensible format
    labels_perm = permute(aligned_force,[1,3,2]);
    labels_dataset = reshape(labels_perm,[],size(aligned_force,2),1);
    switch lasso_or_elastic
        case 'lasso'
            [b, fitinfo] = lasso(dataset ,labels_dataset, 'CV', 3); % Lasso regression
        case 'elastic_nets'
            [b, fitinfo] = lasso(dataset ,labels_dataset, 'CV', 3, 'Alpha', 0.5); % Elastic Net regression regression
    end
    MSE_score(count_idx) = fitinfo.MSE(fitinfo.IndexMinMSE);
    y_value(count_idx) = offset_time;
    count_idx = count_idx + 1;
end
fig1 = figure('visible','off');
hold on
plot(y_value, MSE_score, '-bo', 'color', [0.5,0.5,0.5],'LineWidth', 2, 'MarkerSize', 4, 'MarkerEdgeColor', 'k','MarkerFaceColor', [0.8,0.8,0.8])
xlabel('Offset Time [s]')
ylabel('MSE value')
title('Goodness of fit for different values of offset between the labels and the features')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],[num2str(frequency_range(1)) '_to_' num2str(frequency_range(end))  lasso_or_elastic '_regression_r_value_backwad.png'])
