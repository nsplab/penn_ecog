function [edge_idx] = get_indeces(labels, edges)
%GET_INDECES gets the indexes of the rising and falling edges
%%input:
%labels: sets of 1s and 0s to align the data with
%edges: A vairable that is either 'raise' or 'fall' that controls whether
%we are looking for rising edges or falling edges.
%indexes
%output:
%It generates a plot of the aligned data.
%edge_idx: Has the indexes for the desired events
%Set Parameters
switch edges
    case 'rise'
        edge_idx = find(diff(labels)>0);%find the uprising edges indexes
    case 'fall' 
        edge_idx = find(diff(labels)<0);%find the falling edges indexes
        
end

end