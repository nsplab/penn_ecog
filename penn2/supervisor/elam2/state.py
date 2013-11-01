import numpy as np
import threading

# 0: training, 1: test
# used to tell the filter module the type of the current trial
triainingTestSeq = np.array([0, 0, 0, 0, 0, 1])


class GameState(object):

    def __init__(self, hand_pos=np.zeros(3), ball_pos=np.zeros(3),
                 box_pos=np.zeros(3), score=0, level=1, sublevel=1):
        self.hand_pos = hand_pos
        self.ball_pos = ball_pos
        self.box_pos = box_pos
        self.closed_hand = 0
        self.score = score
        self.level = level
        self.sublevel = sublevel
        self.trial = 1
        self.timeThread = None
        self.ballTimerStarted = False
        self.holdTime = 2.0  # seconds
        self.numTrials = 3
        self.numSublevels = 2
        self.pickedBall = False

    # convert the game state to ascii strin to be sent to the graphics module
    def serialize(self):
        svalue = (str(self.hand_pos[0]) + " " +
                  str(self.hand_pos[1]) + " " +
                  str(self.hand_pos[2]) + " ")
        svalue += (str(self.ball_pos[0]) + " " +
                   str(self.ball_pos[1]) + " " +
                   str(self.ball_pos[2]) + " ")
        svalue += (str(self.box_pos[0]) + " " +
                   str(self.box_pos[1]) + " " +
                   str(self.box_pos[2]) + " ")
        svalue += (str(self.closed_hand) + " " +
                   str(self.score) + " " +
                   str(self.level) + " " +
                   str(self.sublevel) + " " +
                   str(self.trial))
        return svalue

    # close the hand and pick up the ball
    def pickBall(self):
        print "function"
        self.closed_hand = 1
        self.timeThread.cancel()
        self.ballTimerStarted = False
        self.pickedBall = True

    def updateState(self):
        ret = False
        # pick up ball
        if self.closed_hand == 1:
            self.ball_pos = self.hand_pos
        elif np.linalg.norm(self.hand_pos - self.ball_pos) < 0.8:
            if self.ballTimerStarted is False:
                self.timeThread = threading.Timer(self.holdTime, self.pickBall)
                self.ballTimerStarted = True
                self.timeThread.start()
        elif self.ballTimerStarted:
            self.ballTimerStarted = False
            self.timeThread.cancel()
            self.closed_hand = 0

        # drop ball
        if self.closed_hand == 1:
            if np.linalg.norm(self.hand_pos - self.box_pos) < 0.5:
                print "drop the ball"
                self.closed_hand = 0
                self.score += 10
                ret = True

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
