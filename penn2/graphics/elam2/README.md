## The output file format:
Nine files are generated at run time with names e1 to r9.

All times are in seconds, and all fields are single in C (4 bytes).

All the positions and distance are normalized to the target diameter.

- e1: [ **start time of trial** ] [ **trial number** ]
- e2: [ **time of dropping the ball in the target box** ] [ **accumulated time of holding the ball** ] [ **accumulated time of not holding the ball** ] [ **accumulated target engaged time** ] [ **accumulated time with ball - total engaged time** ] [ **number of drops outside target** ]  [ **opened by eye=1/0=eeg** ]
- e3: [ **times of target engaged onset** ] [ **target x** ] [ **target y** ] [ **target z** ] [ **trial number** ]
- e4: [ **times of target disengaged onset** ] [ **trial number** ]
- e5: [ **times of ball pickups** ] [ **ball x** ] [ **ball y** ] [ **ball z** ]
- e6: [ **times of ball drops** ] [ **ball x** ] [ **ball y** ] [ **ball z** ] [ **distance to target normalized by target diameter** ] [ **correct=1/incorrect=0** ] [ **trial number** ]
- e7: reserved
- e8: [ **times of waypoint engaged onsets** ] [ **waypoint numbers** ] [ **waypoint x** ] [ **waypoint y** ] [ **waypoint z** ] [ **trial number** ]
- e9: [ **times of waypoint disengaged onsets** ] [ **waypoint numbers** ] [ **waypoint x** ] [ **waypoint y** ] [ **waypoint z** ] [ **trial number** ]

### Matlab sample script to read the data:

```
fid = fopen('e1','r');
e1 = vec2mat(fread(fid, Inf, '*single'),2);
fclose(fid);

fid = fopen('e2','r');
e2 = vec2mat(fread(fid, Inf, '*single'),7);
fclose(fid);

plot([0 max(e2(:,1))],[0 0]);

scale = 10;

y = scale*ones(1,length(e1(:,1)));
stem(e1(:,1),y,'-r');

scale = 5;
y = scale*ones(1,length(e2(:,1)));
hold on;
stem(e2(:,1),y,'-b');

fid = fopen('e8','r');
e8 = vec2mat(fread(fid, Inf, '*single'),6);
fclose(fid);

scale = 3;
y = scale*ones(1,length(e8(:,1)));
hold on;
stem(e8(:,1),y,'-k');

scale = 3.2;
for r = 1:length(e8(:,1))
    text(double(e8(r,1)), scale, strtrim(cellstr(strtrim(num2str(e8(r,2))))));
end
```

