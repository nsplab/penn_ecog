%This analysis takes the large matrix of features and crops it just around
%the trials. The objective is to decrease noise induced by periods of time
%where the patient may have not been squeezing.
figure
channel_id = 4; %We choose a single channel
frequency_range = [1:200];
rise_or_fall = 'rise';
chan_power_mat = zscore(large_power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F))); %extract the info for the channel_id


[freqs channel_frequency] = extract_frequency_reg(chan_power_mat, F, frequency_range, 'single'); %extract the frequencies that we want
[time_freqs, nfreqs] = size(channel_frequency);
%aligned to raising edges
plot_idx=1;
for num_trials = 10:10:40    
    subplot(4,1,plot_idx)
    num_trials
    [aligned_channel, aligned_time] = align_data(large_labels', channel_frequency, rise_or_fall, fourier_sampling_rate);
    [aligned_force aligned_time] = align_data(large_labels', large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
    aligned_channel = aligned_channel(:,:,20:num_trials+20);
    aligned_force = aligned_force(:,:,20:num_trials+20);
    dataset_perm=permute(aligned_channel,[1,3,2]);%this rearranges the indexes
    dataset = reshape(dataset_perm,[],size(aligned_channel,2),1);%this does the reshaping so it has a sensible format
    labels_perm = permute(aligned_force,[1,3,2]);
    labels_dataset = reshape(labels_perm,[],size(aligned_force,2),1);
    [b,dev,stats] = glmfit(dataset, labels_dataset); % Linear regression
    stem(freqs, stats.p(2:end))
    xlabel('Frequency')
    ylabel('P-values')
    title(['P-Values for ' num2str(num_trials) ' Trials']);
    hold on
    plot([frequency_range(1) frequency_range(end)],[0.05 0.05],'color','r','LineWidth',3)
    plot_idx = plot_idx+1
    set(gca, 'fontsize', 14)
end
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',14,'fontWeight','bold')
figure
plot_idx = 1;
for num_trials = 10:10:40    
    subplot(4,1,plot_idx)
    num_trials
    [aligned_channel, aligned_time] = align_data(large_labels', channel_frequency, rise_or_fall, fourier_sampling_rate);
    [aligned_force aligned_time] = align_data(large_labels', large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
    aligned_channel = aligned_channel(:,:,20:num_trials+20);
    aligned_force = aligned_force(:,:,20:num_trials+20);
    dataset_perm=permute(aligned_channel,[1,3,2]);%this rearranges the indexes
    dataset = reshape(dataset_perm,[],size(aligned_channel,2),1);%this does the reshaping so it has a sensible format
    labels_perm = permute(aligned_force,[1,3,2]);
    labels_dataset = reshape(labels_perm,[],size(aligned_force,2),1);
    [b,dev,stats] = glmfit(dataset, labels_dataset); % Linear regression
    stem(freqs, b(2:end))
    xlabel('Frequency')
    ylabel('B-values')
    title(['B-Values for ' num2str(num_trials) ' Trials']);
    hold on
    plot([frequency_range(1) frequency_range(end)],[0.05 0.05],'color','r','LineWidth',3)
    plot_idx = plot_idx+1
    set(gca, 'fontsize', 14)
end
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',14,'fontWeight','bold')
figure

plot_idx=1;
for num_trials = 10:10:40    
    subplot(4,1,plot_idx)
    num_trials
    [aligned_channel, aligned_time] = align_data(large_labels', channel_frequency, rise_or_fall, fourier_sampling_rate);
    [aligned_force aligned_time] = align_data(large_labels', large_labels, rise_or_fall,fourier_sampling_rate);%align to every rise in the labels for the force
    aligned_channel = aligned_channel(:,:,20:num_trials+20);
    aligned_force = aligned_force(:,:,20:num_trials+20);
    dataset_perm=permute(aligned_channel,[1,3,2]);%this rearranges the indexes
    dataset = reshape(dataset_perm,[],size(aligned_channel,2),1);%this does the reshaping so it has a sensible format
    labels_perm = permute(aligned_force,[1,3,2]);
    labels_dataset = reshape(labels_perm,[],size(aligned_force,2),1);
    [princ_comp_coeff, pcascore, latent] = princomp(dataset); 
    pca_data = dataset*princ_comp_coeff;
    [b,dev,stats] = glmfit(pca_data,labels_dataset); % Logistic regression
    stem([1 freqs], stats.p(1:end))
    xlabel('Frequency')
    ylabel('P-values')
    title(['P-Values for ' num2str(num_trials) ' Trials']);
    hold on
    plot([frequency_range(1) frequency_range(end)],[0.05 0.05],'color','r','LineWidth',3)
    plot_idx = plot_idx+1
    set(gca, 'fontsize', 14)
end
ha = axes('Position',[0 0 1 1],'Xlim',[0 1],'Ylim',[0 1],'Box','off','Visible','off','Units','normalized', 'clipping' , 'off');
text(0.5, 1,'PCA','HorizontalAlignment' ,'center','VerticalAlignment', 'top')
figureHandle = gcf;
set(findall(figureHandle,'type','text'),'fontSize',14,'fontWeight','bold')