imitator=load('imitator.mat')
filter=load('filter.mat')

figure
hold on
scatter(1:numel(imitator.Imitator_pos), imitator.Imitator_pos)
scatter(1:numel(filter.filter_position), filter.filter_position)
