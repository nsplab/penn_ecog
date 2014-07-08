function [] = test_jointrse()
  imitatorBaseline = 50; 
  imitatorAmplifier = 2; 

  filter = FilterJointRSE(1, 1);
  hand = 0;
  filter.setHandPos(hand);
  dimensions = 1;
  channels = 1;
  for i = 1:1000
    target = normrnd(0, 1);
    filter.target(1) = target;
    hand = filter.parameterValues{((dimensions + 1) * channels) + (1:dimensions)}
    obs = (target - hand) / 10;
    features = sqrt(obs + imitatorBaseline) * imitatorAmplifier;
    filter.RunFilter(features);
    filter.LogParameters('/home/bryanhe/penn_ecog/penn2/data', 'JointRSE');
    filter.currentTimeStamp = filter.currentTimeStamp + 1;
  end
end
