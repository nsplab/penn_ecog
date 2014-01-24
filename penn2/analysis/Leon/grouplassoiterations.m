%group Lasso iterations
%show the group lasso iterations for different size of groups
plot_idx = 1;
frequency_range = [0:300];
norm_flag = 0;
log_flag = 0;
for grouping_idx = 0.1:0.1:0.5
    subplot(5,1,plot_idx)
    [F_feat group] = groupLassoAnalysis(4, F, large_power_matrix, large_labels, frequency_range,fourier_sampling_rate, grouping_idx, norm_flag, log_flag);
    freq_groups = grouping_idx*length(frequency_range)
    hold on
    cc=lines(length(group));
    ylimit=get(gca,'YLim');
    set(gca,'layer','top')
    for group_idx = 1:length(group)
        ntp = length(F_feat(group{group_idx}));
        B=area(F_feat(group{group_idx}), ylimit(2)*ones(ntp,1));
        ch = get(B,'child');
        set(ch, 'facea', 0.3,'edgea',0.3,'FaceColor', cc(group_idx,:), 'EdgeColor', cc(group_idx,:));%set the bar to be equal to the stem
        B = area(F_feat(group{group_idx}), ylimit(1)*ones(ntp,1));
        ch = get(B,'child');
        set(ch, 'facea', 0.3,'edgea',0.3,'FaceColor', cc(group_idx,:), 'EdgeColor', cc(group_idx,:));%set the bar to be equal to the stem
    end
    xlim([frequency_range(1) frequency_range(end)])
    plot_idx = plot_idx+1;
    grid on
end
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
set(gcf,'outerposition',[1 1 1920 1200])
plot_title = ['analysis_single_regression' num2str(F_feat(1)) '_to_' num2str(F_feat(end)) '.png'];
if norm_flag ==1
    plot_title = ['normalized_analysis_grouplasso_regression' num2str(F_feat(1)) '_to_' num2str(F_feat(end)) '.png'];
    
end
if log_flag ==1
    plot_title = ['logarithmic_scale_analysis_grouplasso_regression' num2str(F_feat(1)) '_to_' num2str(F_feat(end)) '.png'];
    
end
myaa([4 2],plot_title)