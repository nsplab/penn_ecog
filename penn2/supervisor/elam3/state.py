import numpy as np
import random
import time

import config

# 0: training, 1: test
# used to tell the filter module the type of the current trial
triainingTestSeq = np.array([0, 0, 0, 0, 0, 0])


class GameState(object):

    def __init__(self, hand_pos=np.zeros(3),
                 box_pos=np.zeros(3), score=0, level=1, sublevel=1):
        self.hand_pos = hand_pos
        self.hand_pos[0] = -config.workspaceRadius / 2.0
        self.box_pos = box_pos
        self.score = score
        self.level = level
        self.sublevel = sublevel
        self.trial = 0
        self.pastTrial = -1
        self.numTrials = 3
        self.numSublevels = 2
        self.startingTimes = []
        self.lengths = []
        self.positions = []
        self.thicknesses = []
        self.currentBlock = -1
        self.start_time = time.time()
        self.pause = True
        self.accumTime = 0.0

    def generateBlocks(self):
        #add their starting points in time to a list
        #add their lengths to list
        #add their thicknest to a list
        #add their position/altitude to a list
        startingTime = 10.0
        blockLengthTime = 10.0  # seconds
        restLengthTime = 10.0   # seconds
        # blockThickness = config.workspaceRadius / 8.0
        for x in range(0, 1000):
            self.startingTimes.append(startingTime)
            if (x % 2) == 0:
                startingTime += restLengthTime
                self.positions.append(-0.5 * config.workspaceRadius)
                self.lengths.append(restLengthTime)
            else:
                startingTime += blockLengthTime
                self.positions.append(random.uniform(-0.5, 0.5) * config.workspaceRadius)
                self.lengths.append(blockLengthTime)

    def serializeBlocks(self):
        # based on current time advance among the blocks?
        svalue = "B "
        cBlock = -1
        elapsed_time = time.time() - self.start_time
        self.start_time = time.time()
        if not self.pause:
            self.accumTime += elapsed_time
        for x in range(0, 1000):
            # accum_time += self.startingTimes(x)
            print 'x ', x
            print self.startingTimes[x]
            print 'self.accumTime ', self.accumTime
            if self.accumTime < self.startingTimes[x]:
                break
            cBlock = x
        if cBlock > -1:
            time_remained_from_cblock = (self.startingTimes[cBlock] + self.lengths[cBlock]
                                         - self.accumTime)
            svalue += (str(time_remained_from_cblock) + " " +
                       str(self.startingTimes[cBlock + 1] - self.accumTime) + " " +
                       str(self.lengths[cBlock + 1]) + " ")
            svalue += (str(self.positions[cBlock]) + " 0 0 " +
                       str(self.positions[cBlock + 1]) + " 0 0")
            self.box_pos = np.zeros(3)
            self.box_pos[0] = self.positions[cBlock]
        else:
            svalue += ("0 " +
                       str(self.startingTimes[cBlock + 1]) + " " +
                       str(self.lengths[cBlock + 1]))
            svalue += (" 0 0 0 " +
                       str(self.positions[cBlock + 1]) + " 0 0")
            self.box_pos = np.zeros(3)

        # update the score
        #self.score +=

        self.trial = cBlock

        return svalue

    # convert the game state to ascii strin to be sent to the graphics module
    def serialize(self):
        svalue = " "
        svalue += (str(self.hand_pos[0]) + " " +
                  str(self.hand_pos[1]) + " " +
                  str(self.hand_pos[2]) + " ")
        svalue += (str(self.box_pos[0]) + " " +
                   str(self.box_pos[1]) + " " +
                   str(self.box_pos[2]) + " ")
        svalue += (str(self.score) + " " +
                   str(self.level) + " " +
                   str(self.sublevel) + " " +
                   str(self.trial))
        return svalue

    def updateState(self):
        # score based on distance from blocks
        if self.pastTrial == self.trial:
            ret = False
        else:
            ret = True
            self.pastTrial = self.trial
            # if new trial started and the trial is a rest jump the hand position to the origin
            if config.jumpetoStart:
                if (self.trial % 2) == 0:
                    self.hand_pos = np.zeros(3)
                    self.hand_pos[0] = -config.workspaceRadius / 2.0

        # advance the trial number, sublevel number, and the level number
        if ret:
            self.trial += 1
        if self.trial > self.numTrials:
            self.trial = 1
            self.sublevel += 1
        if self.sublevel > self.numSublevels:
            self.sublevel = 1
            self.level += 1

        return ret


class FilterState(object):

    def __init__(self, target_pos=np.zeros(3), hand_pos=np.zeros(3), trial=1,
                 attending=0.0):
        self.target_pos = target_pos
        self.hand_pos = hand_pos
        self.trial = trial
        self.attending = attending
        self.mode = triainingTestSeq[0]

    def serialize(self):
        svalue = (str(self.target_pos[0]) + " " +
                  str(self.target_pos[1]) + " " +
                  str(self.target_pos[2]) + " ")
        svalue += (str(self.hand_pos[0]) + " " +
                  str(self.hand_pos[1]) + " " +
                  str(self.hand_pos[2]) + " ")
        svalue += str(self.trial) + " "
        svalue += str(triainingTestSeq[self.trial % len(triainingTestSeq)])
        svalue += " " + str(self.attending)
        return svalue
