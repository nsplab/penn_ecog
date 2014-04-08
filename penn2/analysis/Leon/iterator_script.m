%This script iterates over different window_sizes and overlap percentages
data_cell = {'/home/leon/Data/Penn/Nov_18/data_Mon_18.11.2013_12:00:33', '/home/leon/Data/Penn/Nov_21/data_Thu_21.11.2013_12:32:02', '/home/leon/Data/Penn/Nov_26/data_Tue_26.11.2013_13:36:24'};
data_directory = '../../graphics/squeeze/build';
data_file = get_data_file(data_directory);
for overlap_idx = 0.5
    for window_idx = 0.5
        
        keep overlap_idx window_idx data_file
        metadata = '../../launcher/log.txt'%Mosalam's config file location
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %%%%%%%%%%%%%%Get the metadata of the file%%%%%%%%%%%%%%%%%%%%%%%%%%
        %the metadata is usually specified ahead of the call of analyse_data
        %the metadata file has a simple formatting of 2 columns separated by a
        %space
        %The element that contains the date is the second element in the second
        %cell
        [date_str, first_channel_number, number_of_channels] = extract_info(metadata);
        save_variables(600, window_idx, overlap_idx, date_str, first_channel_number, number_of_channels)
        analyse_data
        %analyze_p_value_consistency(large_power_matrix, large_labels, F, 3, 0.01,fourier_sampling_rate, 300, 1, 0)
        %tempo_iter_p_values
        frequency_range = [12:32];
        analyse_all_channels
        plot_deviance_topological(large_power_matrix, large_labels, F, frequency_range, 'single', fourier_sampling_rate, 1,0)
        frequency_range = [72:92];
        analyse_all_channels
        plot_deviance_topological(large_power_matrix, large_labels, F, frequency_range, 'single', fourier_sampling_rate, 1,0)
        %plot_averaged_spectrogram_channels( large_power_matrix, large_labels, large_force, [3:11], F, frequency_range, 'rise', fourier_sampling_rate, 0)
        %average_over_channels( large_power_matrix, large_labels, large_force, [3], frequency_range, F, fourier_sampling_rate ,'rise', 0)
        %frequency_range = [12:42];
        %plot_averaged_spectrogram_channels( large_power_matrix, large_labels, large_force, [3:11], F, frequency_range, 'rise', fourier_sampling_rate, 0)
        %average_over_channels( large_power_matrix, large_labels, large_force, [3], frequency_range, F, fourier_sampling_rate ,'rise', 0)
        
    end
    
end
