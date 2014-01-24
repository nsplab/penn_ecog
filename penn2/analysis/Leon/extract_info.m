function [date_str, first_channel, number_of_channels] = extract_info(data_file)
%EXTRACT_INFO is a function that given a data file that has a specific
%formatting, it extracts the correspondent fields.


%first, we need to extract fields and values
[fields, values] = parse_config_file(data_file);
%Extract the date string
[truefalse, index] = ismember('Time', fields);
date_str = values{index}(1);
%Extract the first ecog channel
[truefalse, index] = ismember('First ECoG Channel', fields);
first_channel = str2num(values{index}{1});
%Extract the number of channels we are working with
[truefalse, index] = ismember('Number of ECoG Channels', fields);
number_of_channels = str2num(values{index}{1});


