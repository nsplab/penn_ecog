function [] = plotresults(parSetColor, trialZeroValue)
%plot the results of the analysis

%load the results of the analysis
load('tmp/results.mat', 'Responses','MaxResponses','samples','BackgroundProb');

%plot Pr(correct response) 
pdata =[];
for t = 1:length(Responses) 
        allsamples   = [samples.p(1,:,t) samples.p(2,:,t) samples.p(3,:,t)];
        sort_samples = sort(allsamples);
        total        = length(sort_samples);
        ll           = sort_samples(fix(0.05*total));  %lower 95%interval
        ml           = sort_samples(fix(0.5*total));
        ul           = sort_samples(fix(0.95*total));
        pdata = [pdata; t ll ml ul];
end

plotI(Responses, MaxResponses); hold on;
title('Success Rate vs Test Trial Number');
xlabel('Test Trial Number');
ylabel('Success Rate');

axis([-0.2 10.2 -0.05 1]);

successLowerBounds = [trialZeroValue(1); pdata(:, 2)];
successMeans = [trialZeroValue(2); pdata(:, 3)];
successUpperBounds = [trialZeroValue(3); pdata(:, 4)];

plotBeautifulData(successMeans, successLowerBounds, successUpperBounds, ...
    parSetColor, 'Best', 0);

drawTrainingRectangles(1.05, 'yes');
end
