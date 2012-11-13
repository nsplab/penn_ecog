#include <armadillo>

mat generateChannelParameters() {
    mat channelParameters = zeros<mat>(N_CHANNELS, 3 * N_LAGS);
    
    return channelParameters;
}


    % Parameters for the neural cortex. There are 'nNeurons' neurons, each
    % represented as a vector 'v_i' in the Cartesian plane. 
    neuronParameters = zeros(nNeurons, 3);
    for i = 1:nNeurons
        if(strcmp(dirGen, 'random'))
            direction = rand() * 2 * pi;
        elseif(strcmp(dirGen, 'custom'))
            v = dirList(i, :);
            direction = atan2(v(2), v(1));
        end

        if(strcmp(magGen, 'random'))
            baseFiringRate = 10 * rand() + 10; % 10 - 20 spikes / s
            maxFiringRate = 15 * rand() + 25; % 25 - 40 spikes / s

            % With the base and max firing rates above, we have that
            %   2.30 < constant  < 3.00 and
            %   1.12 < magnitude < 6.93,
            % assuming that the max firing rate is for a speed of 20 cm / s.
            constant = log(baseFiringRate);
            magnitude = (log(maxFiringRate) - constant) / 0.2;
        elseif(strcmp(magGen, 'mean'))
            constant = log(40) - 1;
            magnitude = 5 * (log(32) - 5 * log(5) / 3);
        elseif(strcmp(magGen, 'custom'))
            baseFiringRate = 10 * baseFactor + 10; % 10 - 20 spikes / s
            maxFiringRate = 15 * maxFactor + 25; % 25 - 40 spikes / s
            
            constant = log(baseFiringRate);
            magnitude = (log(maxFiringRate) - constant) / 0.2;
        end

        neuronParameters(i, :) = [cos(direction) * magnitude ...
            sin(direction) * magnitude constant];
    end

end