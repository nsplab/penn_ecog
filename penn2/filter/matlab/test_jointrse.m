function [] = test_jointrse()
  imitatorBaseline = 50; 
  imitatorAmplifier = 2; 

  dimensions = 3;
  channels = dimensions;

  filter = FilterJointRSE(dimensions, channels);
  hand = zeros(dimensions, 1);
  filter.setHandPos(hand);
  for i = 1:1000
    target = normrnd(0, 1, dimensions, 1);
    filter.target(1:dimensions) = target;
    hand = [filter.parameterValues{((dimensions + 1) * channels) + (1:dimensions)}]'
    obs = (target - hand) / 10;
    features = sqrt(obs + imitatorBaseline) * imitatorAmplifier;
    filter.RunFilter(features);
    filter.LogParameters('/home/bryanhe/penn_ecog/penn2/data', 'JointRSE');
    filter.currentTimeStamp = filter.currentTimeStamp + 1;
  end
end

