in = load('/home/user/code/penn2/penn/penn2/filter/cpp/build/innovation.txt');
h = plot(sqrt(decimate(sum(in.^2,2),300)));
%h = plot(abs(decimate(in(:,4),300))+abs(decimate(in(:,5),300))+abs(decimate(in(:,6),300)));
xlabel('Trial');
ylabel('|innovation|_2');
saveas(h,'innovation','png');

cc=lines(6);
h2 = figure; 
hold on;
for i=1:6
    plot(decimate(in(:,i),300),'color',cc(i,:));
    legstr{i}=['param' int2str(i)];
end
legend(legstr);

xlabel('Trial');
ylabel('innovation');

saveas(h2,'innovation2','png');
