function smoothed_features = smooth_features(feature_vector, smooth_window)
%SMOOTHED_FEATURES
%input
%----------------------------------
%feature_vector: vector to smooth
%smooth_window: window to do the smoothing
%---------------------------------------
%output:
%------------------------------------
%smoothed_features: smoothed vector
%
[cols, rows] = size(feature_vector);
smoothed_features = zeros(cols,rows);
size(smoothed_features)
for rowidx = 1:rows
    smoothed_features(:,rowidx) = smooth(feature_vector(:, rowidx), smooth_window);
end

