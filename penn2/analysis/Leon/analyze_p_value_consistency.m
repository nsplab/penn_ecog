function analyze_p_value_consistency(power_matrix, large_labels, F, channel_id, p_value_limit, fourier_sampling_rate, limit_freq, normalized_flag, log_flag)
%ANALYZE_P_VALUE_CONSISTENCY generates panels that show the different p
%values for different divisions of the data.
%The last panel is a logical AND of the different subdivisions of the data

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
%p_value_limit: Is the limit set for the p_value

rise_or_fall = 'rise';
[val limit_freq_index]=findNearest(limit_freq, F);
F_an = F(1:limit_freq_index);
%First, we extract the frequencies for the channel that we desire to study
chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F)); %extract the info for the channel_id
plot_title = ['concistency_analysis_single_regression' num2str(F_an(1)) '_to_' num2str(F_an(end)) '.png'];
if normalized_flag ==1
    chan_power_mat = zscore(chan_power_mat);
    plot_title = ['consistency_normalized_analysis_single_regression' num2str(round(F_an(1))) '_to_' num2str(round(F_an(end))) '.png'];
    
end
if log_flag ==1
    chan_power_mat = log(chan_power_mat);
    plot_title = ['consistency_logarithmic_scale_analysis_single_regression' num2str(round(F_an(1))) '_to_' num2str(round(F_an(end))) '.png'];
    
end
cell_data_test = {};
cell_label_test = {};
cell_trials = {};
fold_number = 2;
for fold_idx = 1:fold_number
    [cell_data_test{fold_idx}, cell_label_test{fold_idx}, cell_trials{fold_idx}] = extract_subset_trials(chan_power_mat, large_labels, rise_or_fall, fourier_sampling_rate, fold_number, fold_idx);
end
trials = sum([cell_trials{:}]);
%set the place holders for the b and p vectors
color_ind = []; % indicates the color of the marker inc ase there is a Nan
main_title = ['Trials: ' num2str(trials) ', Window Size = ' num2str(get_variables('Window_Size')) ' [s], Overlap:' num2str(100*get_variables('overlap_percentage')) '%, Channel:' num2str(channel_id+64)];
if normalized_flag == 1
    main_title = ['Trials: ' num2str(trials) ', Window Size = ' num2str(get_variables('Window_Size')) ' [s], Normalized, Overlap:' num2str(100*get_variables('overlap_percentage')) '%, Channel:' num2str(channel_id+64)];
end
if log_flag == 1
    main_title = ['Trials: ' num2str(trials) ', Window Size = ' num2str(get_variables('Window_Size')) ' [s], Logarithmic-scaled features, Overlap:' num2str(100*get_variables('overlap_percentage')) '%, Channel: ' num2str(channel_id+64)];
end
p_master = [];
for plot_idx = 1:length(cell_data_test)
    b_vect=[];
    p_vect=[];
    color_ind = [];
    for freq_idx=1:length(F_an)
        [b,dev,stats] = glmfit(cell_data_test{plot_idx}(:, freq_idx), cell_label_test{plot_idx}); % Linear regression
        if isnan(stats.p(2))
            color_ind = [color_ind 1]; %If there is a NaN, use flag 1
            stats.p(2) = 0; %If there is a NaN set the p value to 0
            nan_count = nan_count +1;
        elseif  stats.p(2) >=p_value_limit %If the p value is larger than 0.05
            color_ind = [color_ind 2]; %Setr the flag of the color to 2
            stats.p(2) = 0;
        else
            color_ind = [color_ind 0];
        end

        b_vect = [b_vect b(2)];
        p_vect = [p_vect stats.p(2)]; 
    end
    logical_values = zeros(1,length(p_vect));
    logical_values(color_ind==0) = 1;
    p_master = [p_master; logical_values];
    %%%%%%%%%First Plot the B-Values%%%%%%%%%%%%%%%%%%%%%%%
    figure(1);%This figure plots the b-values
    set(gcf,'Visible','off');
    subplot(fold_number,1,plot_idx)
    stem(F_an, b_vect, 'ko','MarkerSize',10, 'MarkerEdgeColor','k',...
    'MarkerFaceColor',[0.8,0.8,0.8])
    grid on
    grid minor;
    xlim([0, limit_freq])
    xlabel('Frequency')
    ylabel('B-values')
    title(['B-Values']);
    set(gca, 'fontsize', 20)
    xlimit=get(gca,'XLim');
    %Code to format the minor ticks
    increment = xlimit(2)*0.05;
    %Format the minor ticks
    set(gca,'XTick',[xlimit(1):increment:xlimit(2)])
    set(gca,'XTickLabel',[xlimit(1):increment:xlimit(2)])
    %%%%%%%%%%%%%%%%%%%%%%%%Now to plot the P-Values%%%%%%%%%%%%%%%%%%%%%%%
    figure(2); %This figure has the P-values
    set(gcf,'Visible','off');
    subplot(fold_number+4,1,plot_idx)
    bar_plot = p_value_limit * ones(1,length(F_an)); %we create a set of bar values to create the shadings, scaled to 0.05
    stem(F_an, p_vect, 'Marker', 'None')
    p1 = plot(F_an(color_ind == 1), p_vect(color_ind==1), 'ro','MarkerSize',7, 'MarkerEdgeColor','r',...
        'MarkerFaceColor',[0.8,0.4,0.4]);
    hold on
    plot_bars(F_an, color_ind, p_value_limit);%this generates the background bar plots
    p2 = plot(F_an(color_ind == 0), p_vect(color_ind==0), 'bo', 'MarkerSize', 7, 'MarkerEdgeColor','b',...
        'MarkerFaceColor',[0.4,0.4,0.8]);
    p3 = plot(F_an(color_ind == 2), p_vect(color_ind==2), 'go', 'MarkerSize', 7, 'MarkerEdgeColor','g',...
        'MarkerFaceColor',[1,1,0]);
    grid on
    grid minor;
    set(gca, 'fontsize', 20)
    xlim([0, limit_freq])
    xlabel('Frequency [Hz]')
    title(['P-values, Trials: ' num2str(cell_trials{plot_idx})])
    ylabel(['P-Values']);

end
%Plot hte logical and of the P-Values
figure(2); %Switch to the P-values figure
set(gcf,'Visible','off');
p_master = prod(p_master); %Get the logical and of the values of the P
color_ind = zeros(1,length(p_master));%set the color formatting bar
color_ind(p_master==0.0) = 2;
subplot(fold_number+4,1,plot_idx+1)%Go to the last figure
stem(F_an, p_master, 'Marker', 'None')
p1 = plot(F_an(color_ind == 1), p_master(color_ind==1), 'ro','MarkerSize',7, 'MarkerEdgeColor','r',...
    'MarkerFaceColor',[0.8,0.4,0.4]);
hold on
%plot bars that indicate the value
plot_bars(F_an, color_ind, 1);%this generates the background bar plots
p2 = plot(F_an(color_ind == 0), p_master(color_ind==0), 'bo', 'MarkerSize', 7, 'MarkerEdgeColor','b',...
    'MarkerFaceColor',[0.4,0.4,0.8]);
p3 = plot(F_an(color_ind == 2), p_master(color_ind==2), 'go', 'MarkerSize', 7, 'MarkerEdgeColor','g',...
    'MarkerFaceColor',[1,1,0]);
grid on;
grid minor;
ylim([0,1])
xlim([0, limit_freq])
xlabel('Frequency [Hz]')
title(['Logical AND of informative P-values'])
ylabel(['Logical Value']);
set(gca, 'fontsize', 20)

%plot the singe frequency regression using every channel
subplot(fold_number+4,1,plot_idx+2)%Go to the last figure
analyze_single_frequency_regression_p_values(power_matrix, large_labels, F, channel_id, fourier_sampling_rate, limit_freq, p_value_limit, normalized_flag, log_flag)
xlabel([])
%plot the result from the grouped lasso
subplot(fold_number+4, 1, plot_idx+3:plot_idx+4)
grouping_perc = 0.1; %This is the percentage for the LASSO grouping.
groupLassoAnalysis_p_values(grouping_perc, channel_id, F, power_matrix, large_labels, [0:limit_freq], fourier_sampling_rate, normalized_flag, log_flag)


for fig_idx = 1:2
    figure(fig_idx); %Format the figures so they look printable
    ha = axes('Position',[0 0 1 1],'Xlim',[0 1],'Ylim',[0 1],'Box','off','Visible','off','Units','normalized', 'clipping' , 'off');
    text(0.5, 1,main_title, 'HorizontalAlignment','center','VerticalAlignment', 'top')
    text(0.1, 1, get_variables('date_str'),'HorizontalAlignment','center','VerticalAlignment', 'top')
    figureHandle = gcf;
    set(findall(figureHandle,'type','text'),'fontSize',24,'fontWeight','bold')
    set(gcf, 'color', [1,1,1])
    set(gcf,'renderer', 'zbuffer');
    set(gcf,'outerposition',[1 1 1920 1200])
    myaa([4 2],['p_value_' num2str(p_value_limit*100) '_window_' num2str(get_variables('Window_Size')*100) '_overlap_' num2str(100*get_variables('overlap_percentage'))  '_figure_' num2str(fig_idx) plot_title])
end

end