function data_file = get_data_file(location)
%GET_DATA_FILE extracts the details in a directory and chooses the newest
%file that has the prefix data in it
location_search = [location '/data_*'];
d = dir(location_search); %get all the files that have the correct format
[x,y]=sort(datenum(char({d.date}))); %sort the files by order of creation
data_file=char(d(y(end)).name); %take the newest file
data_file = [location '/' data_file];
