%normalize the large_plot_matrix
mean_lp = mean(large_power_matrix,1);
std_lp = std(large_power_matrix);
large_power_matrix_z = bsxfun(@minus, large_power_matrix, mean_lp);
large_power_matrix_z = bsxfun(@rdivide, large_power_matrix_z, std_lp);
