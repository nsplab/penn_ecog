%This script loads the data from the PennPrject dataset into the matlab
%environment
%clear
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Set the critical variables
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
offset_between_kinect_ecog = 70;%offset between kinect and ecog is 70 seconds
start_time = 100;%time to start doing the analysis ins econds
num_chan = get_variables('number_of_channels');
num_record_chan =  get_variables('number_recorded_channels');
originalSamplingRate = get_variables('Original_Sampling_Rate');
samplingRateKinect = get_variables('Kinect_Sampling_Rate');
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
first_batch = 1;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Read the data files
%root_path = ['/home/leon/Data/Penn/Nov_12'];
%time_stamps_file = [root_path '/data_click_Tue_01.10.2013_10:12:28'];
%data_file = [root_path '/data'];
data_file = '/home/leon/Data/Penn/Nov_26/Kinect/data_Tue_26.11.2013_14:07:24';
kinect_data_file = '/home/leon/Data/Penn/Nov_26/Kinect/hand_dataTue_26.11.2013_14:08:34';
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
%max_num_batches = 10;
%finsih reading data files
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Create place holders where 


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
tic

large_power_matrix = [];
T_axis=[];
%Start the batched analysis
for batch_idx = first_batch:max_num_batches
    batch_idx
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    channels_data = return_batch(data_file, size_of_batch, originalSamplingRate, batch_idx, 'eeg');
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %The following code denoises the singla using Andrew's technique.
    %for ch_idx = 1:num_chan
    %    denoised_channel = mtmlinenoise(channels_data(:,ch_idx),2.5,window_size, originalSamplingRate, [62]);
    %    channels_data(:,ch_idx) = channels_data(:,ch_idx) - denoised_channel;
    %end

    %pre process data files (trim, decimate)
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    x=decimate(channels_data(:,1), decimate_factor);%set a decimation on the first channel to get the value of the matrix
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
    %the next part of code is to generate placeholders to prevent overflows
    %in windows computers
     
    
    large_power_matrix = [large_power_matrix; power_matrix];
    if isempty(T_axis) %checks if it is the first iteration
        T_axis = T;%assigns the current time stamp
    else %if not, add the current time stamp plus the last element of the previous
        T_axis = [T_axis T_axis(end)+T];%<<<<<<CHECK THIS
    end
end


%force = force(match_idx,1);%recapture the force using the matched indexes
[kinect_data] = return_batch_kinect(kinect_data_file);
time_axis_kinect = (1:size(kinect_data,1))/samplingRateKinect + offset_between_kinect_ecog;%generate the time series of the force
[TF, match_idx] = findNearest(T_axis,time_axis_kinect); %is member looks up which indexes match the output from the 
kinect_data = kinect_data(match_idx,:);
[c start_index] = min(abs(T_axis-start_time));%we get the index of the start time
%now we cut every dataset to start at the same time
T_axis = T_axis(1,start_index:end);
large_power_matrix = large_power_matrix(start_index:end,:);
kinect_data = kinect_data(start_index:end,:);
kinect_data = smooth_features(kinect_data(:,1:end), 100);
fourier_sampling_rate = 1/diff(T(1:2));
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%align the new label using the EMG
angle_x = calculate_angle(kinect_data,1);

%Here we get the features to do the regression
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


toc
 
