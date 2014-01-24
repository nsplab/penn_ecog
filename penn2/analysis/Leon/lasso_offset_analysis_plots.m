function lasso_offset_analysis_plots(chan_id, F, large_power_matrix, large_labels, frequency_range ,fourier_sampling_rate, lasso_or_elastic, norm_flag, log_flag)
%LASSO_OFFSET_ANALYSIS function will do an offset analysis using either eleastic nets or
%lasso regression.
%This plots the value of the coefficients for different delays
cv_ = 3;
count_idx = 1;
rise_or_fall = 'rise';
differential = 0.1;
total_offset = [-1.0:differential:0.0 differential:differential:1];
numb_rows = length(total_offset);
figure
%fig1 = figure('visible','off');
for offset_time = 1.0:-differential:0 %iterate over seconds
    chan_power_mat = large_power_matrix(:,(chan_id-1)*length(F)+1:chan_id*length(F)); %extract the info for the current channel
    if norm_flag ==1
        chan_power_mat = zscore(chan_power_mat);
        plot_title = ['lasso_normalized_analysis_offset_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];

    elseif log_flag ==1
        chan_power_mat = log(chan_power_mat);
        plot_title = ['lasso_logarithmic_scale_analysis_offset_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
    else 
        plot_title = ['lasso_analysis_offset_regression' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '.png'];
    end
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
            [b, fitinfo] = lasso(dataset ,labels_dataset, 'CV', cv_); % Lasso regression
        case 'elastic_nets'
            [b, fitinfo] = lasso(dataset ,labels_dataset, 'CV', cv_, 'Alpha', 0.5); % Elastic Net regression regression
    end
    if count_idx==1
        numb_columns = length(F_val);
        beta_matrix = zeros(numb_rows, numb_columns);
    end
    y_value(count_idx) = -offset_time;
    beta_matrix(count_idx,:) = b(:,fitinfo.IndexMinMSE);
    count_idx = count_idx + 1;
end
for offset_time = differential:differential:1 %iterate over seconds
    chan_power_mat = large_power_matrix(:,(chan_id-1)*length(F)+1:chan_id*length(F)); %extract the info for the current channel
    if norm_flag ==1
        chan_power_mat = zscore(chan_power_mat);
    elseif log_flag ==1
        chan_power_mat = log(chan_power_mat);
    end
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
            [b, fitinfo] = lasso(dataset ,labels_dataset, 'CV', cv_); % Lasso regression
            algo_type = 'LASSO, ';
        case 'elastic_nets'
            [b, fitinfo] = lasso(dataset ,labels_dataset, 'CV', cv_, 'Alpha', 0.5); % Elastic Net regression regression
            algo_type = 'ElasticNet, '
    end
    y_value(count_idx) = offset_time;
    beta_matrix(count_idx,:) = b(:,fitinfo.IndexMinMSE);
    count_idx = count_idx + 1;
end
surf(F_val, y_value, beta_matrix,'edgecolor','none')
axis tight
colorbar
view(0, 90)
xlim([frequency_range(1) frequency_range(end)])
xlabel('Frequency')
ylabel('Offset [s] of the force happens with respect to the ECoG')
xlimit=get(gca,'XLim');
increment = xlimit(2)*0.05;
set(gca, 'XMinorTick','on')
set(gca,'XTick',[xlimit(1):increment:xlimit(2)])
set(gca,'XTickLabel',[xlimit(1):increment:xlimit(2)])
set(gca, 'XMinorTick','on')
if norm_flag == 1
    title([algo_type 'B-Values, Normalized features, Trials: ' num2str(trials) ', Window Size = 0.5 [s]']);
elseif log_flag ==1
    title([algo_type 'B-Values, log(power) features, Trials: ' num2str(trials) ', Window Size = 0.5 [s]']);
else
    title([algo_type 'B-Values, Trials: ' num2str(trials) ', Window Size = 0.5 [s], Lambda = ' num2str(fitinfo.LambdaMinMSE)]);
end
set(gca, 'fontsize', 18)
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',22,'fontWeight','bold')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
set(gcf,'outerposition',[1 1 1920 1200])
myaa([4 2],plot_title)
