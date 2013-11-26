%This generate significant plots for the data recorded.
%This script needs for analyse_datya.m to be run before so it can pull the
%environment variables and generate the plots
fig1 = figure('visible','off');
subplot(3,1,1)
spectogram_single_channel(1, large_power_matrix, T_axis, F)%Plot the spectrogram of the first channel
title('Spectrogram for Channel 1')
xlimit = get(gca, 'XLim');
xlim([0, xlimit(2)])
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',14,'fontWeight','bold')
subplot(3,1,2)
plot(T_axis, large_force)
xlim([0, xlimit(2)])
ylabel('Force Units')
title(['Force Sensor Input subsampled to ' num2str(desired_samplingRate) ' Hz'])
subplot(3,1,3)
plot(raw_time_axis, raw_data)%Plot the raw data
title('Raw Data')
xlim([0, xlimit(2)])
set(gca,'Color',[1 1 1]);
grid on
xlabel('Time(s)')
ylabel('Volts')
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',14,'fontWeight','bold')
set(gca,'FontSize',14)
set(gcf, 'color', [1,1,1])
set(gcf,'units','normalized','outerposition',[0 0 0.4 0.7])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'raw_data_spectrogram.png');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%Plot the aligned rising edges
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

fig3 = figure('visible','off');
%figure
frequency_matrix = zeros(num_chan, length(T_axis)); %matrix that has channels in the rows and time in the X, to plot averaged frequencies
desired_frequencies = [12:30]; %Frequencies in the Beta Band
for chan_idx = 1:num_chan
    chan_power_mat = large_power_matrix(:,(chan_idx-1)*length(F)+1:chan_idx*length(F)); %extract the info for the current channel
    channel_frequency = extract_frequency(chan_power_mat, F, desired_frequencies, 'average'); %extract the frequencies that we want
    frequency_matrix(chan_idx,:) = channel_frequency;%assigns those frequencies to the channel
end
%surf(T_axis,[1:num_chan],10*log10(frequency_matrix),'edgecolor','none'); axis tight; %plot them
view(0,90)%change the vie
%generate the aligned values
[aligned_mat aligned_time] = align_data(large_labels', frequency_matrix', 'rise', fourier_sampling_rate);%align to every rise in the labels
[aligned_force aligned_time] = align_data(large_labels', large_force, 'rise', fourier_sampling_rate);%align to every rise in the labels
subplot(4,1,1:3)
if log_flag == 1
    surf(aligned_time,[1:num_chan],10*log(mean(aligned_mat,3)'),'edgecolor','none'); axis tight;
    file_id = 'beta_rising_log.png';
else
    surf(aligned_time,[1:num_chan],(mean(aligned_mat,3)'),'edgecolor','none'); axis tight;
    file_id = 'beta_rising.png';
end
colorbar('location','SouthOutside')
title('Beta band (average power at 12-30 Hz) for all channels, aligned to rising edge ')
ylabel('Channels')
view(0,90)
subplot(4,1,4)
plot(aligned_time, mean(aligned_force,3))
xlabel('Time[s]')
ylabel('Onset')
axis tight
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],file_id)


%figure
fig4 = figure('visible','off');
frequency_matrix = zeros(num_chan, length(T_axis)); %matrix that has channels in the rows and time in the X, to plot averaged frequencies
desired_frequencies = [80:100]; %Frequencies in the Gamma Band
for chan_idx = 1:num_chan
    chan_power_mat = large_power_matrix(:,(chan_idx-1)*length(F)+1:chan_idx*length(F)); %extract the info for the current channel
    channel_frequency = extract_frequency(chan_power_mat, F, desired_frequencies, 'average'); %extract the frequencies that we want
    frequency_matrix(chan_idx,:) = channel_frequency;%assigns those frequencies to the channel
end
%surf(T_axis,[1:num_chan],10*log10(frequency_matrix),'edgecolor','none'); axis tight; %plot them
view(0,90)%change the view

%generate the aligned values
[aligned_mat aligned_time] = align_data(large_labels', frequency_matrix', 'rise', fourier_sampling_rate);%align to every rise in the labels
[aligned_force aligned_time] = align_data(large_labels', large_force, 'rise', fourier_sampling_rate);%align to every rise in the labels for the force
subplot(4,1,1:3)
if log_flag == 1
    surf(aligned_time,[1:num_chan],10*log(mean(aligned_mat,3)'),'edgecolor','none'); axis tight;
    file_id = 'gamma_rising_log.png';
else
    surf(aligned_time,[1:num_chan],(mean(aligned_mat,3)'),'edgecolor','none'); axis tight;
    file_id = 'gamma_rising.png';
end
title('Gamma band (average power at 65-115 Hz) for all channels, aligned to rising edge ')
colorbar('location','SouthOutside')
ylabel('Channels')
view(0,90)
subplot(4,1,4)
plot(aligned_time, mean(aligned_force,3))
xlabel('Time[s]')
ylabel('Force [Force Units]')
axis tight;
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2], file_id)



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%Plot the aligned falling edges
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

fig5 = figure('visible','off')
%figure
frequency_matrix = zeros(num_chan, length(T_axis)); %matrix that has channels in the rows and time in the X, to plot averaged frequencies
desired_frequencies = [12:30]; %Frequencies in the Beta Band
for chan_idx = 1:num_chan
    chan_power_mat = large_power_matrix(:,(chan_idx-1)*length(F)+1:chan_idx*length(F)); %extract the info for the current channel
    channel_frequency = extract_frequency(chan_power_mat, F, desired_frequencies, 'average'); %extract the frequencies that we want
    frequency_matrix(chan_idx,:) = channel_frequency;%assigns those frequencies to the channel
end
%surf(T_axis,[1:num_chan],10*log10(frequency_matrix),'edgecolor','none'); axis tight; %plot them
view(0,90)%change the view

%generate the aligned values
[aligned_mat aligned_time] = align_data(large_labels', frequency_matrix', 'fall', fourier_sampling_rate);%align to every rise in the labels
[aligned_force aligned_time] = align_data(large_labels', large_force, 'fall', fourier_sampling_rate);%align to every rise in the labels
subplot(4,1,1:3)
if log_flag == 1
    surf(aligned_time,[1:num_chan],10*log(mean(aligned_mat,3)'),'edgecolor','none'); axis tight;
    file_id = 'beta_falling_log.png';
else
    surf(aligned_time,[1:num_chan],(mean(aligned_mat,3)'),'edgecolor','none'); axis tight;
    file_id = 'beta_falling.png';
end
colorbar('location','SouthOutside')
title('Beta band (average power at 12-30 Hz) for all channels, aligned to falling edge ')
ylabel('Channels')
view(0,90)
subplot(4,1,4)
plot(aligned_time, mean(aligned_force,3))
xlabel('Time[s]')
ylabel('Force [Force Units]')
axis tight;
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],file_id)


fig6 = figure('visible','off')
%figure
frequency_matrix = zeros(num_chan-8, length(T_axis)); %matrix that has channels in the rows and time in the X, to plot averaged frequencies
desired_frequencies = [80:100]; %Frequencies in the Gamma Band
for chan_idx = 1:num_chan
    chan_power_mat = large_power_matrix(:,(chan_idx-1)*length(F)+1:chan_idx*length(F)); %extract the info for the current channel
    channel_frequency = extract_frequency(chan_power_mat, F, desired_frequencies, 'average'); %extract the frequencies that we want
    frequency_matrix(chan_idx,:) = channel_frequency;%assigns those frequencies to the channel
end
%surf(T_axis,[1:num_chan],10*log10(frequency_matrix),'edgecolor','none'); axis tight; %plot them
view(0,90)%change the view

%generate the aligned values
[aligned_mat aligned_time] = align_data(large_labels', frequency_matrix', 'fall', fourier_sampling_rate);%align to every rise in the labels
[aligned_force aligned_time] = align_data(large_labels', large_force, 'fall', fourier_sampling_rate);%align to every rise in the labels for the force
subplot(4,1,1:3)
if log_flag == 1
    surf(aligned_time,[1:num_chan],10*log(mean(aligned_mat,3)'),'edgecolor','none'); axis tight;
    file_id = 'gamma_falling_log.png';
else
    surf(aligned_time,[1:num_chan],(mean(aligned_mat,3)'),'edgecolor','none'); axis tight;
    file_id = 'gamma_falling.png';
end
title('Gamma band (average power at 65-115 Hz) for all channels, aligned to falling edge ')
ylabel('Channels')
colorbar('location','SouthOutside')
view(0,90)
subplot(4,1,4)
plot(aligned_time, mean(aligned_force,3))
xlabel('Time[s]')
ylabel('Force [Force Units]')
axis tight;
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2], file_id)
    
