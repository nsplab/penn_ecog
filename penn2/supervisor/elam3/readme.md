Data log format
===============
the log file is an ascii/text file, the columns are separated by white space.

column 1: timestamp send from data acquisition module (passed through the feature extractor and filter modules)
column 2: the time of the wall clock
column 3: the trial number, i.e., the number of the current target bar that the user is supposed to catch
column 4-6: x y z of target position
column 7-9: x y z of hand position
column 10: the continuos and accumulated score
column 11: the score per minute which is shown to the user
column 12: target bar width in percentage of the workspace
column 13: target bar length in seconds
column 14: workspace radius
column 15: whether to move the hand (1) to the initial/resting position after every even number of target bars passed or not (0)
 	   
column 16: the distance threshold on the distance between the hand and the target bar, distances smaller than this threshold increase the score (you should divide this by the workspace radius to get the percentage of the workspace for this parameter)
column 17-[17+n]: the n-dimensional feature vector 
column [18+n]-[18+n+m]: the m-dimensional filter parameter vector
