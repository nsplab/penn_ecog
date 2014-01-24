function [batch_data] = return_batch_kinect( data_file)
%RETURN_BATCH_KINECT return batches of the data for large datafiles associated with the kinect.
%The data files are T X N, where T is the time and N is the different
%features
%input:
%data_file = loation of the data file to segment
%size_of_batch =  size of the desired batch in seconds
%samplingRate = sampling rate of the original non decimated signal
%batch number = decide which batch of data do you wish to extract
%if the batch number is inconsistent with the size of the batch, and error
%will be created.
%if the batch is the last one, the function will return the rest available
%data, whcih won't have the requested size because is what is left in the
%file.
%Before using the function we recomend to calculate the number of batches
%first, although if there is a mismatch, the function will indicate which is
%this.
%The dta format for the kinect is:
% - 8 bytes, the time stamp, which tells the corresponding row number in
% the first file. So if it's 100, it means this row corresponds to the
% row number 100 in the ECoG data file.
% - x (4 bytes)
% - y (4 bytes)
% - z (4 bytes)
% - x' (4 bytes)
% - y' (4 bytes)
% - z' (4 bytes)
%x,y,z have the unit of meters. x',y',z' are scaled values in the
%virtual environment.
time_stamp_bytes = 8;     
num_4_byte_column = 6;
dinfo = dir(data_file); %get the information of the data file
fid = fopen(data_file,'r');
%first we read 
fseek(fid, (time_stamp_bytes), 'bof');%the second parameter of fseek indicates where we start reading
v1 = fread(fid, Inf, 'float', 4*(num_4_byte_column-1)+time_stamp_bytes);
velocity_data = zeros(size(v1,1), 6);
velocity_data(:,1) = v1;
clear v1; %remove ch1 from the workspace to free up memory
%now loop through all the channels to populate the data file
for vidx=2:6
    channel_offset = (vidx-1); %channel ofsset is the offset to move in the binary file
    fseek(fid, (time_stamp_bytes+channel_offset*4), 'bof');%fseek moves the pointer over the binary file
    velocity_data (:,vidx) = fread(fid, Inf, 'float', 4*(num_4_byte_column-1)+time_stamp_bytes);%copy everything to the pre created array
end
batch_data = velocity_data;

fclose(fid);

end

