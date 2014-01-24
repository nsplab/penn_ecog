%This script goes for different lasso features and plots them in a 3x1
%subplot
for end_freq_idx = 100:100:400
    %figure('visible','off');
    figure
    plot_title = ['elastic_net_analysis_' num2str(end_freq_idx) '_hz.png'];
    subplot(3,1,1)
    elastic_net_regression(large_power_matrix, large_labels, F, 1, fourier_sampling_rate, [0:end_freq_idx], 0,0);
    subplot(3,1,2)
    elastic_net_regression(large_power_matrix, large_labels, F, 1, fourier_sampling_rate, [0:end_freq_idx], 1,0);
    subplot(3,1,3)
    elastic_net_regression(large_power_matrix, large_labels, F, 1, fourier_sampling_rate, [0:end_freq_idx], 0,1);
    set(gcf, 'color', [1,1,1])
    set(gcf,'renderer', 'zbuffer');
    set(gcf,'outerposition',[1 1 1920 1200])
    myaa([4 2],plot_title)
end

