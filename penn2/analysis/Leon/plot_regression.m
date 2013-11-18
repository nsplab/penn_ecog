%%This generates the regression plots witha given frequency band
%The script analyse_data.m needs to be run at least once before this script
%so it generates significant vaiables like the feature vector and the
%labels.
desired_reg_frequencies = [15:40];
%desired_reg_frequencies = get_variables('beta'); %Get the desired frequencies to analyze in the regression
%desired_reg_frequencies = get_variables('High Gamma'); %Get the desired frequencies to analyze in the regression

frequency_matrix = zeros(num_chan, length(T_axis)); %matrix that has channels in the rows and time in the X, to plot averaged frequencies
    for chan_idx = 1:num_chan
        chan_power_mat = large_power_matrix(:,(chan_idx-1)*length(F)+1:chan_idx*length(F)); %extract the info for the current channel
        channel_frequency = extract_frequency(chan_power_mat, F, desired_reg_frequencies, 'average'); %extract the frequencies that we want
        frequency_matrix(chan_idx,:) = channel_frequency;%assigns those frequencies to the channel and generate a new features vector
    end

[b,dev,stats] = glmfit(frequency_matrix' ,large_labels); % Logistic regression
[XL,YL,XS,YS,BETA,PCTVAR, MSE, statspls] = plsregress(frequency_matrix', large_labels);
[princ_comp_coeff, pcascore, latent] = princomp(frequency_matrix'); 
pca_data = frequency_matrix'*princ_comp_coeff(:,1:5);
[bpca,devpca,statspca] = glmfit(pca_data,large_labels); % Logistic regression
[bpls,devpls,statspls] = glmfit(XS,large_labels); % Logistic regression



fig1 = figure('visible','off');
subplot(2,1,1);
channel_id = [0:size(b,1)-1];
stem(channel_id, b,'r')
xlabel('Coefficient for the regression')
ylabel('Magnitude of the coefficient')
title('Regression Weights using normal regression')
%Plot whether a given power band/channel combination has a statistically significant regression coefficient.
%Color scheme is black if coefficient is not significant.
subplot(2,1,2);
stem(channel_id, stats.p, 'r')
ylim([0,0.05])
xlabel('Coefficients for the regression')
ylabel('P-values')
title('P-Values for the regression');
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'regression_weights_p_value.png')


fig2 = figure('visible','off');
subplot(2,1,1)
channel_id = [0:size(BETA,1)-1];
stem(channel_id, BETA,'r')
xlabel('Coefficient for the PLS regression')
ylabel('Magnitude of the coefficient')
title('Regression Weights using PLS')
subplot(2,1,2)
plot(1:num_chan,cumsum(100*PCTVAR(2,:)),'-bo');
xlabel('Number of PLS components');
ylabel('Percent Variance Explained in y');
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'pls_regression_weights_pctvar.png')

fig2_5 = figure('visible','off');
subplot(2,1,1);
channel_id = [0:size(b,1)-1];
stem(channel_id, bpls,'r')
xlabel('Coefficient for the regression')
ylabel('Magnitude of the coefficient')
title('Regression Weights using PLS components')
%Plot whether a given power band/channel combination has a statistically significant regression coefficient.
%Color scheme is black if coefficient is not significant.
subplot(2,1,2);
stem(channel_id, statspls.p, 'r')
ylim([0,0.05])
xlabel('Coefficients for the regression using PLS components')
ylabel('P-values')
title('P-Values for the PLS regression');
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'glmfit_regression_pls.png')

fig3 = figure('visible','off');
%figure
subplot(2,1,1);
channel_id = [0:size(bpca,1)-1];
stem(channel_id, bpca,'r')
xlabel('Coefficient for the regression using PCA')
ylabel('Magnitude of the coefficient')
title('Regression Weights using PCA')
%Plot whether a given power band/channel combination has a statistically significant regression coefficient.
%Color scheme is black if coefficient is not significant.
subplot(2,1,2);
stem(channel_id, statspca.p, 'r')
ylim([0,0.05])
xlabel('Coefficients for the regression')
ylabel('P-values')
title('P-Values for the regression');
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'PCA_regression_weights_p_value.png')

fig4 = figure('visible','off');
imagesc(princ_comp_coeff)
xlabel('Principal Component ID')
ylabel('Channel Weights')
title('PCA Heatmap')
colorbar
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'PCA_heatmap.png')

fig4 = figure('visible','off');
imagesc(XL)
xlabel('PLS Principal Component ID')
ylabel('Channel Weights')
title('PLS Heatmap')
colorbar
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'PLS_heatmap.png')