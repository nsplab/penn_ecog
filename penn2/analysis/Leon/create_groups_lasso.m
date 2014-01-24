function[group_name group] = create_groups_lasso(F_vect, group_perc)
%CREATE_GROUPS_LASSO generates the groups needed to run the grouped lasso
%implementation, it uses a percentaje of size of groups and a feature
%vector
%input:
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%F_vect: Features to be grouped, they have to be cattegorical features. E.G
%['length', 'space', 'height']
%group_perc: how de we look for the groups to be separeted (each group
%should have 

numb_groups = ceil(1/group_perc);
feat_per_group = floor(length(F_vect)*group_perc); %this is the number of features per group
%create queues to store the indexes of the columns, and its names to
%generate the cell
column_queue = q_fifo({});
feat_queue = q_fifo({});
for freq_idx = 1:length(F_vect)
    column_queue = q_fifo(column_queue, 'push', freq_idx);
    feat_queue = q_fifo(feat_queue, 'push', F_vect(freq_idx));
end
for group_idx = 1:numb_groups-1
    column_feat = [];%create placeholders for the columns
    name_feat = [];%create place holders for the names
    for g_idx = 1:feat_per_group
        [column_queue new_column_feat] = q_fifo(column_queue, 'get'); %we pull the values from the queue
        column_feat = [column_feat new_column_feat]; %append new columnd index
        [feat_queue feat_] = q_fifo(feat_queue, 'get');
        name_feat = [name_feat feat_];%append new name index
    end
    group_name{group_idx} = column_feat; 
    group{group_idx} = name_feat;
end
%allocate whatever is left in the buffer to the last group
[column_queue available_elem_buff] = q_fifo(column_queue, 'n');
column_feat = [];%create placeholders for the columns
name_feat = [];%create place holders for the names
for g_idx = 1:available_elem_buff
        [column_queue new_column_feat] = q_fifo(column_queue, 'get'); %we pull the values from the queue
        column_feat = [column_feat new_column_feat]; %append new columnd index
        [feat_queue feat_] = q_fifo(feat_queue, 'get');
        name_feat = [name_feat feat_];%append new name index
end
group_name{group_idx+1} = column_feat; 
group{group_idx+1} = name_feat;