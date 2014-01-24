function grouped_lasso_offset_analysis(chan_id, F, large_power_matrix, large_labels, frequency_range ,fourier_sampling_rate, norm_flag, log_flag)
%LASSO_OFFSET_ANALYSIS function will do an offset analysis using either eleastic nets or
%lasso regression.

rise_or_fall = 'rise';
differential = 0.1;
fig1 = figure('visible','off');
frequency_sweep = [10:10:100];
cc=copper(length(frequency_sweep));
count_group = 1;
legend_names={};
for group_freq_idx = frequency_sweep
    count_idx = 1;
    group_freq = group_freq_idx; %grouping parameter in hz
    group_perc = ((frequency_range(end) - frequency_range(1))/group_freq)^-1;
    lambda = 0.5;
    for offset_time = 1.0:-differential:0 %iterate over seconds
        chan_power_mat = large_power_matrix(:,(chan_id-1)*length(F)+1:chan_id*length(F)); %extract the info for the current channel
        if norm_flag ==1
            chan_power_mat = zscore(chan_power_mat);
            plot_title = ['delay_glasso_normalized_analysis_single_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];

        elseif log_flag ==1
            chan_power_mat = log(chan_power_mat);
            plot_title = ['delay_glasso_logarithmic_scale_analysis_single_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
        else 
            plot_title = ['delay_glasso_analysis_single_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
        end
        [F_vect chan_power_mat] = extract_frequency_reg(chan_power_mat, F, frequency_range, 'single'); %extract the frequencies that we want
        [off_large_labels, chan_power_mat] = offset_label(chan_power_mat, large_labels, offset_time, 'backward', fourier_sampling_rate);%add an offset to the labels
        [aligned_channel, aligned_time] = align_data(off_large_labels', chan_power_mat, rise_or_fall, fourier_sampling_rate);
        [aligned_force aligned_time] = align_data(off_large_labels', off_large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
        [time, feat, trials] = size(aligned_channel);
        dataset_perm=permute(aligned_channel,[1,3,2]);%this rearranges the indexes
        dataset = reshape(dataset_perm,[],size(aligned_channel,2),1);%this does the reshaping so it has a sensible format
        labels_perm = permute(aligned_force,[1,3,2]);
        labels_dataset = reshape(labels_perm,[],size(aligned_force,2),1);
        [group group_name] = create_groups_lasso(F_vect, group_perc);
        [beta, beta_hist, mse_score] = LassoShootingGroup(dataset, labels_dataset, group, lambda) ;

        MSE_score(count_idx) = mse_score;
        y_value(count_idx) = -offset_time;
        count_idx = count_idx + 1;
    end
    for offset_time = differential:differential:1 %iterate over seconds
        chan_power_mat = large_power_matrix(:,(chan_id-1)*length(F)+1:chan_id*length(F)); %extract the info for the current channel
        if norm_flag ==1
            chan_power_mat = zscore(chan_power_mat);
        elseif log_flag ==1
            chan_power_mat = log(chan_power_mat);
        end
        [F_vect chan_power_mat] = extract_frequency_reg(chan_power_mat, F, frequency_range, 'single'); %extract the frequencies that we want
        [off_large_labels, chan_power_mat] = offset_label(chan_power_mat, large_labels, offset_time, 'forward', fourier_sampling_rate);%add an offset to the labels
        [aligned_channel, aligned_time] = align_data(off_large_labels', chan_power_mat, rise_or_fall, fourier_sampling_rate);
        [aligned_force aligned_time] = align_data(off_large_labels', off_large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
        [time, feat, trials] = size(aligned_channel);
        dataset_perm=permute(aligned_channel,[1,3,2]);%this rearranges the indexes
        dataset = reshape(dataset_perm,[],size(aligned_channel,2),1);%this does the reshaping so it has a sensible format
        labels_perm = permute(aligned_force,[1,3,2]);
        labels_dataset = reshape(labels_perm,[],size(aligned_force,2),1);
        [group group_name] = create_groups_lasso(F_vect, group_perc);
        [beta, beta_hist, mse_score] = LassoShootingGroup(dataset, labels_dataset, group, lambda) ;
        MSE_score(count_idx) = mse_score;
        y_value(count_idx) = offset_time;
        count_idx = count_idx + 1;
    end  
    hold on
    plot(y_value, MSE_score, '-bo', 'color', cc(count_group, :),'LineWidth', 2, 'MarkerSize', 4, 'MarkerEdgeColor', 'k','MarkerFaceColor', [0.8,0.8,0.8])
    legend_names{count_group} = [num2str(group_freq_idx) ' Hz Groups'];
    count_group = count_group + 1;
end
xlabel('Offset Time [s]')
ylabel('MSE value')
legend_names
grid on
legend(legend_names, 'location', 'Best')
title('Goodness of fit for different values of offset between the labels and the features')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],plot_title)
