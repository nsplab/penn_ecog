channels_to_plot = [3:3+11];
%frequency_range = [72:82];
rise_or_fall = 'rise';
plot_idx = 1;
log_flag = 0;
figure
for chan_idx = channels_to_plot
    subplot(10,4,[plot_idx, plot_idx+4, plot_idx+8])
    num_trials = plot_averaged_spectrogram_sub( large_power_matrix, large_labels, large_force, chan_idx, F, frequency_range, rise_or_fall, fourier_sampling_rate, log_flag)
    colorbar('location','SouthOutside')
    x_limit = get(gca, 'xlim');
    if mod(plot_idx, 4) == 0
        plot_idx=plot_idx+9;
    else
        plot_idx = plot_idx +1;
    end
end
[aligned_force aligned_time] = align_data(large_labels', large_force, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
for plot_idx = 37:40
    subplot(10,4,plot_idx)
    hold on
    for ind = 1:num_trials
        plot(aligned_time, aligned_force(:,:,ind),'Color',[0.8, 0.8, 0.8])
    end
    plot(aligned_time, mean(aligned_force,3), 'k' ,'LineWidth', 3)
    xlim(x_limit)
    xlabel('Time[s]')
    ylabel('Force [Force Units]')
    grid on
end
ha = axes('Position',[0 0 1 1],'Xlim',[0 1],'Ylim',[0 1],'Box','off','Visible','off','Units','normalized', 'clipping' , 'off');
if log_flag == 1
    title = ['\bf Averaged trials from ' num2str(frequency_range(1)) ' to ' num2str(frequency_range(end)) ' Hz with log scale'];
    plot_title = ['log_' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '_grid_average_channel_' num2str(channels_to_plot(1)) '.png'];
else
    title = ['\bf Averaged trials' num2str(frequency_range(1)) ' to ' num2str(frequency_range(end)) ' Hz'];
    plot_title = [num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '_grid_average_channel_' num2str(channels_to_plot(1)) '.png'];

end
text(0.5, 1,title,'HorizontalAlignment' ,'center','VerticalAlignment', 'top')
data_title = ['Trials:' num2str(num_trials) ', Window Size:' num2str(get_variables('Window_Size')) '[sec], ' (num2str(floor(size(T,2)/T(end)))) '[FFT/sec]'];
text(0.5, 0.98 ,data_title,'HorizontalAlignment' ,'center','VerticalAlignment', 'top')
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',12,'fontWeight','bold')
set(gcf,'outerposition',[1 1 1920 1200])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],plot_title)