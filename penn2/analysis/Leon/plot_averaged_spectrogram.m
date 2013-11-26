function plot_averaged_spectrogram( power_matrix, labels, force, channel_id, F, frequency_range, rise_or_fall, fourier_sampling, log_flag)
%PLOT_AVERAGED_SPECTROGRAM plots the spectrogram of a given channel, given
%a range of frequencies to plot through, all aligned to the signel onset or
%offset
%---------------------------------------------------------------
%input:
%-----------------------------------------------------------
%power_matrix: matrix of powers, usually computed using matlab spectrogram
%labels: array of 0s and 1s aligned to the onset of the squeeze
%channel_id: channel to get the spectrogram from
%F: array of frequencies
%frequency_range: range of frequencies to plot
%rise_or_fall: set if we look for raising or falling edges
chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F)); %extract the info for the channel_id
channel_frequency = extract_frequency(chan_power_mat, F, frequency_range, 'single'); %extract the frequencies that we want
%aligned to raising edges
[aligned_channel, aligned_time] = align_data(labels', channel_frequency, rise_or_fall, fourier_sampling);
[aligned_force aligned_time] = align_data(labels', force, rise_or_fall,fourier_sampling);%align to every rise in the labels for the force
[time_freq, channel_f, num_trials] = size(aligned_channel);%get the features toi create the window
size(mean(aligned_channel,3))
subplot(4,1,1:3)
if log_flag == 0
    surf(aligned_time, frequency_range, (mean(aligned_channel,3))','edgecolor', 'none')
    plot_title = [num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '_averaged_spectrogram_channel_' num2str(channel_id) '.png'];
    title([num2str(frequency_range(1)) ' to ' num2str(frequency_range(end)) ' Hz Trials aligned for Channel ' num2str(channel_id) ' without log scale, ' num2str(num_trials) ' trials' ])
else
    surf(aligned_time, frequency_range, log(mean(aligned_channel,3))','edgecolor', 'none')
    plot_title = ['log_' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '_averaged_spectrogram_channel_' num2str(channel_id) '.png'];
    title([num2str(frequency_range(1)) ' to ' num2str(frequency_range(end)) ' Hz Trials aligned for Channel ' num2str(channel_id) ' with log scale,' num2str(num_trials) ' trials' ])
end
view(0,90)
axis tight
x_limit = get(gca, 'xlim');
%xlabel('Time(s)')
ylabel('Frequencies')
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

