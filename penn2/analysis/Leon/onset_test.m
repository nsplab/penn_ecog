%Script to analyze the effect of the threshold on the onset detection
onset = raw_data(:,71)- raw_data(:,70);%This are the EMG channels iand the difference is the real EMG
for onset_idx=100:10:3000
    onset_idx
    [labels] = onset_detection(abs(onset),'Teager',onset_idx);
    hold on
    plot(labels*2e5,'red')
    pause
    cla
end