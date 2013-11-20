function [edge_idx] = get_indeces(labels, edges)
%GET_INDECES gets the indexes of the rising and falling edges
%%input:
%labels: sets of 1s and 0s to align the data with
%data: original data, which is already aligned, so it shares the same
%indexes
%output:
%It generates a plot of the aligned data.
%data_array: Is a TimexFeatxTrials matrix
%Set Parameters
switch edges
    case 'rise'
        edge_idx = find(diff(labels)>0);%find the uprising edges indexes
    case 'fall' 
        edge_idx = find(diff(labels)<0);%find the falling edges indexes
        
end

end