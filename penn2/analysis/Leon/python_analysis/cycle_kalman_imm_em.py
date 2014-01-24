#This code implements a working example using a test dataset
import numpy as np #Python module for math functions
import pylab as plt #Python module for plotting
import sys #Basic System functions for Python
import os
import GlobalEM as gem
import var_file as vf
from sklearn.preprocessing import MinMaxScaler
import KalmanModule as kl
fourier_sampling_rate = 3.3352;
folder_name = 'EEGDatasets'
band_number = len([name for name in os.listdir('./'+folder_name) if os.path.isdir('./'+folder_name+'/'+name)])
for band_idx in range(1,band_number+1):
    folder = os.path.join(folder_name, 'band_' + str(band_idx))
    channel_number = len([name for name in os.listdir('./'+folder) if os.path.isfile('./'+folder+'/'+name)])
    for chan_idx in range(1, channel_number+1):
        data_file = folder+'/'+'channel_'+str(chan_idx)+'.nspd'
        dataset = np.loadtxt(data_file,delimiter=',',dtype=float)
        labels = dataset[:, 1]
        dataset = dataset[:, 0:1]
        dataset = ((dataset-dataset.mean(0))/dataset.std(0))
        #convert the dataset in [time, dims, trials]
        n = dataset.shape[0]/39; #time points per trial, this is hardcoded
        aligned_dataset=np.array([dataset[i:i+n] for i in range(0, len(dataset), n) ])[:,:,0]
        aligned_dataset=np.reshape(aligned_dataset.T, (n,1,39))
        #######Data Format######
        time, dims, trials = aligned_dataset.shape
        time_raw, dims = dataset.shape
        ##################################################################
        ########Preprocessing of the data################################
        time_axis = np.linspace(0,time_raw/fourier_sampling_rate,time_raw)

        ####We also define the number of filters###########################
        n_states = vf.get_param_value('EM_latent_variables')
        em_iter = vf.get_param_value('EM_Iterations')
        Kfilt = kl.KalmanFilter()
        Kfilt.n_dim_state=n_states #Numb of states
        Kfilt.n_dim_obs=dims #Numb of dimensions
        Kfilt.observation_matrices = np.ones((dims,n_states))
        #Initialize the training matrices
        filtered_state_means = np.zeros((time_raw, n_states))
        filtered_state_covariances = np.zeros((time_raw, n_states, n_states))
        predicted_observation = np.zeros((time_raw, dims))
        predicted_observation_covariance = np.zeros((time_raw, dims, dims))
        Kfilt.em(aligned_dataset[:,:,0],em_vars='all', n_iter=1)
        for idx in range(em_iter):#run em 
                MainStruct=gem.GlobalVariables_Modified(aligned_dataset,Kfilt)
                obs_matrices,obs_covariance,trans_matrix,trans_cov,init_st_mean,init_st_cov,trans_off,obs_off=gem._em_main_loop(Kfilt,MainStruct,aligned_dataset)
                Kfilt.observation_matrices=obs_matrices
                Kfilt.transition_matrices=trans_matrix
                Kfilt.transition_offsets=trans_off
                Kfilt.observation_offsets=obs_off
                Kfilt.transition_covariance=trans_cov
                Kfilt.observation_covariance=obs_covariance
                #print Kfilt.observation_matrices
                #print Kfilt.observation_offsets
                Kfilt.initial_state_mean=init_st_mean
                Kfilt.initial_state_covariance=init_st_cov



        for t in range(time_raw-1):
            if t == 0:
                filtered_state_means[t] = np.zeros((1,n_states))
                filtered_state_covariances[t] = np.eye(n_states)
            predicted_observation[t+1], predicted_observation_covariance[t+1], filtered_state_means[t + 1], filtered_state_covariances[t + 1], _ = (
                Kfilt.filter_update(
                    filtered_state_means[t],
                    filtered_state_covariances[t],
                    dataset[t + 1],
                )
            )
        trials =39;
        n = dataset.shape[0]/39; #time points per trial, this is hardcoded

        #aligned_dataset=np.array([dataset[i:i+n] for i in range(0, len(dataset), n) ])[:,:,0]
        plt.figure()
        cov_vector = gem._smoothed_state_covariance(gem._smoothed_state_mean(MainStruct), MainStruct)
        mean_vect = gem._smoothed_state_mean(MainStruct)
        upper_CI = mean_vect-1.96*np.sqrt(np.abs(cov_vector))/np.sqrt(39)
        lower_CI = mean_vect+1.96*np.sqrt(np.abs(cov_vector))/np.sqrt(39)
        time_axis = np.linspace(-1,1,n)
        p1, = plt.plot(time_axis, mean_vect, 'k', linewidth=2.0)
        plt.fill_between(time_axis,upper_CI[:,0], lower_CI[:,0], alpha = 0.5, color = '0.8')
        p2, = plt.plot(time_axis, lower_CI, 'b--', linewidth=1.0)
        p3, = plt.plot(time_axis, upper_CI, 'r--', linewidth=1.0)
        plt.xlabel('Time[s]')
        plt.ylabel('Power Means')
        plot_title = 'State Space model using EM, Band: '+str(band_idx)
        plt.title(plot_title)
        plt.legend([p1, p2, p3], ["Power Estimate Mean", "Upper 95% CI", "Lower 95% CI"])
        plt.grid()
        plt.savefig('./figures'+'/band'+str(band_idx)+'channel'+str(chan_idx+64)+'.png')
        
        
        
