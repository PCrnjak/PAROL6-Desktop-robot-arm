# This file acts as configuration file for robot you are using
# It works in conjustion with configuration file from robotics toolbox

from swift import Swift
import spatialmath.base.symbolic as sym
from roboticstoolbox import ETS as ET
from roboticstoolbox import *
import roboticstoolbox as rtb
from spatialmath import *
from spatialgeometry import *
from math import pi
import numpy as np
import time
import random

Joint_num = 6 # Number of joints
Microstep = 32
steps_per_revolution=200
degree_per_step_constant = 360/(32*200) 
radian_per_step_constant = (2*pi) / (32*200)
radian_per_sec_2_deg_per_sec_const = 360/ (2*np.pi)
deg_per_sec_2_radian_per_sec_const = (2*np.pi) / 360

# robot length values (metres)
a1 = 110.50 / 1000
a2 = 23.42 / 1000
a3 = 180 / 1000
a4 = 43.5 / 1000
a5 = 176.35 / 1000
a6 = 62.8 / 1000
a7 = 45.25 / 1000

alpha_DH = [-pi / 2,pi,pi/2,-pi/2,pi/2,pi]

robot = DHRobot(
    [
        RevoluteDH(d=a1, a=a2, alpha=alpha_DH[0]),
        RevoluteDH(a=a3,d = 0,alpha=alpha_DH[1]),
        RevoluteDH(alpha= alpha_DH[2], a= -a4),
        RevoluteDH(d=-a5, a=0, alpha=alpha_DH[3]),
        RevoluteDH(a=0,d=0,alpha=alpha_DH[4]),
        RevoluteDH(alpha=alpha_DH[5], a = -a7,d = -a6),
    ],
    name="PAROL6",
)
print(robot.isspherical())
#pyplot = rtb.backends.PyPlot()

# in degrees
Joints_standby_position_degree = [0,-90,180,0,0,180] 
# in radians
Joints_standby_position_radian = [np.deg2rad(angle) for angle in Joints_standby_position_degree]

# values you get after homing robot and moving it to its most left and right sides
# In degrees
Joint_limits_degree =[[-123.046875,123.046875], [-145.0088,-3.375], [107.866,287.8675], [-105.46975,105.46975], [-90,90], [-100000,100000]] 
# in radians
Joint_limits_radian = []
for limits in Joint_limits_degree:
    radian_limits = [np.deg2rad(angle) for angle in limits]
    Joint_limits_radian.append(radian_limits)

# Reduction ratio we have on our joints
Joint_reduction_ratio = [6.4, 20, 20*(38/42) , 4, 4, 10] 

Joint_max_jog_speed = [1.57075, 1.57075, 1.57075, 1.57075, 1.57075, 1.57075]
Joint_min_jog_speed = [1.57075, 1.57075, 1.57075, 1.57075, 1.57075, 1.57075]

Joint_max_speed = [1.57075, 1.57075, 1.57075, 1.57075, 1.57075, 1.57075] # max speed in RAD/S used, can go to much more than 1.57 but there is bug in S-drive firmware
Joint_min_speed = [0.02617993875, 0.02617993875, 0.02617993875,0.02617993875, 0.02617993875, 0.02617993875] # min speed in RAD/S used 

Joint_max_acc = [5, 5, 5, 5, 5, 5] # max acceleration in RAD/S²
Joint_min_acc = [0.1, 0.1, 0.1, 0.1, 0.1, 0.1] # min acceleration in RAD/S²

# 360 / (200 * 32) = 0.05625
def DEG2STEPS(Degrees, index):
    Steps = Degrees / degree_per_step_constant * Joint_reduction_ratio[index]
    return Steps

def STEPS2DEG(Steps,index):
    Degrees = Steps * degree_per_step_constant / Joint_reduction_ratio[index]
    return Degrees

def RAD2STEPS(Rads,index):
    deg = np.rad2deg(Rads)
    steps = DEG2STEPS(deg,index)
    return steps

def STEPS2RADS(Steps,index):
    deg = STEPS2DEG(Steps,index)
    rads = np.deg2rad(deg)
    return rads

def RAD2DEG(radian):
    return np.rad2deg(radian)

def DEG2RAD(degree):
    return np.deg2rad(degree)

def SPEED_STEPS2DEG(Steps_per_second,index):

    '''     Transform true RADS/S to true RPM.
    Both these values are true values at witch motor joints move '''

    degrees_per_step = degree_per_step_constant / Joint_reduction_ratio[index]
    degrees_per_second = Steps_per_second * degrees_per_step
    return degrees_per_second

def SPEED_DEG2STEPS(Deg_per_second,index):
    steps_per_second = Deg_per_second / degree_per_step_constant * Joint_reduction_ratio[index]
    return steps_per_second

def SPEED_STEP2RAD(Steps_per_second,index):
    degrees_per_step = radian_per_step_constant / Joint_reduction_ratio[index]
    rad_per_second = Steps_per_second * degrees_per_step
    return rad_per_second

def SPEED_RAD2STEP(Rad_per_second,index):
    steps_per_second = Rad_per_second / radian_per_step_constant * Joint_reduction_ratio[index]
    return steps_per_second

def RAD_SEC_2_DEG_SEC(rad_per_sec):
    return rad_per_sec * radian_per_sec_2_deg_per_sec_const

def DEG_SEC_2_RAD_SEC(deg_per_sec):
    return deg_per_sec * deg_per_sec_2_radian_per_sec_const


if __name__ == "__main__":
    print(DEG2STEPS(180,2))
    print(STEPS2DEG(57905,2))
    print(RAD2STEPS(pi,5))
    print(STEPS2RADS(32000,5))
    print(SPEED_STEPS2DEG(1000,5))
    print(SPEED_STEP2RAD(1000,5))
    print(Joint_limits_radian)
    print(Joints_standby_position_radian)
    robot.ikine_LMS()

