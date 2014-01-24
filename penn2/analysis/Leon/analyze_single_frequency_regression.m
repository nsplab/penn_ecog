function p_vect = analyze_single_frequency_regression(power_matrix, large_labels, F, channel_id, fourier_sampling_rate, limit_freq, normalized_flag, log_flag)
%ANALYZE_SINGLE_FREQUENCY_REGRESSION generates a two panel plot that shows
%the b coefficients and the p values for individual regressions in each of
%the feature frequencies.
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


rise_or_fall = 'rise';
%power_matrix = zscore(power_matrix);
[val limit_freq_index]=findNearest(limit_freq, F);
F_an = F(1:limit_freq_index);

%First, we extract the frequencies for the channel that we desire to study
chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F)); %extract the info for the channel_id
plot_title = ['analysis_single_regression' num2str(F_an(1)) '_to_' num2str(F_an(end)) '.png'];
if normalized_flag ==1
    chan_power_mat = zscore(chan_power_mat);
    plot_title = ['normalized_analysis_single_regression' num2str(F_an(1)) '_to_' num2str(F_an(end)) '.png'];
    
end
if log_flag ==1
    chan_power_mat = log(chan_power_mat);
    plot_title = ['logarithmic_scale_analysis_single_regression' num2str(F_an(1)) '_to_' num2str(F_an(end)) '.png'];
    
end

    %then we get the trials 

[aligned_channel, aligned_time] = align_data(large_labels', chan_power_mat, rise_or_fall, fourier_sampling_rate);
[aligned_force aligned_time] = align_data(large_labels', large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
[time, feat, trials] = size(aligned_channel);
dataset_perm=permute(aligned_channel,[1,3,2]);%this rearranges the indexes
dataset = reshape(dataset_perm,[],size(aligned_channel,2),1);%this does the reshaping so it has a sensible format
labels_perm = permute(aligned_force,[1,3,2]);
labels_dataset = reshape(labels_perm,[],size(aligned_force,2),1);
%set the place holders for the b and p vectors
b_vect = [];
p_vect = [];
color_ind = []; % indicates the color of the marker inc ase there is a Nan
main_title = ['Trials: ' num2str(trials) 'Window Size = 0.5 [s]'];
if normalized_flag == 1
    main_title = ['Trials: ' num2str(trials) 'Window Size = 0.5 [s], Normalized'];
end
if log_flag == 1
    main_title = ['Trials: ' num2str(trials) 'Window Size = 0.5 [s], Logarithmic-scaled features'];
end
%then, we iterate over each freqeuncy and obtain the p-value
nan_count = 0;

for freq_idx=1:length(F_an)
    [b,dev,stats] = glmfit(dataset(:, freq_idx), labels_dataset); % Linear regression
    if isnan(stats.p(2))
        color_ind = [color_ind 1]; %If there is a NaN, use flag 1
        stats.p(2) = 0; %If there is a NaN set the p value to 0
        nan_count = nan_count +1;
    elseif  stats.p(2) >=0.05 %If the p value is larger than 0.05
        color_ind = [color_ind 2]; %Setr the flag of the color to 2
        stats.p(2) = 0;
    else
        color_ind = [color_ind 0];
    end
        
    b_vect = [b_vect b(2)];
    p_vect = [p_vect stats.p(2)]; 
end
figure
%figure('visible','off');
bar_plot = 0.05 * ones(1,length(F_an)); %we create a set of bar values to create the shadings, scaled to 0.05
subplot(2,1,1)
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
increment = xlimit(2)*0.05;
%Format the minor ticks
set(gca,'XTick',[xlimit(1):increment:xlimit(2)])
set(gca,'XTickLabel',[xlimit(1):increment:xlimit(2)])
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
set(gca, 'XMinorTick','on')
subplot(2,1,2)
stem(F_an, p_vect, 'Marker', 'None')

p1 = plot(F_an(color_ind == 1), p_vect(color_ind==1), 'ro','MarkerSize',7, 'MarkerEdgeColor','r',...
    'MarkerFaceColor',[0.8,0.4,0.4]);
hold on
B = bar(F_an(color_ind == 0), bar_plot(color_ind==0), 1);
ch = get(B,'child');
set(ch, 'facea', 0.3,'FaceColor', [0,1,1], 'EdgeColor', [0,1,1]);%set the bar to be equal to the stem
p2 = plot(F_an(color_ind == 0), p_vect(color_ind==0), 'bo', 'MarkerSize', 7, 'MarkerEdgeColor','b',...
    'MarkerFaceColor',[0.4,0.4,0.8]);
p3 = plot(F_an(color_ind == 2), p_vect(color_ind==2), 'go', 'MarkerSize', 7, 'MarkerEdgeColor','g',...
    'MarkerFaceColor',[1,1,0]);
size(F_an(color_ind == 1))
size(bar_plot(color_ind==1)')
B = bar(F_an(color_ind == 1), bar_plot(color_ind==1)', 1);
ch = get(B,'child');
set(ch, 'facea', 0.3,'FaceColor', [205,51,51]/255, 'EdgeColor', [205,51,51]/255);%set the bar to be equal to the stem
grid on
grid minor;
xlim([0, limit_freq])
xlabel('Frequency [Hz]')
ylabel('P-values')
title(['P-Values']);
%text(1000,0.025,);
ylimit=get(gca,'YLim');
xlimit=get(gca,'XLim');
set(gca, 'XMinorTick','on')
set(gca,'XTick',[xlimit(1):increment:xlimit(2)])
set(gca,'XTickLabel',[xlimit(1):increment:xlimit(2)])
set(gca,'layer','top')
text(xlimit(2)*0.8,ylimit(2)*0.95,[num2str(nan_count) ' out of ' num2str(length(color_ind)) ' values were Nan'],...
   'VerticalAlignment','bottom',...
   'HorizontalAlignment','left')
set(gca, 'fontsize', 20)
if nan_count == 0, %If there is not a single nan
    legend([p2, p3],{'P-values', 'P >=0.05'});
else
    legend([p1, p2, p3],{'NaN' ,'P-values', 'P >=0.05'});
end
plot([F_an(1) F_an(end)],[0.05 0.05],'color','r','LineWidth',3)
ha = axes('Position',[0 0 1 1],'Xlim',[0 1],'Ylim',[0 1],'Box','off','Visible','off','Units','normalized', 'clipping' , 'off');
text(0.5, 1,main_title, 'HorizontalAlignment','center','VerticalAlignment', 'top')
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',24,'fontWeight','bold')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
set(gcf,'outerposition',[1 1 1920 1200])
myaa([4 2],plot_title)

