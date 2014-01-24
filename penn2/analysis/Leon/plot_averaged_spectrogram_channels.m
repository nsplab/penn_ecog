function num_trials = plot_averaged_spectrogram_channels( power_matrix, labels, force, channels_to_plot, F, frequency_range, rise_or_fall, fourier_sampling, log_flag)
%PLOT_AVERAGED_SPECTROGRAM plots the spectrogram of a given channel, given
%a range of frequencies to plot through, all aligned to the signel onset or
%offset
%---------------------------------------------------------------
%input:
%-----------------------------------------------------------
%power_matrix: matrix of powers, usually computed using matlab spectrogram
%labels: array of 0s and 1s aligned to the onset of the squeeze
%channel__set: channel to get the spectrogram from
%F: array of frequencies
%frequency_range: range of frequencies to plot
%rise_or_fall: set if we look for raising or falling edges
%here we extract everything to generate the average over channels
average_chan = 0; %set to zero the average channel;s
for channel_id = channels_to_plot
    chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F)); %extract the info for the channel_id
    freq_chan = extract_frequency(chan_power_mat, F, frequency_range, 'single'); %extract the frequencies that we want
    average_chan = average_chan + freq_chan;
end
average_chan = average_chan/length(channels_to_plot);
%aligned to raising edges
[aligned_channel, aligned_time] = align_data(labels', average_chan, rise_or_fall, fourier_sampling);
[aligned_force aligned_time] = align_data(labels', force, rise_or_fall,fourier_sampling);%align to every rise in the labels for the force
[time_freq, channel_f, num_trials] = size(aligned_channel);%get the features toi create the window
subplot(4,1,1:3)
if log_flag == 0
    surf(aligned_time, frequency_range, (mean(aligned_channel,3))','edgecolor', 'none')
    plot_title = [num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '_averaged_freq_grid_channel_' num2str(channel_id) '.png'];
    title([num2str(frequency_range(1)) ' to ' num2str(frequency_range(end)) ' Hz Averaged Powers for Grid without log scale, ' num2str(num_trials) ' trials' ])
else
    surf(aligned_time, frequency_range, log(mean(aligned_channel,3))','edgecolor', 'none')
    plot_title = ['log_' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '_averaged_freq_grid_channel_' num2str(channel_id) '.png'];
    title([num2str(frequency_range(1)) ' to ' num2str(frequency_range(end)) ' Hz Averaged Powers for Grid without log scale, ' num2str(num_trials) ' trials' ])
end
view(0,90)
colorbar('location','SouthOutside')
axis tight
xlabel('Time(s)')
ylabel('Frequencies')
x_limit = get(gca, 'xlim');
set(gca,'FontSize',14)
set(gcf, 'color', [1,1,1])
set(gca,'FontSize',14)
subplot(4,1,4)
hold on
for ind = 1:num_trials
    plot(aligned_time, aligned_force(:,:,ind),'Color',[0.8, 0.8, 0.8])
end
plot(aligned_time, mean(aligned_force,3), 'k' ,'LineWidth', 3)
xlim(x_limit)
xlabel('Time[s]')
ylabel('Force [Force Units]')
grid on
set(gcf, 'color', [1,1,1])
set(gca,'FontSize',14)
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',14,'fontWeight','bold')
set(gcf,'outerposition',[1 1 1192 804])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],plot_title)
end

