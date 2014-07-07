function [] = test_jointrse()
  imitatorBaseline = 50; 
  imitatorAmplifier = 2; 

  filter = FilterJointRSE(1, 1);
  hand = 0;
  filter.setHandPos(hand);
  for i = 1:100
    target = normrnd(0, 1);
    filter.target(1) = target;
    obs = (target - hand) / 10;
    features = sqrt(obs + imitatorBaseline) * imitatorAmplifier;
    filter.RunFilter(features);
  end
end

