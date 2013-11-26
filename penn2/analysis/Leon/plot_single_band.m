%This generate significant plots for the data recorded.
%This script needs for analyse_datya.m to be run before so it can pull the
%environment variables and generate the plots
%fig1 = figure('visible','off');
figure
chan_idx = 3;
base_value = 300;
end_value = 5000;
desired_frequencies = [12:42];
channels_to_plot = [3:9];
average_chan = 0;
for channel_id = channels_to_plot
    chan_power_mat = large_power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F)); %extract the info for the channel_id
    freq_chan = extract_frequency(chan_power_mat, F, desired_frequencies, 'average'); %extract the frequencies that we want
    average_chan = average_chan + freq_chan;
end
average_chan = average_chan/length(channels_to_plot);
channel_frequency = extract_frequency(chan_power_mat, F, desired_frequencies, 'average'); %extract the frequencies that we want
[haxes,hline1,hline2] = plotyy(T_axis(base_value:end_value),average_chan(base_value:end_value),T_axis(base_value:end_value),large_force(base_value:end_value),'plot','plot');
set(hline2,'LineWidth',2,'Color', [0.8,0.8,0.8])
title(['Power for Grid /Band: ' num2str(desired_frequencies(1)) ' to ' num2str(desired_frequencies(end)) ' [Hz]'])
axes(haxes(1));
ylabel('Power')
axes(haxes(2));
ylabel('Force Units')
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',14,'fontWeight','bold')
grid on
set(gcf, 'color', [1,1,1])
set(gcf,'units','normalized','outerposition',[0 0 0.4 0.7])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],['grid_plot_aligned_single_band_' num2str(desired_frequencies(1)) '_to_' num2str(desired_frequencies(end)) '.png']);