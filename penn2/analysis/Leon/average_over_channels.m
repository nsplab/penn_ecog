function average_over_channels(power_matrix, labels, force, channels_to_plot, frequency_range, F, fourier_sampling, rise_or_fall, log_flag)
%GENERATE PLOT OVER A GIVEN NUMBER OF CHANNELS it will present the average
%for single trials

average_chan = 0
for channel_id = channels_to_plot
    chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F)); %extract the info for the channel_id
    freq_chan = extract_frequency(chan_power_mat, F, frequency_range, 'average'); %extract the frequencies that we want
    average_chan = average_chan + freq_chan;
end
average_chan = average_chan/length(channels_to_plot);
[aligned_channel, aligned_time] = align_data(labels', average_chan, rise_or_fall, fourier_sampling);
[aligned_force aligned_time] = align_data(labels', force, rise_or_fall,fourier_sampling);%align to every rise in the labels for the force
[time_freq, channel_f, num_trials] = size(aligned_channel);%get the features toi create the window
trial_matrix = zeros(num_trials, time_freq);%create the place holder matrix
for trial_idx = 1:num_trials
    channel_frequency = aligned_channel(:,:,trial_idx); %extract the frequencies that we want
    trial_matrix(trial_idx,:) = channel_frequency;%assigns those frequencies to the channel
end
subplot(4,1,1:3)
if log_flag == 0
    surf(aligned_time, [1:num_trials], (trial_matrix),'edgecolor', 'none')
    plot_title = [num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '_averaged_grid_channel_' num2str(channel_id) '.png'];
    title([num2str(frequency_range(1)) ' to ' num2str(frequency_range(end)) ' Hz Trials aligned for Grid without log scale, ' num2str(num_trials) ' trials' ])
else
    surf(aligned_time, [1:num_trials], log(trial_matrix),'edgecolor', 'none')
    plot_title = ['log_' num2str(frequency_range(1)) '_to_' num2str(frequency_range(end)) '_averaged_grid_channel_' num2str(channel_id) '.png'];
    title([num2str(frequency_range(1)) ' to ' num2str(frequency_range(end)) ' Hz Trials aligned for Grid  with log scale,' num2str(num_trials) ' trials' ])
end
view(0,90)
axis tight
x_limit = get(gca, 'xlim');
%xlabel('Time(s)')
ylabel('Trials')
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
