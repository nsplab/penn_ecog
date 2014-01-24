function create_metadata(bands, fourier_sampling_rate, directory)
%CREATE_METADATA creates a metadata file called metadata.dta located in the
%specified folder.
%so we do not need to set them by hand in each individual file
%   Input:
%-------------------------------------------------------------------------
%   bands: a cell aray that contains the bands that we want to use to work
%   with
%   fourier_sampling_rate: is the sampling rate in the recorded files
%-------------------------------------------------------------------------
%%%



working_channels = get_variables('number_of_channels');
recorded_channels = get_variables('number_recorded_channels');
original_sampling_rate = get_variables('Original_Sampling_Rate');
sampling_rate = get_variables('Desired_Sampling_Rate');
date_str = get_variables('date_str');
filename = [directory '/metadata.nspd']
fileID = fopen(filename, 'w');
fprintf(fileID,'Meta information about the files contained in the folders\n');
fprintf(fileID,'%2.0f recorded channels\n',recorded_channels);
fprintf(fileID,'%2.0f working channels\n', working_channels);
fprintf(fileID,'Date of the experiment: %10s\n', date_str{1});
fprintf(fileID,'Original Sampling Rate: %6.2f\n', original_sampling_rate);
fprintf(fileID,'Sub Sampling Rate: %5.2f\n', sampling_rate);
fprintf(fileID,'Fourier Sampling Rate: %5.2f\n', fourier_sampling_rate);
fprintf(fileID,'First Channel: %.0f\n', get_variables('first_channel_number'));
for band_idx = 1:length(bands)
    fprintf(fileID,'Band %.0f: %5.2f to %5.2f\n', band_idx, bands{band_idx}(1),bands{band_idx}(end));
end

fprintf(fileID,'This data is property of the NSPLAB, for its use and distribution, contact leonpalafox@ucla.edu');

fclose(fileID)