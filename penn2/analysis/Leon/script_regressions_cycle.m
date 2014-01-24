%script to generate multiple analysis for teh regressions
%first with logscale
for i = 1000:1000:5000
    
    analyze_single_frequency_regression(large_power_matrix, large_labels, F, 3, fourier_sampling_rate, i, 0,0);
    analyze_single_frequency_regression(large_power_matrix, large_labels, F, 3, fourier_sampling_rate, i, 1,0);
    analyze_single_frequency_regression(large_power_matrix, large_labels, F, 3, fourier_sampling_rate, i, 0,1);
end