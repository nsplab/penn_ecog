#This module takes charge of learning the parameters for multiple training instances
import warnings

import numpy as np
from scipy import linalg

def _em_main_loop(filt,MainStruct_reduced,Full_Data):
        C=_em_observation_matrix(Full_Data,filt.observation_offsets, MainStruct_reduced)#outputs the observation matrix
        D=_em_observation_covariance(Full_Data, filt.observation_offsets,C, MainStruct_reduced)
        E=_em_transition_matrix(filt.transition_offsets,MainStruct_reduced) #Transition Matrix
        F=_em_transition_covariance(E,filt.transition_offsets,MainStruct_reduced)#Transition Covariance
        G=_em_initial_state_mean(MainStruct_reduced)#Initial state means
        H=_em_initial_state_covariance(G,MainStruct_reduced)#initial state covariances
        I=_em_transition_offset(E,MainStruct_reduced)#Transition offset
        J=_em_observation_offset(C,MainStruct_reduced,Full_Data)#Observation Offset
        return C,D,E,F,G,H,I,J

def GlobalVariables(*args):
        data=args[0]
        filters=args[1:]
        nstates=filters[0].n_dim_state
        NoFilt,time,dims,trainN=np.shape(data)
        Master_Smooth_states=np.zeros((len(filters),trainN,time,nstates))
        Master_smooth_covs=np.zeros((len(filters),trainN,time,nstates,nstates))
        fil=0
        #geting the smoothings
        MainStruct={}
        for filt in filters:
                MainStruct[fil]={}
                for i in range(trainN):
                        #A=filt.smooth(data[fil,:,:,i])
                        MainStruct[fil][i]={}
                        (predicted_state_means, predicted_state_covariances,kalman_gains, filtered_state_means,filtered_state_covariances)=(_filter(filt.transition_matrices,filt.observation_matrices,filt.transition_covariance,filt.observation_covariance,filt.transition_offsets,filt.observation_offsets,filt.initial_state_mean,filt.initial_state_covariance,data[fil,:,:,i]))
                        (smoothed_state_means, smoothed_state_covariances,kalman_smoothing_gains)=(_smooth(filt.transition_matrices,filtered_state_means,filtered_state_covariances,predicted_state_means,predicted_state_covariances))
                        #print smoothed_state_means
                        pairwise_covariances = _smooth_pair(smoothed_state_covariances,kalman_smoothing_gains)
                        #Master_Smooth_states[fil,i,:,:]=A[0]
                        #print sigma_pair_smooth
                        MainStruct[fil][i]['predicted_state_means']=predicted_state_means
                        MainStruct[fil][i]['predicted_state_covariances']=predicted_state_covariances
                        MainStruct[fil][i]['kalman_gains']=kalman_gains
                        MainStruct[fil][i]['filtered_state_means']=filtered_state_means
                        MainStruct[fil][i]['filtered_state_covariances']=filtered_state_covariances
                        MainStruct[fil][i]['smoothed_state_means']=smoothed_state_means
                        MainStruct[fil][i]['smoothed_state_covariances']=smoothed_state_covariances
                        MainStruct[fil][i]['kalman_smoothing_gains']=kalman_smoothing_gains
                        MainStruct[fil][i]['pairwise_covariances']=pairwise_covariances

                        
                fil+=1
        return MainStruct
def GlobalVariables_Modified(*args):
        data=args[0]
        filt=args[1]
        nstates=filt.n_dim_state
        time,dims,trainN=np.shape(data)
        fil=0
        #geting the smoothings
        MainStruct={}
        for i in range(trainN):
                #A=filt.smooth(data[fil,:,:,i])
                MainStruct[i]={}
                (predicted_state_means, predicted_state_covariances,kalman_gains, filtered_state_means,filtered_state_covariances)=(_filter(filt.transition_matrices,filt.observation_matrices,filt.transition_covariance,filt.observation_covariance,filt.transition_offsets,filt.observation_offsets,filt.initial_state_mean,filt.initial_state_covariance,data[:,:,i]))
                (smoothed_state_means, smoothed_state_covariances,kalman_smoothing_gains)=(_smooth(filt.transition_matrices,filtered_state_means,filtered_state_covariances,predicted_state_means,predicted_state_covariances))
                #print smoothed_state_means
                pairwise_covariances = _smooth_pair(smoothed_state_covariances,kalman_smoothing_gains)
                #Master_Smooth_states[fil,i,:,:]=A[0]
                #print sigma_pair_smooth
                MainStruct[i]['predicted_state_means']=predicted_state_means
                MainStruct[i]['predicted_state_covariances']=predicted_state_covariances
                MainStruct[i]['kalman_gains']=kalman_gains
                MainStruct[i]['filtered_state_means']=filtered_state_means
                MainStruct[i]['filtered_state_covariances']=filtered_state_covariances
                MainStruct[i]['smoothed_state_means']=smoothed_state_means
                MainStruct[i]['smoothed_state_covariances']=smoothed_state_covariances
                MainStruct[i]['kalman_smoothing_gains']=kalman_smoothing_gains
                MainStruct[i]['pairwise_covariances']=pairwise_covariances
        return MainStruct


def _em_observation_matrix(observations, observation_offsets,MainStruct_reduced):
    r"""Apply the EM algorithm to parameter `observation_matrix`

    Maximize expected log likelihood of observations with respect to the
    observation matrix `observation_matrix`.

    .. math::

        C &= ( \sum_{t=0}^{T-1} (z_t - d_t) \mathbb{E}[x_t]^T )
             ( \sum_{t=0}^{T-1} \mathbb{E}[x_t x_t^T] )^-1

    """
    Ntrain=len(MainStruct_reduced)
    smoothed_state_means=MainStruct_reduced[0]['smoothed_state_means']#initialize smoothed_state_means with the first training example
    smoothed_state_covariances=MainStruct_reduced[0]['smoothed_state_covariances']
    for i in range(Ntrain-1):
        smoothed_state_means=np.dstack((smoothed_state_means,MainStruct_reduced[i+1]['smoothed_state_means']))
    _, n_dim_state,Ntrain = smoothed_state_means.shape
    n_timesteps, n_dim_obs,Ntrain = observations.shape
    smoothed_state_covariances=np.zeros((Ntrain,_,n_dim_state,n_dim_state))
    for i in range(Ntrain):
        smoothed_state_covariances[i,:,:,:]=MainStruct_reduced[i]['smoothed_state_covariances']
    res1 = np.zeros((n_dim_obs, n_dim_state))
    res2 = np.zeros((n_dim_state, n_dim_state))
    for N in range(Ntrain):
            for t in range(n_timesteps):
                if not np.any(np.ma.getmask(observations[t,:,N])):
                    observation_offset = _last_dims(observation_offsets, t, ndims=1)
                    res1 += np.outer(observations[t,:,N] - observation_offset,
                                     smoothed_state_means[t,:,N])
                    res2 += (
                        smoothed_state_covariances[N,t,:,:]
                        + np.outer(smoothed_state_means[t,:,N], smoothed_state_means[t,:,N])#outer is a surrogato to avoid declaring the dimensions of the matrices
                    )
    return np.dot(res1, linalg.pinv(res2))


def _em_observation_covariance(observations, observation_offsets,
                              transition_matrices, MainStruct_reduced):
    r"""Apply the EM algorithm to parameter `observation_covariance`

    Maximize expected log likelihood of observations with respect to the
    observation covariance matrix `observation_covariance`.

    .. math::

        R &= \frac{1}{T} \sum_{t=0}^{T-1}
                [z_t - C_t \mathbb{E}[x_t] - b_t]
                    [z_t - C_t \mathbb{E}[x_t] - b_t]^T
                + C_t Var(x_t) C_t^T
    """
    Ntrain=len(MainStruct_reduced)
    smoothed_state_means=MainStruct_reduced[0]['smoothed_state_means']#initialize smoothed_state_means with the first training example
    smoothed_state_covariances=MainStruct_reduced[0]['smoothed_state_covariances']
    for i in range(Ntrain-1):
        smoothed_state_means=np.dstack((smoothed_state_means,MainStruct_reduced[i+1]['smoothed_state_means']))
    _, n_dim_state,Ntrain = smoothed_state_means.shape
    smoothed_state_covariances=np.zeros((Ntrain,_,n_dim_state,n_dim_state))
    for i in range(Ntrain):
        smoothed_state_covariances[i,:,:,:]=MainStruct_reduced[i]['smoothed_state_covariances']
    n_timesteps, n_dim_obs,Ntrain = observations.shape
    res = np.zeros((n_dim_obs, n_dim_obs))
    n_obs = 0
    for N in range(Ntrain):
            for t in range(n_timesteps):
                if not np.any(np.ma.getmask(observations[t,:,N])):
                    transition_matrix = _last_dims(transition_matrices, t)
                    transition_offset = _last_dims(observation_offsets, t, ndims=1)
                    #print transition_offset
                    #raw_input()
                    err = (
                        observations[t,:,N]
                        - np.dot(transition_matrix, smoothed_state_means[t,:,N])
                        - transition_offset
                    )
                    res += (
                        np.outer(err, err)
                        + np.dot(transition_matrix,
                                 np.dot(smoothed_state_covariances[N,t,:,:],
                                        transition_matrix.T))
                    )
                    n_obs += 1
    if n_obs > 0:
        return (1.0 /( Ntrain*n_obs)) * res
    else:
        return res


def _em_transition_matrix(transition_offsets, MainStruct_reduced):
    r"""Apply the EM algorithm to parameter `transition_matrix`

    Maximize expected log likelihood of observations with respect to the state
    transition matrix `transition_matrix`.

    .. math::

        A &= ( \sum_{t=1}^{T-1} \mathbb{E}[x_t x_{t-1}^{T}]
                - b_{t-1} \mathbb{E}[x_{t-1}]^T )
             ( \sum_{t=1}^{T-1} \mathbb{E}[x_{t-1} x_{t-1}^T] )^{-1}
    """
    
    Ntrain=len(MainStruct_reduced)
    smoothed_state_means=MainStruct_reduced[0]['smoothed_state_means']#initialize smoothed_state_means with the first training example
    smoothed_state_covariances=MainStruct_reduced[0]['smoothed_state_covariances']#initialize covariances
    pairwise_covariances=MainStruct_reduced[0]['pairwise_covariances']
    for i in range(Ntrain-1):
        smoothed_state_means=np.dstack((smoothed_state_means,MainStruct_reduced[i+1]['smoothed_state_means']))
    _, n_dim_state,Ntrain = smoothed_state_means.shape
    smoothed_state_covariances=np.zeros((Ntrain,_,n_dim_state,n_dim_state))
    pairwise_covariances=np.zeros((Ntrain,_,n_dim_state,n_dim_state))
    for i in range(Ntrain):
        smoothed_state_covariances[i,:,:,:]=MainStruct_reduced[i]['smoothed_state_covariances']
        pairwise_covariances[i,:,:,:]=MainStruct_reduced[i]['pairwise_covariances']
    res1 = np.zeros((n_dim_state, n_dim_state))
    res2 = np.zeros((n_dim_state, n_dim_state))
    Ntrain,n_timesteps, n_dim_state, _ = smoothed_state_covariances.shape
    for N in range(Ntrain):
            for t in range(1, n_timesteps):
                transition_offset = _last_dims(transition_offsets,t-1, ndims=1)
                res1 += (
                    pairwise_covariances[N,t,:,:]
                    + np.outer(smoothed_state_means[t,:,N],
                               smoothed_state_means[t-1,:,N])
                    - np.outer(transition_offset, smoothed_state_means[t-1,:,N])
                )
                res2 += (
                    smoothed_state_covariances[N,t-1,:,:]
                    + np.outer(smoothed_state_means[t-1,:,N],
                               smoothed_state_means[t-1,:,N])
                )
    return np.dot(res1, linalg.pinv(res2))


def _em_transition_covariance(transition_matrices, transition_offsets,
                              MainStruct_reduced):
    r"""Apply the EM algorithm to parameter `transition_covariance`

    Maximize expected log likelihood of observations with respect to the
    transition covariance matrix `transition_covariance`.

    .. math::

        Q &= \frac{1}{T-1} \sum_{t=0}^{T-2}
                (\mathbb{E}[x_{t+1}] - A_t \mathbb{E}[x_t] - b_t)
                    (\mathbb{E}[x_{t+1}] - A_t \mathbb{E}[x_t] - b_t)^T
                + A_t Var(x_t) A_t^T + Var(x_{t+1})
                - Cov(x_{t+1}, x_t) A_t^T - A_t Cov(x_t, x_{t+1})
    """
    Ntrain=len(MainStruct_reduced)
    smoothed_state_means=MainStruct_reduced[0]['smoothed_state_means']#initialize smoothed_state_means with the first training example
    for i in range(Ntrain-1):
        smoothed_state_means=np.dstack((smoothed_state_means,MainStruct_reduced[i+1]['smoothed_state_means']))
    _, n_dim_state,Ntrain = smoothed_state_means.shape
    smoothed_state_covariances=np.zeros((Ntrain,_,n_dim_state,n_dim_state))
    pairwise_covariances=np.zeros((Ntrain,_,n_dim_state,n_dim_state))
    for i in range(Ntrain):
        smoothed_state_covariances[i,:,:,:]=MainStruct_reduced[i]['smoothed_state_covariances']
        pairwise_covariances[i,:,:,:]=MainStruct_reduced[i]['pairwise_covariances']
    Ntrain,n_timesteps, n_dim_state, _ = smoothed_state_covariances.shape
    res = np.zeros((n_dim_state, n_dim_state))
    for N in range(Ntrain):
            for t in range(n_timesteps - 1):
                transition_matrix = _last_dims(transition_matrices, t)
                transition_offset = _last_dims(transition_offsets, t, ndims=1)
                err = (
                    smoothed_state_means[t+1,:,N]
                    - np.dot(transition_matrix, smoothed_state_means[t,:,N])
                    - transition_offset
                )
                Vt1t_A = (
                    np.dot(pairwise_covariances[N,t+1,:,:],
                           transition_matrix.T)
                )
                res += (
                    np.outer(err, err)
                    + np.dot(transition_matrix,
                             np.dot(smoothed_state_covariances[N,t,:,:],
                                    transition_matrix.T))
                    + smoothed_state_covariances[N,t + 1,:,:]
                    - Vt1t_A - Vt1t_A.T
                )

    return (1.0 /(Ntrain*(n_timesteps - 1))) * res


def _em_initial_state_mean(MainStruct_reduced):
    r"""Apply the EM algorithm to parameter `initial_state_mean`

    Maximize expected log likelihood of observations with respect to the
    initial state distribution mean `initial_state_mean`.

    .. math::

        \mu_0 = \mathbb{E}[x_0]
    """
        #modify to accept multiple training instances, and as such is the average of initial states
    Ntrain=len(MainStruct_reduced)
    _,n_dim_state=MainStruct_reduced[0]['smoothed_state_means'].shape
    Mean_Smoothed=np.zeros((n_dim_state))
    for N in range(Ntrain):
        Mean_Smoothed+=MainStruct_reduced[N]['smoothed_state_means'][0]
    Mean_Smoothed/=Ntrain
    return Mean_Smoothed


def _em_initial_state_covariance(initial_state_mean, MainStruct_reduced):
    r"""Apply the EM algorithm to parameter `initial_state_covariance`

    Maximize expected log likelihood of observations with respect to the
    covariance of the initial state distribution `initial_state_covariance`.

    .. math::

        \Sigma_0 = \mathbb{E}[x_0, x_0^T] - mu_0 \mathbb{E}[x_0]^T
                   - \mathbb{E}[x_0] mu_0^T + mu_0 mu_0^T
    """
    Ntrain=len(MainStruct_reduced)
    _,n_dim_state=MainStruct_reduced[0]['smoothed_state_means'].shape
    Mean_Smoothed=np.zeros((1,n_dim_state))
    Covariance_Smoothed=np.zeros((n_dim_state,n_dim_state))
    for N in range(Ntrain):
        Covariance_Smoothed+=MainStruct_reduced[N]['smoothed_state_covariances'][0]
        Mean_Smoothed+=MainStruct_reduced[N]['smoothed_state_means'][0]
    Mean_Smoothed/=Ntrain
    Covariance_Smoothed/=Ntrain
    x0 = Mean_Smoothed
    x0_x0 = Covariance_Smoothed + np.outer(x0, x0)
    return (
        x0_x0
        - np.outer(initial_state_mean, x0)
        - np.outer(x0, initial_state_mean)
        + np.outer(Mean_Smoothed, Mean_Smoothed)
    )


def _em_transition_offset(transition_matrices, MainStruct_reduced):
    r"""Apply the EM algorithm to parameter `transition_offset`

    Maximize expected log likelihood of observations with respect to the
    state transition offset `transition_offset`.

    .. math::

        b = \frac{1}{T-1} \sum_{t=1}^{T-1}
                \mathbb{E}[x_t] - A_{t-1} \mathbb{E}[x_{t-1}]
    """
    Ntrain=len(MainStruct_reduced)
    smoothed_state_means=MainStruct_reduced[0]['smoothed_state_means']#initialize smoothed_state_means with the first training example
    smoothed_state_covariances=MainStruct_reduced[0]['smoothed_state_covariances']
    for i in range(Ntrain-1):
        smoothed_state_means=np.dstack((smoothed_state_means,MainStruct_reduced[i+1]['smoothed_state_means']))
    n_timesteps, n_dim_state,Ntrain = smoothed_state_means.shape
    #n_timesteps, n_dim_state = smoothed_state_means.shape
    transition_offset = np.zeros(n_dim_state)
    for N in range(Ntrain):
            for t in range(1, n_timesteps):
                transition_matrix = _last_dims(transition_matrices, t-1)
                transition_offset += (
                    smoothed_state_means[t,:,N]
                    - np.dot(transition_matrix, smoothed_state_means[t-1,:,N])
                )
    if n_timesteps > 1:
        return (1.0 / (Ntrain*(n_timesteps-1)))*transition_offset
    else:
        return np.zeros(n_dim_state)


def _em_observation_offset(observation_matrices, MainStruct_reduced,
                           observations):
    r"""Apply the EM algorithm to parameter `observation_offset`

    Maximize expected log likelihood of observations with respect to the
    observation offset `observation_offset`.

    .. math::

        d = \frac{1}{T} \sum_{t=0}^{T-1} z_t - C_{t} \mathbb{E}[x_{t}]
    """
    Ntrain=len(MainStruct_reduced)
    smoothed_state_means=MainStruct_reduced[0]['smoothed_state_means']#initialize smoothed_state_means with the first training example
    for i in range(Ntrain-1):
        smoothed_state_means=np.dstack((smoothed_state_means,MainStruct_reduced[i+1]['smoothed_state_means']))
    _, n_dim_state,Ntrain = smoothed_state_means.shape
    n_timesteps, n_dim_obs,Ntrain = observations.shape
    observation_offset = np.zeros(n_dim_obs)
    for N in range(Ntrain):
            n_obs=0
            for t in range(n_timesteps):
                if not np.any(np.ma.getmask(observations[t])):
                    observation_matrix = _last_dims(observation_matrices, t)
                    observation_offset += (
                        observations[t,:,N]
                        - np.dot(observation_matrix, smoothed_state_means[t,:,N])
                    )
                    n_obs += 1
    if n_obs > 0:
        return (1.0 / (Ntrain*n_obs)) * observation_offset
    else:
        return observation_offset

def _last_dims(X, t, ndims=2):
    """Extract the final dimensions of `X`

    Extract the final `ndim` dimensions at index `t` if `X` has >= `ndim` + 1
    dimensions, otherwise return `X`.

    Parameters
    ----------
    X : array with at least dimension `ndims`
    t : int
        index to use for the `ndims` + 1th dimension
    ndims : int, optional
        number of dimensions in the array desired

    Returns
    -------
    Y : array with dimension `ndims`
        the final `ndims` dimensions indexed by `t`
    """
    X = np.asarray(X)
    if len(X.shape) == ndims + 1:
        return X[t]
    elif len(X.shape) == ndims:
        return X
    else:
        raise ValueError(("X only has %d dimensions when %d" +
                " or more are required") % (len(X.shape), ndims))

def _filter(transition_matrices, observation_matrices, transition_covariance,
            observation_covariance, transition_offsets, observation_offsets,
            initial_state_mean, initial_state_covariance, observations):
    """Apply the Kalman Filter

    Calculate posterior distribution over hidden states given observations up
    to and including the current time step.

    Parameters
    ----------
    transition_matrices : [n_timesteps-1,n_dim_state,n_dim_state] or
    [n_dim_state,n_dim_state] array-like
        state transition matrices
    observation_matrices : [n_timesteps, n_dim_obs, n_dim_obs] or [n_dim_obs, \
    n_dim_obs] array-like
        observation matrix
    transition_covariance : [n_timesteps-1,n_dim_state,n_dim_state] or
    [n_dim_state,n_dim_state] array-like
        state transition covariance matrix
    observation_covariance : [n_timesteps, n_dim_obs, n_dim_obs] or [n_dim_obs,
    n_dim_obs] array-like
        observation covariance matrix
    transition_offsets : [n_timesteps-1, n_dim_state] or [n_dim_state] \
    array-like
        state offset
    observation_offsets : [n_timesteps, n_dim_obs] or [n_dim_obs] array-like
        observations for times [0...n_timesteps-1]
    initial_state_mean : [n_dim_state] array-like
        mean of initial state distribution
    initial_state_covariance : [n_dim_state, n_dim_state] array-like
        covariance of initial state distribution
    observations : [n_timesteps, n_dim_obs] array
        observations from times [0...n_timesteps-1].  If `observations` is a
        masked array and any of `observations[t]` is masked, then
        `observations[t]` will be treated as a missing observation.

    Returns
    -------
    predicted_state_means : [n_timesteps, n_dim_state] array
        `predicted_state_means[t]` = mean of hidden state at time t given
        observations from times [0...t-1]
    predicted_state_covariances : [n_timesteps, n_dim_state, n_dim_state] array
        `predicted_state_covariances[t]` = covariance of hidden state at time t
        given observations from times [0...t-1]
    kalman_gains : [n_timesteps, n_dim_state] array
        `kalman_gains[t]` = Kalman gain matrix for time t
    filtered_state_means : [n_timesteps, n_dim_state] array
        `filtered_state_means[t]` = mean of hidden state at time t given
        observations from times [0...t]
    filtered_state_covariances : [n_timesteps, n_dim_state] array
        `filtered_state_covariances[t]` = covariance of hidden state at time t
        given observations from times [0...t]
    """
    n_timesteps = observations.shape[0]
    n_dim_state = len(initial_state_mean)
    n_dim_obs = observations.shape[1]

    predicted_state_means = np.zeros((n_timesteps, n_dim_state))
    predicted_state_covariances = np.zeros(
        (n_timesteps, n_dim_state, n_dim_state)
    )
    kalman_gains = np.zeros((n_timesteps, n_dim_state, n_dim_obs))
    filtered_state_means = np.zeros((n_timesteps, n_dim_state))
    filtered_state_covariances = np.zeros(
        (n_timesteps, n_dim_state, n_dim_state)
    )

    for t in range(n_timesteps):
        if t == 0:
            predicted_state_means[t] = initial_state_mean
            predicted_state_covariances[t] = initial_state_covariance
        else:
            transition_matrix = _last_dims(transition_matrices, t - 1)
            transition_covariance = _last_dims(transition_covariance, t - 1)
            transition_offset = _last_dims(transition_offsets, t - 1, ndims=1)
            predicted_state_means[t], predicted_state_covariances[t] = (
                _filter_predict(
                    transition_matrix,
                    transition_covariance,
                    transition_offset,
                    filtered_state_means[t - 1],
                    filtered_state_covariances[t - 1]
                )
            )

        observation_matrix = _last_dims(observation_matrices, t)
        observation_covariance = _last_dims(observation_covariance, t)
        observation_offset = _last_dims(observation_offsets, t, ndims=1)
        (kalman_gains[t], filtered_state_means[t],
         filtered_state_covariances[t]) = (
            _filter_correct(observation_matrix,
                observation_covariance,
                observation_offset,
                predicted_state_means[t],
                predicted_state_covariances[t],
                observations[t]
            )
        )

    return (predicted_state_means, predicted_state_covariances,
            kalman_gains, filtered_state_means,
            filtered_state_covariances)

def _smooth(transition_matrices, filtered_state_means,
            filtered_state_covariances, predicted_state_means,
            predicted_state_covariances):
    """Apply the Kalman Smoother

    Estimate the hidden state at time for each time step given all
    observations.

    Parameters
    ----------
    transition_matrices : [n_timesteps-1, n_dim_state, n_dim_state] or \
    [n_dim_state, n_dim_state] array
        `transition_matrices[t]` = transition matrix from time t to t+1
    filtered_state_means : [n_timesteps, n_dim_state] array
        `filtered_state_means[t]` = mean state estimate for time t given
        observations from times [0...t]
    filtered_state_covariances : [n_timesteps, n_dim_state, n_dim_state] array
        `filtered_state_covariances[t]` = covariance of state estimate for time
        t given observations from times [0...t]
    predicted_state_means : [n_timesteps, n_dim_state] array
        `predicted_state_means[t]` = mean state estimate for time t given
        observations from times [0...t-1]
    predicted_state_covariances : [n_timesteps, n_dim_state, n_dim_state] array
        `predicted_state_covariances[t]` = covariance of state estimate for
        time t given observations from times [0...t-1]

    Returns
    -------
    smoothed_state_means : [n_timesteps, n_dim_state]
        mean of hidden state distributions for times [0...n_timesteps-1] given
        all observations
    smoothed_state_covariances : [n_timesteps, n_dim_state, n_dim_state] array
        covariance matrix of hidden state distributions for times
        [0...n_timesteps-1] given all observations
    kalman_smoothing_gains : [n_timesteps-1, n_dim_state, n_dim_state] array
        Kalman Smoothing correction matrices for times [0...n_timesteps-2]
    """
    n_timesteps, n_dim_state = filtered_state_means.shape

    smoothed_state_means = np.zeros((n_timesteps, n_dim_state))
    smoothed_state_covariances = np.zeros((n_timesteps, n_dim_state,
                                           n_dim_state))
    kalman_smoothing_gains = np.zeros((n_timesteps - 1, n_dim_state,
                                       n_dim_state))

    smoothed_state_means[-1] = filtered_state_means[-1]
    smoothed_state_covariances[-1] = filtered_state_covariances[-1]

    for t in reversed(range(n_timesteps - 1)):
        transition_matrix = _last_dims(transition_matrices, t)
        (smoothed_state_means[t], smoothed_state_covariances[t],
         kalman_smoothing_gains[t]) = (
            _smooth_update(
                transition_matrix,
                filtered_state_means[t],
                filtered_state_covariances[t],
                predicted_state_means[t + 1],
                predicted_state_covariances[t + 1],
                smoothed_state_means[t + 1],
                smoothed_state_covariances[t + 1]
            )
        )
    return (smoothed_state_means, smoothed_state_covariances,
            kalman_smoothing_gains)


def _smooth_pair(smoothed_state_covariances, kalman_smoothing_gain):
    r"""Calculate pairwise covariance between hidden states

    Calculate covariance between hidden states at :math:`t` and :math:`t-1` for
    all time step pairs

    Parameters
    ----------
    smoothed_state_covariances : [n_timesteps, n_dim_state, n_dim_state] array
        covariance of hidden state given all observations
    kalman_smoothing_gain : [n_timesteps-1, n_dim_state, n_dim_state]
        Correction matrices from Kalman Smoothing

    Returns
    -------
    pairwise_covariances : [n_timesteps, n_dim_state, n_dim_state] array
        Covariance between hidden states at times t and t-1 for t =
        [1...n_timesteps-1].  Time 0 is ignored.
    """
    n_timesteps, n_dim_state, _ = smoothed_state_covariances.shape
    pairwise_covariances = np.zeros((n_timesteps, n_dim_state, n_dim_state))
    for t in range(1, n_timesteps):
        pairwise_covariances[t] = (
            np.dot(smoothed_state_covariances[t],
                   kalman_smoothing_gain[t - 1].T)
        )
    return pairwise_covariances


def _filter_predict(transition_matrix, transition_covariance,
                    transition_offset, current_state_mean,
                    current_state_covariance):
    r"""Calculate the mean and covariance of :math:`P(x_{t+1} | z_{0:t})`

    Using the mean and covariance of :math:`P(x_t | z_{0:t})`, calculate the
    mean and covariance of :math:`P(x_{t+1} | z_{0:t})`.

    Parameters
    ----------
    transition_matrix : [n_dim_state, n_dim_state} array
        state transition matrix from time t to t+1
    transition_covariance : [n_dim_state, n_dim_state] array
        covariance matrix for state transition from time t to t+1
    transition_offset : [n_dim_state] array
        offset for state transition from time t to t+1
    current_state_mean: [n_dim_state] array
        mean of state at time t given observations from times
        [0...t]
    current_state_covariance: [n_dim_state, n_dim_state] array
        covariance of state at time t given observations from times
        [0...t]

    Returns
    -------
    predicted_state_mean : [n_dim_state] array
        mean of state at time t+1 given observations from times [0...t]
    predicted_state_covariance : [n_dim_state, n_dim_state] array
        covariance of state at time t+1 given observations from times
        [0...t]
    """
    predicted_state_mean = (
        np.dot(transition_matrix, current_state_mean)
        + transition_offset
    )
    predicted_state_covariance = (
        np.dot(transition_matrix,
               np.dot(current_state_covariance,
                      transition_matrix.T))
        + transition_covariance
    )

    return (predicted_state_mean, predicted_state_covariance)


def _filter_correct(observation_matrix, observation_covariance,
                    observation_offset, predicted_state_mean,
                    predicted_state_covariance, observation):
    r"""Correct a predicted state with a Kalman Filter update

    Incorporate observation `observation` from time `t` to turn
    :math:`P(x_t | z_{0:t-1})` into :math:`P(x_t | z_{0:t})`

    Parameters
    ----------
    observation_matrix : [n_dim_obs, n_dim_state] array
        observation matrix for time t
    observation_covariance : [n_dim_obs, n_dim_obs] array
        covariance matrix for observation at time t
    observation_offset : [n_dim_obs] array
        offset for observation at time t
    predicted_state_mean : [n_dim_state] array
        mean of state at time t given observations from times
        [0...t-1]
    predicted_state_covariance : [n_dim_state, n_dim_state] array
        covariance of state at time t given observations from times
        [0...t-1]
    observation : [n_dim_obs] array
        observation at time t.  If `observation` is a masked array and any of
        its values are masked, the observation will be ignored.

    Returns
    -------
    kalman_gain : [n_dim_state, n_dim_obs] array
        Kalman gain matrix for time t
    corrected_state_mean : [n_dim_state] array
        mean of state at time t given observations from times
        [0...t]
    corrected_state_covariance : [n_dim_state, n_dim_state] array
        covariance of state at time t given observations from times
        [0...t]
    """
    if not np.any(np.ma.getmask(observation)):
        predicted_observation_mean = (
            np.dot(observation_matrix,
                   predicted_state_mean)
            + observation_offset
        )
        predicted_observation_covariance = (
            np.dot(observation_matrix,
                   np.dot(predicted_state_covariance,
                          observation_matrix.T))
            + observation_covariance
        )

        kalman_gain = (
            np.dot(predicted_state_covariance,
                   np.dot(observation_matrix.T,
                          linalg.pinv(predicted_observation_covariance)))
        )

        corrected_state_mean = (
            predicted_state_mean
            + np.dot(kalman_gain, observation - predicted_observation_mean)
        )
        corrected_state_covariance = (
            predicted_state_covariance
            - np.dot(kalman_gain,
                     np.dot(observation_matrix,
                            predicted_state_covariance))
        )
    else:
        n_dim_state = predicted_state_covariance.shape[0]
        n_dim_obs = observation_matrix.shape[0]
        kalman_gain = np.zeros((n_dim_state, n_dim_obs))

        corrected_state_mean = predicted_state_mean
        corrected_state_covariance = predicted_state_covariance

    return (kalman_gain, corrected_state_mean,
            corrected_state_covariance)
def _smooth_update(transition_matrix, filtered_state_mean,
                   filtered_state_covariance, predicted_state_mean,
                   predicted_state_covariance, next_smoothed_state_mean,
                   next_smoothed_state_covariance):
    r"""Correct a predicted state with a Kalman Smoother update

    Calculates posterior distribution of the hidden state at time `t` given the
    observations all observations via Kalman Smoothing.

    Parameters
    ----------
    transition_matrix : [n_dim_state, n_dim_state] array
        state transition matrix from time t to t+1
    filtered_state_mean : [n_dim_state] array
        mean of filtered state at time t given observations from
        times [0...t]
    filtered_state_covariance : [n_dim_state, n_dim_state] array
        covariance of filtered state at time t given observations from
        times [0...t]
    predicted_state_mean : [n_dim_state] array
        mean of filtered state at time t+1 given observations from
        times [0...t]
    predicted_state_covariance : [n_dim_state, n_dim_state] array
        covariance of filtered state at time t+1 given observations from
        times [0...t]
    next_smoothed_state_mean : [n_dim_state] array
        mean of smoothed state at time t+1 given observations from
        times [0...n_timesteps-1]
    next_smoothed_state_covariance : [n_dim_state, n_dim_state] array
        covariance of smoothed state at time t+1 given observations from
        times [0...n_timesteps-1]

    Returns
    -------
    smoothed_state_mean : [n_dim_state] array
        mean of smoothed state at time t given observations from times
        [0...n_timesteps-1]
    smoothed_state_covariance : [n_dim_state, n_dim_state] array
        covariance of smoothed state at time t given observations from
        times [0...n_timesteps-1]
    kalman_smoothing_gain : [n_dim_state, n_dim_state] array
        correction matrix for Kalman Smoothing at time t
    """
    kalman_smoothing_gain = (
        np.dot(filtered_state_covariance,
               np.dot(transition_matrix.T,
                      linalg.pinv(predicted_state_covariance)))
    )

    smoothed_state_mean = (
        filtered_state_mean
        + np.dot(kalman_smoothing_gain,
                 next_smoothed_state_mean - predicted_state_mean)
    )
    smoothed_state_covariance = (
        filtered_state_covariance
        + np.dot(kalman_smoothing_gain,
                 np.dot(
                    (next_smoothed_state_covariance
                        - predicted_state_covariance),
                    kalman_smoothing_gain.T
                 ))
    )

    return (smoothed_state_mean, smoothed_state_covariance,
            kalman_smoothing_gain)
def _smoothed_state_mean(MainStruct_reduced):
    r"""Apply the EM algorithm to parameter `initial_state_mean`

    Maximize expected log likelihood of observations with respect to the
    initial state distribution mean `initial_state_mean`.

    .. math::

        \mu_0 = \mathbb{E}[x_0]
    """
        #modify to accept multiple training instances, and as such is the average of initial states
    Ntrain=len(MainStruct_reduced)
    _,n_dim_state=MainStruct_reduced[0]['smoothed_state_means'].shape
    Mean_Smoothed=np.zeros((_,n_dim_state))
    for N in range(Ntrain):
        Mean_Smoothed+=MainStruct_reduced[N]['smoothed_state_means']
    Mean_Smoothed/=Ntrain
    return Mean_Smoothed


def _smoothed_state_covariance(smoothed_mean, MainStruct_reduced):
    r"""Apply the EM algorithm to parameter `initial_state_covariance`

    Maximize expected log likelihood of observations with respect to the
    covariance of the initial state distribution `initial_state_covariance`.

    .. math::

        \Sigma_0 = \mathbb{E}[x_0, x_0^T] - mu_0 \mathbb{E}[x_0]^T
                   - \mathbb{E}[x_0] mu_0^T + mu_0 mu_0^T
    """
    time_index, n_dim_state=MainStruct_reduced[0]['smoothed_state_means'].shape
    smoothed_covariance = np.zeros((time_index, n_dim_state))
    for time_idx in range(time_index):
        smoothed_covariance[time_idx,:] = _em_initial_state_covariance(smoothed_mean[time_idx], MainStruct_reduced)
    return (smoothed_covariance)

