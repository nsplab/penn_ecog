function [chan_data] = reference_strip(chan_data, reference)
%REFERENCE_STRIP re references each channel by substracting the mean of
%itws correspondent strip or by renormalizing using a baseline period from
%the beginning
%This is fixed for channels of size 60, if the number of channels changes
%this will break
%Each stip consists of groups of 16 channels
%Inputs:
%chan_data: Data from the channels
%reference = decission to reference using stip or the baseline (strip or
%baseline)
%outputs
%chan_data = re referenced channels

 switch reference
     case 'strip'
         %Take the mean of each strip
         for strip_idx = [1:15:60]
            mean_chan = mean(chan_data(:,strip_idx:strip_idx+14),2);
            chan_data(:,strip_idx:strip_idx+14) = chan_data(:,strip_idx:strip_idx+14)-repmat(mean_chan,1,15);
         end
     case 'baseline'
         chan_data = zscore(chan_data);
    
 end