function plot_bars(frequency_array, color_array, P_value_limit)
%PLOT_BARS generates background bars that serve to visualize the
%significatn p values.
%It requires a vector of frequency, and a color array that indicates the color of each bar
%--------------------------------------------------------------------------
%input:
%-------------------------------------------------------------------------
%frequency_array: is a frequency array that has the values for all the
%frequency feature sthat we have.
%color_array: is an array of the same size of the frequency array that
%controls the color of each of the bars
%0: The value is within the p-value limit
%1: The value associated to that frequency is a NAN (color red)
%2: The value associated with that frequency is larger than the thresholded
%p_value
hist_plot_nan = []; %we create a set of bar values for the nan
hist_plot_valid = []; %we create a set of bar values for the values that pass the p_value threshold
hist_plot_invalid = []; %we create a new set of values for those that 
for freq_idx = 1:length(frequency_array)
    if color_array(freq_idx) == 0;
        hist_plot_valid = [hist_plot_valid frequency_array(freq_idx)];
        %
    elseif color_array(freq_idx) == 1;
        hist_plot_nan = [hist_plot_nan frequency_array(freq_idx)];
        %set(ch, 'facea', 1,'FaceColor', [1,1,1], 'EdgeColor', [1,1,1]);%set the bar to be equal to the stem
    else
        hist_plot_invalid = [hist_plot_invalid frequency_array(freq_idx)];
        %set(ch, 'facea', 1,'FaceColor', [205,51,51]/255, 'EdgeColor', [205,51,51]/255);%set the bar to be equal to the stem
    end
end

    hold on
    hist(hist_plot_valid, frequency_array);
    %B=bar(x,P_value_limit*h, 3.2);
    ylim([0,P_value_limit])
    H=findobj('type','patch');
    %h = get(B,'child');
    set(H, 'facea', 0.5,'FaceColor', [0,1,1], 'EdgeColor', [0,1,1]);%set the bar to be equal to the stem
    %[h,x] = hist(hist_plot_nan);
    % B=bar(x, h*P_value_limit)
    % h = get(B,'child');
     %set(h, 'facea', 1,'FaceColor', [205,51,51]/255, 'EdgeColor', [205,51,51]/255);%set the bar to be equal to the stem
end