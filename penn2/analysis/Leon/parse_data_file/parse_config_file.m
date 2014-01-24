function[field, value]  = parse_config_file(filename)
%PARSE_CONFIG_FILE takes the data configuration file created by Mosalam in
%the Penn experiments and extracts information regarding the experiment.
%input:
%--------------------------------------------------------
fileID=fopen(filename);
tline=fgets(fileID);
field={};
value={};
line_idx = 1;
while ischar(tline)
    [label remainder] = strtok(tline, ':'); %This takes out the characters ahead of the colon
    field{line_idx} = label;
    remain = strsplit(remainder);
    value{line_idx} = remain(2:end-1); %this is needed to take out the leading character, and the final trailing space
    line_idx = line_idx + 1;
    tline = fgets(fileID);
end
fclose(fileID);