for p_index = 0.01:0.01:0.05
    analyze_p_value_consistency(large_power_matrix, large_labels, F, 1, p_index,fourier_sampling_rate, 300, 1, 0)
    close all
end

