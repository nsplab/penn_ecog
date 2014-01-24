%create set of plots for the powerpoint
desired_frequencies = [72:82];
plot_single_band
desired_frequencies = [12:42];
plot_single_band
frequency_range = [72:82];
analyse_single_trial
analyse_average_spectro
average_over_channels(large_power_matrix, large_labels, large_force, [3:11], frequency_range, F, fourier_sampling_rate, 'rise', 0)
plot_averaged_spectrogram_channels(large_power_matrix, large_labels, large_force, [3:11], F, frequency_range,'rise', fourier_sampling_rate, 0)
analyse_all_channels
frequency_range = [12:42];
analyse_single_trial
analyse_average_spectro
average_over_channels(large_power_matrix, large_labels, large_force, [3:11], frequency_range, F, fourier_sampling_rate, 'rise', 0)
plot_averaged_spectrogram_channels(large_power_matrix, large_labels, large_force, [3:11], F, frequency_range,'rise', fourier_sampling_rate, 0)
analyse_all_channels
