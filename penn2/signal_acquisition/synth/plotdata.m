%%%%%
%% prepare matlab
%%%%%
set(gcf,'Visible','off');
set(0,'defaultAxesFontName', 'Arial')
set(0,'defaultaxesfontSize',8)
set(gcf,'color','w');

% load data file
fid = fopen('sdata','r');
fseek(fid, 0, 'bof');
raw_data = fread(fid, Inf, '*single');
mdata = vec2mat(raw_data,20);
fclose(fid);

subplot(2,1,1);
plot(mdata(:,1));
subplot(2,1,2);
plot(mdata(:,6));

myaa([4 2],'forcedata.png');
