function num_trials = plot_averaged_spectrogram_sub( power_matrix, labels, force, channel_id, F, frequency_range, rise_or_fall, fourier_sampling, log_flag)
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
if log_flag == 0
    surf(aligned_time, frequency_range, (mean(aligned_channel,3))','edgecolor', 'none')
    title(['Channel ' num2str(channel_id+64)])
else
    surf(aligned_time, frequency_range, log(mean(aligned_channel,3))','edgecolor', 'none')
    title(['Channel ' num2str(channel_id+64)])
end
view(0,90)
axis tight
xlabel('Time(s)')
ylabel('Frequencies')
set(gca,'FontSize',14)
set(gcf, 'color', [1,1,1])
set(gca,'FontSize',14)

end

