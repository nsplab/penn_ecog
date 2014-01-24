function plot_tunning_curve(power_matrix, select_freqs, channel_id, kinematic_data, F, type)
%PLOT_TUNNING_CURVES plots the tunning curves of type 1, 2 and 3
% Tuning curve Type 1 is: y-axis:  average power in select band, x-axis:
% polar angle relative to x-axis in x-y plane, ranging from 0 to 360
% plotted in units of degrees 
% Tuning curve Type 2 is: y-axis:  average
% power in select band, x-axis: polar angle relative to z-axis in x-z
% plane. ranging from 0 to 360 plotted in units of degrees Tuning curve
% 
% Type 3 is: y-axis:  average power in select band, x-axis: polar angle
% relative to z-axis in y-z plane. ranging from 0 to 360 plotted in units
% of degrees
%---------------------------------------------------
%input: 
%------------------------------------------------------
%power_matrix: precomputed matrix 
%select_channel : channel to plot
%select_freqs: is the frequency band that we wish to plot
%kinematic_data: Tx3 matrix that has x,y,z data
%type: 1,2 or 3
chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F)); %extract the info for the channel_id
freq_chan = extract_frequency(chan_power_mat, F, select_freqs, 'average'); %extract the frequencies that we want

switch type
    case 1
        angle_kinect = calculate_angle(kinematic_data(:,1:2),1);
        plot(angle_kinect, freq_chan, 'o',  'MarkerSize', 8, 'MarkerEdgeColor', 'k','MarkerFaceColor', [0.8,0.8,0.8])
        plot_title = ['Type 1 Curve for Channel ' num2str(channel_id+64) ' and from ' num2str(select_freqs(1)) ' to ' num2str(select_freqs(end)) 'Hz'];
        xlabel('X-Y Angle wrt X [deg]')
        
    case 2
        angle_kinect = calculate_angle(kinematic_data(:,[1,3]),2);
        plot(angle_kinect, freq_chan, 'o',  'MarkerSize', 8, 'MarkerEdgeColor', 'k','MarkerFaceColor', [0.8,0.8,0.8])
        plot_title = ['Type 2 Curve for Channel ' num2str(channel_id+64) ' and from ' num2str(select_freqs(1)) ' to ' num2str(select_freqs(end)) 'Hz'];
        xlabel('X-Z Angle wrt Z [deg]')
        
    case 3
        angle_kinect = calculate_angle(kinematic_data(:,2:3),2);
        plot(angle_kinect, freq_chan, 'o',  'MarkerSize', 8, 'MarkerEdgeColor', 'k','MarkerFaceColor', [0.8,0.8,0.8])
        plot_title = ['Type 3 Curve for Channel ' num2str(channel_id+64) ' and from ' num2str(select_freqs(1)) ' to ' num2str(select_freqs(end)) 'Hz'];
        xlabel('Y-Z Angle wrt Z [deg]')
end
title(plot_title);
grid on
xlim([0,360])
ylabel('Power')
set(gca,'FontSize',10)
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',12,'fontWeight','bold')
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
end
        
    
    
        

        