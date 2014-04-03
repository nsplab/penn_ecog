
#include <armadillo>
#include <map>
#include <ComputerEEGFilterRSE.h>
#include <ComputerEEGFilterCursorOnly.h>
#include <time.h>

void trainingRunOld()
{
    /* TODO: Code to initialize EEG system?
       Any code that does this should #include <parameters.h> and set 
       CHANNEL_VARIANCES, an N_CHANNELS x 1 mat that contains the variance of 
       the signal at each channel, recorded during some baseline testing
       scenario.
     */
    
    /* TODO: Code to initialize rendering system */
    
    // Initialize the filters.
    ComputerEEGFilterRSE trainComputer = new ComputerEEGFilterRSE();
    ComputerEEGFilterCursorOnly testComputer = \
        new ComputerEEGFilterCursorOnly();
        
    trainTrialNum = 0;
    testTrialNum = 0;
    for(int trialNum = 0; trialNum < N_TRIALS; trialNum++)
    {
        // Resets the filter for a new trial and updates pos with the starting
        // position of the hand.
        mat pos;
        if(TEST_TRIALS[trialNum] == false)
        {
            trainTrialNum++;
            pos = trainComputer.newTrial();
        }
        else
        {
            testTrialNum++;
            testComputer.setChannelParameters( \
                trainComputer.getChannelParameters);
            pos = testComputer.newTrial();
        }
        
        /* TODO: Let the user look at a frozen screen for
           TIME_BETWEEN_TRIALS seconds so he knows where his hand is in
           relation to the target.
         */
         timespec start, end;
         clock_gettime(CLOCK_REALTIME, &start);
         clock_gettime(CLOCK_REALTIME, &end);
         while(difftime(end.tv_sec - start.tv_sec) < TIME_BETWEEN_TRIALS)
            clock_gettime(CLOCK_REALTIME, &end);
             
        // The loop that runs the trial. Each iteration is one time step.
        int timeStep = 0;
        int holdCount = 0;
        while(timeStep < MAX_TRIAL_LENGTH && holdCount < HOLD_TIME_STEPS)
        {
            timespec start, end;
            clock_gettime(CLOCK_REALTIME, &start);
            
            // Get EEG signals as N_CHANNELS x 1 mat, and get the updated
            // position of the arm from the filter.
            mat eegSignals = /* TODO: function that obtains EEG signal */;
            map<string, mat> dataMap;
            mat pos;
            if(TEST_TRIALS[trialNum] == false)
            {
                /* TODO: The other things you can extract out of dataMap include
                         "state", "covariance", "velocity", and
                         "channelParameters". The channel parameters, 
                         covariance, and position should all be saved into a
                         data structure of your choosing. */
                dataMap = trainComputer.filter(eegSignals);
                pos = dataMap("position");
            }
            else
            {
                /* TODO: Same as in the training trials case. */
                dataMap = testComputer.filter(eegSignals);
                pos = dataMap("position");
            }
            
            /* TODO: Update position of hand on screen. */
            
            if(norm(REACH_TARGET_POS - pos, 2) < TARGET_RADIUS)
            {
                holdCount++;
            }
            else
            {
                holdCount = 0;
            }
            
            timeStep++;
            
            // Wait until the end of the time step.
            clock_gettime(CLOCK_REALTIME, &end);
            while(difftime(end.tv_sec - start.tv_sec) < TIME_BIN)
                clock_gettime(CLOCK_REALTIME, &end);
        }
    }   
}