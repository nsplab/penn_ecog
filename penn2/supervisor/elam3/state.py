import numpy as np
import random
import time
import math

import config

#####################################################################
## In the supervisor the origin is at the back left bottom of the working space
## box. This is also communicated to the other modules. In the
## graphics module (elam3) the coordinates are shifted and the bottom
## right corner of the box (corner closer to the screen), but this is
## internal to elam3 and does NOT affect the other modules.
## For all the modules, except elam3,
##
##
##              Y ^
##                |
##                |
##                |__________> x
##               /
##              /
##             /
##         Z  v
##
#####################################################################


# 0: training, 1: test
# used to tell the filter module the type of the current trial
triainingTestSeq = np.array([0, 0, 0, 0, 0, 0])


class GameState(object):

    def __init__(self, workspaceRadius, blockWidth, blockLengthTime,
                 hand_pos=np.zeros(3),
                 box_pos=np.zeros(3), score=0, level=1, sublevel=1):
        self.hand_pos = hand_pos
        self.hand_pos[0] = 0.5
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
        self.prevPause = True
        self.accumTime = 0.0
        self.scoreRefTime = 0
        self.scoreCurTime = 0
        self.scorePlotRefTime = 0
        self.scorePlot = -1
        self.scorePrevPlot = 0
        self.blockLengthTime = blockLengthTime
        self.blockWidth = blockWidth
        self.workspaceRadius = workspaceRadius
        self.isAtTarget = 0 # Using a number rather than true/false for transmitting to MATLAB filter

    # generate 1000 target bars prior to running the game
    def generateBlocks(self, workspace_axis):
        #add their starting points in time to a list
        #add their lengths to list
        #add their thicknest to a list
        #add their position/altitude to a list
        startingTime = 10.0
        # blockThickness = config.workspaceRadius / 8.0

        ## TODO: change this so the target bars are generated in N dimension and X axis
        ## depending on the parameter that the launcher sets
        for x in range(0, 1000):
            self.startingTimes.append(startingTime)
            #if (x % 2) == 0:
                #startingTime += self.blockLengthTime
                #self.positions.append(-0.5 * self.workspaceRadius)
                #self.lengths.append(self.blockLengthTime)
            #else:
            startingTime += self.blockLengthTime
            if workspace_axis == 1:
                self.positions.append(random.uniform(
                                      0.0,
                                      1.0))
            if workspace_axis == 2:
                self.positions.append(random.uniform(
                                      0.0,
                                      1.0))
            if workspace_axis == 4:
                self.positions.append([random.uniform(0.0, 1.0), random.uniform(0.0, 1.0)])

            self.lengths.append(self.blockLengthTime)

    def serializeBlocks(self, workspace_axis):
        # based on current time advance among the blocks?
        svalue = "B "
        cBlock = -1
        elapsed_time = time.time() - self.start_time

        self.start_time = time.time()
        if not self.pause:
            self.accumTime += elapsed_time
        for x in range(0, 1000):
            # accum_time += self.startingTimes(x)
            #print 'x ', x
            #print self.startingTimes[x]
            #print 'self.accumTime ', self.accumTime
            if self.accumTime < self.startingTimes[x]:
                break
            cBlock = x
        if cBlock > -1:
            time_remained_from_cblock = (self.startingTimes[cBlock] + self.lengths[cBlock]
                                         - self.accumTime)
            svalue += (str(time_remained_from_cblock) + " " +
                       str(self.startingTimes[cBlock + 1] - self.accumTime) + " " +
                       str(self.lengths[cBlock + 1]) + " ")
            ## TODO: depending onth e selected axis by launcher set the the other axises ot zero

            ## TODO: zero velocity target bars fir 3d
            ## also make for this the length short enough to be a square / show target position
            self.box_pos = np.zeros(3)
            if workspace_axis == 1:
                self.box_pos[0] = self.positions[cBlock]
                self.box_pos[1] = 0.0
                self.box_pos[2] = 0.0
                svalue += (str(self.positions[cBlock]) + " 0 0 " +
                           str(self.positions[cBlock + 1]) + " 0 0 ")
            if workspace_axis == 2:
                self.box_pos[0] = 1.0
                self.box_pos[1] = self.positions[cBlock]
                self.box_pos[2] = 0.0
                svalue += ("1 " + str(self.positions[cBlock]) + " 0 " +
                          "1 " + str(self.positions[cBlock + 1]) + " 0 ")
            if workspace_axis == 4:
                self.box_pos[0] = self.positions[cBlock][0]
                self.box_pos[1] = self.positions[cBlock][1]
                self.box_pos[2] = 0.0
                svalue += (str(self.positions[cBlock][0]) + " " + str(self.positions[cBlock][1]) + " 0 " +
                          str(self.positions[cBlock + 1][0]) + " " + str(self.positions[cBlock + 1][1]) + " 0 ")

            svalue += (str(self.blockWidth) + " ")
        else:
            svalue += ("0 " +
                       str(self.startingTimes[cBlock + 1] - self.accumTime) + " " +
                       str(self.lengths[cBlock + 1]))
            if workspace_axis == 1:
                svalue += (" 1 0 0 " +
                           str(self.positions[cBlock + 1]) + " 0 0 ")
            if workspace_axis == 2:
                svalue += (" 1 0 0 " +
                           " 1 " + str(self.positions[cBlock + 1]) + " 0 ")
            if workspace_axis == 4:
                svalue += (" 1 0 0 " +
                          str(self.positions[cBlock + 1][0]) + " " + str(self.positions[cBlock + 1][1]) + " 0 ")

            svalue += (str(self.blockWidth) + " ")
            self.box_pos = np.zeros(3)
            #self.box_pos[2] = 15

        # update the score
        distanceHandToBlock = math.sqrt((self.box_pos[0] - self.hand_pos[0]) *
                                        (self.box_pos[0] - self.hand_pos[0]) +
                                        (self.box_pos[1] - self.hand_pos[1]) *
                                        (self.box_pos[1] - self.hand_pos[1]) +
                                        (self.box_pos[2] - self.hand_pos[2]) *
                                        (self.box_pos[2] - self.hand_pos[2]))

        #print "********************************"
        #print "********************************"
        #print "distanceHandToBlock: ", distanceHandToBlock
        #print "********************************"

        self.isAtTarget = 0
        if not self.pause:
            if distanceHandToBlock < (self.blockWidth / 100.0):
                if cBlock > -1:
                    if self.scoreRefTime > 0.0:
                        self.score += config.scoreIncrement * (time.time() - self.scoreRefTime)
                        self.isAtTarget = 1
                        #print 'scoring'
        if cBlock > -1:
            self.scoreRefTime = time.time()
            if self.scorePlotRefTime > 0.0:
                if self.pause:
                    self.scorePlotRefTime = time.time()
                    self.scorePrevPlot = self.score
                if (time.time() - self.scorePlotRefTime) > 60.0:
                    self.scorePlot = (self.score - self.scorePrevPlot) / 60.0
                    self.scorePrevPlot = self.score
                    self.scorePlotRefTime = time.time()
                else:
                    self.scorePlot = -1
            else:
                self.scorePlotRefTime = time.time()

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
                   str(self.scorePlot) + " " +
                   str(self.sublevel) + " " +
                   str(self.trial)) + " "
        svalue += (str(self.isAtTarget))
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
                    self.hand_pos[0] = 0.5

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
        self.paused = False
        self.index = 0

    def serialize(self):
        if self.paused:
            svalue = "paused"
            return svalue
        #self.index += 1
        #if (self.index % 100 == 0):
        #    self.target_pos[0] = random.uniform(0.0, 1.0)
        #    self.target_pos[1] = random.uniform(0.0, 1.0)
        #    self.target_pos[2] = random.uniform(0.0, 1.0)
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
