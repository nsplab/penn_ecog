%Test Andrew's code to take out noise in the data using the multitaper
%method.
%Generate a composite signal with 3 known frequencies:
sr = 1024; %Sampling rate of 1024 Khz
total_time = 10;%Generate 20 seconds of data
ntp = total_time * sr;
t = [1:ntp]/sr;
test_signal = sin(60*2*pi*t) + sin(30*2*pi*t) + sin(62*2*pi*t);
[S,F,T,P] = spectrogram(test_signal, hann(500), 10, 500, sr);
surf(T,F,log10(P),'edgecolor','none')
axis tight;
view(0,90)
pause
noisy_signal = mtmlinenoise(test_signal,4,100,sr,[62]);
[S,F,T,Pp] = spectrogram(noisy_signal, hann(500), 10, 500, sr);
surf(T,F,log10(Pp),'edgecolor','none')
axis tight;
view(0,90)
pause
clean_signal = test_signal - noisy_signal';
[S,F,T,Pc] = spectrogram(clean_signal, hann(500), 10, 500, sr);
surf(T,F,(Pc),'edgecolor','none')
axis tight;
view(0,90)
