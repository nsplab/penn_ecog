clear;
close all;

load('emg_freq');
load('predictor');
load('signal');

%% Find center of squeezes
signal(signal < 500) = 0; % cut noise

% Fill in gaps
gap_size = 10;
prev = 0;
count = 0;
for i = 1:size(signal, 1)
  if (signal(i) ~= 0)
    if (i - prev <= gap_size)
      index = 1:(i-prev-1);
      count = count + 1;
      signal(prev + index) = ((i-prev-index) * signal(prev) + index * signal(i)) / 2 / sum(index);
      %if (i - prev > 1)
      %  ((i-prev-index) * signal(prev) + index * signal(i)) / 2 / sum(index)
      %  %for j = prev+1:i-1
      %  %  signal(j) = ((j - prev) * signal(prev) + (i - j) * signal(i)) / (i - prev);
      %  %end
      %end
    end
    prev = i;
  end
end
count;

% Index squeezes
squeeze = zeros(1, size(signal, 1));
continuation = false;
index = 0;
for i = 1:size(signal, 1)
  if (signal(i) ~= 0)
    if (~continuation)
      index = index + 1;
    end
    continuation = true;
    squeeze(i) = index;
  else
    continuation = false;
  end
end

figure(1);
hold on;

% Set limits
ax = [1 size(signal, 1) min(signal(:, 1)) max(signal(:, 1))];
axis(ax);

% Boxes around squeezes
yl = ylim;
y = yl(1);
h = yl(2) - yl(1);
for i = 1:index
  x = find(squeeze == i);
  w = x(end) - x(1);
  x = x(1);
  if (w == 0)
    w = 1;
  end
  %rectangle('Position',[x,y,w,h]);
  fill([x x x+w x+w], [y y+h y+h y], [0.5 0.5 0.5])
end

% Remove small squeezes (noise)
minimum_width = 50;
for i = 1:index
  x = sum(squeeze == i);
  if (x < minimum_width)
    signal(squeeze == i) = 0;
    squeeze(squeeze == i) = 0;
  end
end

% Remove large squeezes (merged squeezes)
maximum_width = 200;
for i = 1:index
  x = sum(squeeze == i);
  if (x > maximum_width)
    signal(squeeze == i) = 0;
    squeeze(squeeze == i) = 0;
  end
end

% Boxes around remaining squeezes
yl = ylim;
y = yl(1);
h = yl(2) - yl(1);
for i = 1:index
  x = find(squeeze == i);
  if numel(x) ~= 0
    w = x(end) - x(1);
    x = x(1);
    if (w == 0)
      w = 1;
    end
    %rectangle('Position',[x,y,w,h]);
    fill([x x x+w x+w], [y y+h y+h y], [1 0.0 0.0])
  end
end

% Plot signal (to move on top of boxes)
scatter(1:size(signal, 1), signal(:, 1));

% Set limits
axis(ax);


%% Aggregate EEG data
interval = -500:500;
%data = zeros(size(predictor, 1)-1, numel(interval)); % -1 to remove constant
data = zeros(10, numel(interval)); % -1 to remove constant
count = 0;
for i = 1:index
  x = find(squeeze == i);
  if numel(x) ~= 0
    count = count + 1;

    onset = x(1);
    offset = x(end);

    data = data + predictor(7:16, onset + interval);

  end
end

count

figure(2);
image(interval, 6:15 / 512 * 1024, data, 'CDataMapping', 'scaled');

colorbar;
