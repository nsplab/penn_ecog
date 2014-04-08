%This script loads the data from the PennPrject dataset into the matlab
%environment
%clear
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Set the critical variables
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
num_chan = get_variables('number_of_channels');
num_record_chan =  get_variables('number_recorded_channels');
originalSamplingRate = get_variables('Original_Sampling_Rate');
desired_samplingRate = get_variables('Desired_Sampling_Rate'); %This is the desired post decimating sampling rate
desired_reg_frequencies = get_variables('beta'); %Get the desired frequencies to analyze in the regression
%desired_reg_frequencies = get_variables('High Gamma'); %Get the desired frequencies to analyze in the regression
ref_channel = get_variables('Reference_Channel');
decimate_factor = floor(originalSamplingRate/desired_samplingRate);%set the decimation factor
samplingRate = originalSamplingRate/decimate_factor; %The 25000 is hardcoded to the sampling rate we used to do the data capture.
window_size = get_variables('Window_Size'); %size of the window in seconds
window_size = floor(window_size * samplingRate); %transform the window size to samples
overlap_perc = get_variables('overlap_percentage');
win_overlap = floor(window_size * overlap_perc);
size_of_batch = 30; %size of the batch in seconds
%first_batch = 5;
first_batch = 1;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Read the data files
%root_path = ['/home/leon/Data/Penn/Nov_12'];
%time_stamps_file = [root_path '/data_click_Tue_01.10.2013_10:12:28'];
%data_file = [root_path '/data'];
%data_file = 'hemi_1';

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Given the batch size and the sampling rate we calculate the number of
%batches to iterate over
time_stamp_bytes = get_variables('Time_stamp_bytes');
num_4_byte_column = 4+num_record_chan; %number of columns that have 4 bytes 
dinfo = dir(data_file); %get the information of the data file
num_rows = dinfo.bytes/(time_stamp_bytes+num_4_byte_column*4);% we need to divide the total amount of bytes by the amount of bytes per row.
%The amount of bytes per row depends on the data format
total_time = num_rows/originalSamplingRate;%calculate the total time of captured data
batch_size_samples = floor((size_of_batch/total_time)*num_rows);%calculate the number of samples that correspond to the desired size
%calculate the maximum number of batches
max_num_batches = ceil(total_time/size_of_batch)-1;
%max_num_batches = 13;
%finsih reading data files



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Create place holders where 


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
tic
large_power_matrix = [];
large_labels = [];
T_axis=[];
large_force =[];
%Start the batched analysis
batch_counter = 0;
for batch_idx = first_batch:max_num_batches
    batch_idx
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    channels_data = return_batch(data_file, size_of_batch, originalSamplingRate, batch_idx, 'eeg');
    force = return_batch(data_file, size_of_batch, originalSamplingRate, batch_idx, 'force');
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %The following code denoises the singla using Andrew's technique.
    %for ch_idx = 1:num_chan
    %    denoised_channel = mtmlinenoise(channels_data(:,ch_idx),2.5,window_size, originalSamplingRate, [62]);
    %    channels_data(:,ch_idx) = channels_data(:,ch_idx) - denoised_channel;
    %end

    %pre process data files (trim, decimate)
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    x=decimate(channels_data(:,1), decimate_factor);%set a decimation on the first channel to get the value of the matrix
    force = decimate(force, decimate_factor);
    channels_data_dec = zeros(size(x,1), num_chan);%generate matrix for speed purposes
    channels_data_dec(:,1) = x; %allocate the first channel
    for chidx=2:num_chan
        channels_data_dec(:,chidx) = decimate(channels_data(:,chidx), decimate_factor); %do the decimation of the data
    end
    channels_data = channels_data_dec;
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  
    %clean the workspace to free up memory
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%55
    clear x;
    clear channels_data_dec;
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%5
    %Adding Reference to the channels
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %channels_data = reference_strip(channels_data, 'baseline');
    %reference_data = repmat(mean(channels_data,2),1,num_chan);%obtain the mean and repeat it over the number of channels for the later operation.
    %reference_data = repmat(channels_data(:,ref_channel),1,num_chan);
    %%choose a reference channel
    %channels_data=channels_data-reference_data; %rest the mean to normalize

        
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%Check if it is the first iteration to generate the placeholder
    %%%matrix for the raw data
    [time, data] = size(channels_data); %get the size of the data

    if batch_idx == first_batch
        raw_data =zeros(time*(max_num_batches-first_batch), data);% generate the place holder matrix
      
    end
    start_idx = (batch_idx-1)*time+1;
    if batch_idx~=max_num_batches
        raw_data(start_idx:batch_idx*time,:) = channels_data;
    else
        raw_data = [raw_data; channels_data];%The last batch might not be consistent, so we only append it
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %calculate the spectrogram for the first channel to generate the matrix
    [S,F,T,P] = spectrogram(channels_data(:,1), hann(window_size), win_overlap, window_size, samplingRate);
    [power_p, time_p] = size(P);
    power_matrix = zeros(time_p, power_p * num_chan); %generate the nameholder matrix for the pwoers
    power_matrix(:,1:power_p) = P'; %allocate the powers from the first channels
    
      
    
    for chidx = 2:num_chan
        [S,F,T,P] = spectrogram(channels_data(:,chidx), blackmanharris(window_size), win_overlap, window_size, samplingRate);  %transform the channel
        pow_idx = (chidx-1)*power_p+1;
        power_matrix(:,pow_idx:chidx*power_p) = P'; %allocate the powers from the channels
    end

    %code to match the force sensors with the spectrogram data
    time_axis = (1:size(force,1))/samplingRate;%generate the time series of the force
    [TF, match_idx] = findNearest(T,time_axis); %is member looks up which indexes match the output from the 
    force = force(match_idx,1);%recapture the force using the matched indexes
    %force_threshold =  0.0170559; %This is hardocded from the baseline of the data
    force_threshold =  0.2; %This is hardocded from the baseline of the data
    labels = labelize(force, force_threshold);
    %the next part of code is to generate placeholders to prevent overflows
    %in windows computers
    
%     if batch_idx == 1 %if we have the first batch, we generate the placeholder matrices.
%          rowpow, colpow = size(power_matrix); %we check the size of the working matrix
%          large_power_matrix = zeros(rowpow*(max_num_batches-first_batch), colpow);
%          large_power_matrix(1:rowpow,:) = power_matrix;
%     else
%          large_power_matrix(row_pow*batch_counter+1:2*row_pow)
%         
    
    large_power_matrix = [large_power_matrix; power_matrix];
    large_labels = [large_labels; labels];
    large_force = [large_force; force];
    if isempty(T_axis) %checks if it is the first iteration
        T_axis = T;%assigns the current time stamp
    else %if not, add the current time stamp plus the last element of the previous
        T_axis = [T_axis T_axis(end)+T];%<<<<<<CHECK THIS
    end
    batch_counter = batch_counter + 1;
end


%align the new label using the EMG
% onset = large_force; %get the EMG data
% EMG_Threshold = 1000;%Set a threshold for the EMG data for the Hemicraneoctomy paper data
% large_labels = onset_detection(abs(onset),'Teager',EMG_Threshold); %Generate the labels
% large_force = large_labels;

ntp_raw_data = size(raw_data,1);%Time points in the raw data vector
raw_time_axis = (1:ntp_raw_data)/samplingRate; %Get the new time axis based on the decimated sampling rate
fourier_sampling_rate = 1/diff(T(1:2));
[TF, match_idx] = findNearest(T_axis, raw_time_axis);
%large_labels = labels(match_idx,1);
%large_force = large_force(match_idx,1);
%Here we get the features to do the regression
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
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

%coeff_test = testing_hyp(b);%test hypotesis to check whcih values are non zero
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%
%Here we create some analysis plots that might be useful if the regression
%is working fine
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
plot_flag =0; %This flag controls the plots
if plot_flag ==1

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%Plotting Section
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    frequency_range = [12:40];
    analyse_all_channels

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    frequency_range = [72:82];
    analyse_all_channels
    
    
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%Plot full spectrograms aligend to raising edges per channel
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



toc
 
