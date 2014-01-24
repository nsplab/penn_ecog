function speed_regression(power_matrix, channel_to_regress, desired_frequencies, F, kinematic_values)
%This function takes the desired channel(s), the frequencies and does a
%regression of the speed against the channel
%input
%---------------------------------------------
%power_matrix: matrix of pwoers (size TxN)
%channel_to_regress: channel(s) chosen to do the regression against
%desired_frequencies: frequencies to do the regressiona against
%F: frequency feature vector
%kinematic values: speed to do the regression against

%first, we extract the powers at the desired channels
average_chan = 0; %this has the average information of all the channels
for channel_id = channel_to_regress
    chan_power_mat = power_matrix(:,(channel_id-1)*length(F)+1:channel_id*length(F)); %extract the info for the channel_id
    chan_freq = extract_frequency(chan_power_mat, F, desired_frequencies, 'average'); %extract the frequencies that we want
    average_chan = average_chan + chan_freq;
end
average_chan = average_chan/length(channel_to_regress);
[b,dev,stats] = glmfit(kinematic_values ,average_chan); % Linear regression
subplot(2,1,1);
stem(b,'r')
xlabel('Coefficient for the regression')
ylabel('Magnitude of the coefficient')
title('Regression Weights using normal regression')
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
%myaa([4 2],[num2str(desired_frequencies(1)) '_to_' num2str(desired_frequencies(end)) 'regression_weights_p_value.png'])




