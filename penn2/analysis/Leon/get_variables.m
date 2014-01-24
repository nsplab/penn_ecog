function [ variable ] = get_variables( variable_name, varargin )
%This function consolidates all the significant variables in a single file,
%using the varargin we control whatever variable we want from outside the
%file
%so we do not need to set them by hand in each individual file
%   Input:
%-------------------------------------------------------------------------
%   variable_name: name of the variable to get its value
%-------------------------------------------------------------------------
%  Output
%-------------------------------------------------------------------------
%  vairable: value of the variable
%%%
%Create the parser for the data, 
%first we specify the default values for our data
load('variables.mat')
p = inputParser;
default_number_of_channels = 60;
default_original_sampling_rate = 24414;
default_desired_sampling_rate = prime_desired_sampling_rate;
default_Window_Size = prime_window_size;
default_number_of_recorded_channels = 60;
default_overlap_percentage = prime_overlap_percentage;
default_Reference_channel = 2;
default_right_limit = 1;
default_left_limit = 1;
default_beta = [12:30];
default_high_gamma = [65:115];
default_Time_stamp_Bytes = 8;
default_Kinect_Sampling_rate = 28;
addOptional(p,'Desired_Sampling_Rate',default_desired_sampling_rate,@isnumeric);
addOptional(p,'Window_Size',default_Window_Size,@isnumeric);
addOptional(p,'overlap_percentage',default_overlap_percentage,@isnumeric);
parse(p,varargin{:})


switch variable_name
    case 'number_of_channels'
        %desired number of channels to work with
        variable = 10;
        %variable = number_of_channels_prime;
    case 'Original_Sampling_Rate'
        variable = 24414;
    case 'Desired_Sampling_Rate'
        variable = p.Results.Desired_Sampling_Rate;
    case 'Window_Size'
        variable = p.Results.Window_Size;
    case 'number_recorded_channels'
        %number of channels recorded
        %variable = 60;
        variable = number_of_channels_prime;
    case 'overlap_percentage'
        variable = p.Results.overlap_percentage;
    case 'Reference_Channel'
        variable = 2; %use channel 1 as reference, it can be changed to any channel 
    case 'right_limit'
        variable = 1; %limit of the right side in seconds
    case 'left_limit'
        variable = 1; %limit of the left side in seconds
    case 'beta'
        variable = [12:30];
    case 'High Gamma'
        variable = [65:115];
    case 'Time_stamp_bytes'
        variable = 8;
        %variable = 0;
    case 'Kinect_Sampling_Rate'
        variable = 28;
    case 'date_str'
        variable = date_str;
    case 'first_channel_number'
        variable = first_channel_prime;
    otherwise
        error('There is no such variable to query')
   
        
        
end



end

