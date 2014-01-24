#This code implements a working example using a test dataset
import numpy as np #Python module for math functions
import pylab as plt #Python module for plotting
import sys #Basic System functions for Python
import var_file as vf
from sklearn.preprocessing import MinMaxScaler
import KalmanModule as kl
fourier_sampling_rate = 3.3352;
data_file = 'test.nspd'
time_series = 'time.nspd'
dataset = np.loadtxt(data_file,delimiter=',',dtype=float)
#labels = dataset[:, 1]
dataset = dataset[:, None]
#dataset = ((dataset-dataset.mean(0))/dataset.std(0))+5.0
#######Data Format######
time, dims = dataset.shape
#convert the dataset in [time, dims, trials]
n = dataset.shape[0]/39; #time points per trial, this is hardcoded
aligned_dataset=np.array([dataset[i:i+n] for i in range(0, len(dataset), n) ])
aligned_dataset=np.reshape(aligned_dataset, (39,1,n))
##################################################################
########Preprocessing of the data################################
time_axis = np.linspace(0,time/fourier_sampling_rate,time)
#time, dims, trials = aligned_dataset.shape
####We also define the number of filters###########################
n_states = vf.get_param_value('EM_latent_variables')
em_iter = vf.get_param_value('EM_Iterations')
Kfilt = kl.KalmanFilter()
Kfilt.n_dim_state=n_states #Numb of states
Kfilt.n_dim_obs=dims #Numb of dimensions
Kfilt.observation_matrices = np.ones((dims,n_states))
Kfilt.observation_offsets = np.zeros((dims))
Kfilt.transition_matrices = np.ones((n_states,n_states))
Kfilt.transition_offsets = np.zeros((n_states))
#Initialize the training matrices
variables_to_train = ['transition_covariance', 'observation_covariance']
Kfilt.em(dataset[1:dataset.shape[0]/2,:],em_vars = variables_to_train, n_iter=em_iter)
filtered_state_means = np.zeros((time, n_states))
filtered_state_covariances = np.zeros((time, n_states, n_states))
predicted_observation = np.zeros((time, dims))
predicted_observation_covariance = np.zeros((time, dims, dims))
kalman_gain = np.zeros((time, n_states, dims))
print 'Start Filtering'
raw_input("Press Enter to continue...")
for t in range(time-1):
    #raw_input("Press Enter to continue...")
    if t == 0:
        filtered_state_means[t] = np.zeros((1,n_states))
        filtered_state_covariances[t] = np.eye(n_states)
    predicted_observation[t+1], predicted_observation_covariance[t+1], filtered_state_means[t + 1], filtered_state_covariances[t + 1], kalman_gain[t+1] = (
        Kfilt.filter_update(
            filtered_state_means[t],
            filtered_state_covariances[t],
            dataset[t + 1],
        )
    )
plt.figure()
p1, = plt.plot(time_axis, predicted_observation)
p2, = plt.plot(time_axis, dataset)
#p3, = plt.plot(time_axis, labels*np.max(dataset))
plt.legend([p1, p2, p3], ["Predicted Observations", "Real Observations", "Labels"])
plt.title('Normalized observations of the Beta Band, Channel 67')
plt.xlabel('Time[s]')
plt.ylabel('Normalized Magnitude')

plt.figure()
sys.exit()
trials =39;
n = dataset.shape[0]/39; #time points per trial, this is hardcoded

aligned_dataset=np.array([dataset[i:i+n] for i in range(0, len(dataset), n) ])[:,:,0]
aligned_predictions=np.array([predicted_observation[i:i+n] for i in range(0, len(predicted_observation), n) ])[:,:,0]
plt.plot(aligned_predictions.T, '0.8')
plt.plot(np.mean(aligned_predictions.T,1), 'k')
data_to_save = np.hstack((predicted_observation, predicted_observation_covariance[:,0:1,0]))
np.savetxt('predicted.nspd', data_to_save, delimiter = ',')
np.savetxt('kalman_gain.nspd', kalman_gain[-1]) #saves the last element of the Kalman Gain


