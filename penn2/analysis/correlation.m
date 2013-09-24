function [c] = correlation(a, b)
  a = a - mean(a);
  s = size(a);
  if (s(1) < s(2))
    a = a';
  end

  b = b - mean(b);
  s = size(b);
  if (s(1) < s(2))
    b = b';
  end

  c = (a' * b) / sqrt((a' * a) * (b' * b));
end
