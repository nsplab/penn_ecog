%this script plots the grid 
plot_count = 1;
desired_freqs = [72:82];
type = 1;
chan_to_plot = 3:14;
subplot_y = ceil(length(chan_to_plot)/4);
subplot_x = ceil(length(chan_to_plot)/subplot_y);
for ch_idx = chan_to_plot
    subplot(subplot_y, subplot_x, plot_count);
    plot_tunning_curve(large_power_matrix, desired_freqs, ch_idx, kinect_data, F, type)
    plot_count = plot_count +1;
end
sup_title = ['Trials:' num2str(0) ', Window Size:' num2str(get_variables('Window_Size')) '[sec], ' (num2str(floor(size(T,2)/T(end)))) '[FFT/sec]'];
ha = axes('Position',[0 0 1 1],'Xlim',[0 1],'Ylim',[0 1],'Box','off','Visible','off','Units','normalized', 'clipping' , 'off');
text(0.5, 0.98 ,sup_title,'HorizontalAlignment' ,'center','VerticalAlignment', 'top','fontSize',12,'fontWeight','bold')
set(gcf,'units','normalized','outerposition',[0 0 1 1]);
plot_title = ['type_' num2str(type), '.png'];
myaa([4 2], plot_title)


    
    