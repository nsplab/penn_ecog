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

offset_time = 0.50; %offset in seconds
%[large_labels, large_power_matrix] = offset_label(large_power_matrix, large_labels', offset_time, 'positive');%add an offset to the labels
%large_labels = large_labels';

[b,dev,stats] = glmfit(frequency_matrix' ,large_labels); % Logistic regression
[XL,YL,XS,YS,BETA,PCTVAR, MSE, statspls] = plsregress(frequency_matrix', large_labels);
[princ_comp_coeff, pcascore, latent] = princomp(frequency_matrix'); 
pca_data = frequency_matrix'*princ_comp_coeff(:,1:8);
[bpca,devpca,statspca] = glmfit(pca_data,large_labels); % Logistic regression


fig1 = figure('visible','off');
subplot(2,1,1);
stem(b,'r')
xlabel('Coefficient for the regression')
ylabel('Magnitude of the coefficient')
title('Regression Weights')
%Plot whether a given power band/channel combination has a statistically significant regression coefficient.
%Color scheme is black if coefficient is not significant.
subplot(2,1,2);
stem(stats.p, 'r')
ylim([0,0.05])
xlabel('Coefficients for the regression')
ylabel('P-values')
title('P-Values for the regression');
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'regression_weights_p_value.png')


fig2 = figure('visible','off');
subplot(2,1,1)
stem(BETA,'r')
xlabel('Coefficient for the PLS regression')
ylabel('Magnitude of the coefficient')
title('Regression Weights using PLS')
subplot(2,1,2)
plot(1:num_chan,cumsum(100*PCTVAR(2,:)),'-bo');
xlabel('Number of PLS components');
ylabel('Percent Variance Explained in y');
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'pls_regression_weights_pctvar.png')

fig3 = figure('visible','off');
%figure
subplot(2,1,1);
stem(bpca,'r')
xlabel('Coefficient for the regression using PCA')
ylabel('Magnitude of the coefficient')
title('Regression Weights using PCA')
%Plot whether a given power band/channel combination has a statistically significant regression coefficient.
%Color scheme is black if coefficient is not significant.
subplot(2,1,2);
stem(statspca.p, 'r')
ylim([0,0.05])
xlabel('Coefficients for the regression')
ylabel('P-values')
title('P-Values for the regression');
set(gcf, 'color', [1,1,1])
set(gcf,'renderer', 'zbuffer');
myaa([4 2],'PCA regression_weights_p_value.png')