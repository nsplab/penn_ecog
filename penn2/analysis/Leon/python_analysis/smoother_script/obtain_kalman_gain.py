#Code to obtain the kalman gain from a string of observations
#By Leon Palafox
#Example
#Given a FILENAME located in the same directory.
#The Filename needs to be a one column csv file.
#>>python obtain_kalman_gain.py
#After finishing, the script generates a file named kalman_gain.nspd
#kalman_gain.nsp is a one value file that has the kalman gain to smooth 
#equations of the form:
#y_{k+1|k+1}=y_{k|k}(1-KalmanGain)+y_{k+1}(KalmanGain)


#Define all the needed libraries
import pylab as plt
import numpy as np #Python module for math functions
import sys #Basic System functions for Python
import var_file as vf #Define variables in the program
import KalmanModule as kl #Module that implements the kalman filter
data_file = '../../../../feature_extraction/feature_extract_cpp/build/datalog.txt' #<--- add the name of the files
dataset = np.loadtxt(data_file,delimiter=',',dtype=float)
dataset = dataset[:, None]
#dataset = ((dataset-dataset.mean(0))/dataset.std(0))+5.0
#######Data Format######
time, dims = dataset.shape
##################################################################
####Define basic parameters for the Kalman FIlter###########################
n_states = vf.get_param_value('EM_latent_variables')
em_iter = vf.get_param_value('EM_Iterations')
Kfilt = kl.KalmanFilter()
Kfilt.n_dim_state=n_states #Numb of states
Kfilt.n_dim_obs=dims #Numb of dimensions
Kfilt.observation_matrices = np.ones((dims,n_states))
Kfilt.observation_offsets = np.zeros((dims))#affine observation term
Kfilt.transition_matrices = np.ones((n_states,n_states))
Kfilt.transition_offsets = np.zeros((n_states))#aqffine transition term
#########################################################
#Initialize the training matrices
variables_to_train = ['transition_covariance', 'observation_covariance'] #define which variables to train, the rest are left untouched
Kfilt.em(dataset[1:dataset.shape[0]/2,:],em_vars = variables_to_train, n_iter=em_iter)
#train EM only with half of the data
filtered_state_means = np.zeros((time, n_states))
filtered_state_covariances = np.zeros((time, n_states, n_states))
predicted_observation = np.zeros((time, dims))
predicted_observation_covariance = np.zeros((time, dims, dims))
kalman_gain = np.zeros((time, n_states, dims))
print 'Start Filtering'
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
p1, = plt.plot(predicted_observation)
p2, = plt.plot(dataset)
plt.legend([p1, p2], ["Predicted Observations", "Real Observations"])
plt.title('Normalized observations of the Beta Band, Channel 67')
plt.xlabel('Samples')
plt.ylabel('Normalized Magnitude')
np.savetxt('kalman_gain.nspd', kalman_gain[-1]) #saves the last element of the Kalman Gain, which is the Steady state Kalman Gain


