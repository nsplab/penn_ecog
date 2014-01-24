function [angle] = calculate_angle(xy_data, reference_axis)
%CALCULATE_ANGLE calculates the angle with respect of  a reference axis
%given in the input.
%inputs:
%--------------------------------------
%xyz_data : T x 2 array that has to contain a set of X, Y, Z
%reference_axis = axis to have as a reference to calculate the angle
y = xy_data(:,3-reference_axis);
x = xy_data(:,reference_axis);
angle = rad2deg(mod(atan2(y,x),2*pi));%calculate the angles in degrees from 0-360
end