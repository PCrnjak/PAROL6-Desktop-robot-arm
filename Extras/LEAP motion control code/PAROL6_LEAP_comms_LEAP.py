# A stripped down commander software code
# You can write your own functions here easily and communicate
# with other  scripts on your PC or local network via UDP
# In this example you can perform some actions via keyboard:
# * Pressing h will home the robot
# * Pressing e will enable robot
# * If you press estop robot will stop and you need to enable it by pressing e

from roboticstoolbox import DHRobot, RevoluteDH, ERobot, ELink, ETS
from math import pi, sin, cos
import numpy as np
from oclock import Timer, loop, interactiveloop
import time
import socket
import select
import serial
import platform
import os
import logging
import struct
import keyboard
import roboticstoolbox
from spatialmath import *


# Define the window size for the moving average filter
WINDOW_SIZE = 5

# Initialize lists for storing recent cart_data values
cart_data_history = [[] for _ in range(6)]

# Function to calculate moving average
def moving_average(values):
    return sum(values) / len(values)

cart_data_filter = np.array([0.0, 0.0, 0.0, 0.0, 0.0, 0.0])  

data_file = open("motion_data.txt", "a")

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

q_t1 = np.array([np.pi / 2, -5 * np.pi / 12, 31 * np.pi / 36, 0, -np.pi / 3, np.pi])
q_t1 = np.array([np.pi / 3, -5 * np.pi / 12, 31 * np.pi / 36, 0, -np.pi / 3, np.pi])

# Set interval
INTERVAL_S = 0.01
prev_time = 0

logging.basicConfig(level = logging.DEBUG,
    format='%(asctime)s.%(msecs)03d %(levelname)s:\t%(message)s',
    datefmt='%H:%M:%S'
)
logging.disable(logging.DEBUG)


my_os = platform.system()
if my_os == "Windows": 
    STARTING_PORT = 58 # COM3
str_port = ''

if my_os == "Windows":
    try:
        str_port = 'COM' + str(STARTING_PORT)
        ser = serial.Serial(port=str_port, baudrate=3000000, timeout=0)
    except:
        ser = serial.Serial()

# in big endian machines, first byte of binary representation of the multibyte data-type is stored first. 
int_to_3_bytes = struct.Struct('>I').pack # BIG endian order

# data for output string (data that is being sent to the robot)
#######################################################################################
#######################################################################################
start_bytes =  [0xff,0xff,0xff] 
start_bytes = bytes(start_bytes)

end_bytes =  [0x01,0x02] 
end_bytes = bytes(end_bytes)


# data for input string (Data that is being sent by the robot)
#######################################################################################
#######################################################################################
input_byte = 0 # Here save incoming bytes from serial

start_cond1_byte = bytes([0xff])
start_cond2_byte = bytes([0xff])
start_cond3_byte = bytes([0xff])

end_cond1_byte = bytes([0x01])
end_cond2_byte = bytes([0x02])

start_cond1 = 0 #Flag if start_cond1_byte is received
start_cond2 = 0 #Flag if start_cond2_byte is received
start_cond3 = 0 #Flag if start_cond3_byte is received

good_start = 0 #Flag if we got all 3 start condition bytes
data_len = 0 #Length of the data after -3 start condition bytes and length byte, so -4 bytes

data_buffer = [None]*255 #Here save all data after data length byte
data_counter = 0 #Data counter for incoming bytes; compared to data length to see if we have correct length
#######################################################################################
#######################################################################################
prev_positions = [0,0,0,0,0,0]

#######################################################################################
#######################################################################################
Position_out = [1,11,111,1111,11111,10]
Speed_out = [2,21,22,23,24,25]
Command_out = 255
Affected_joint_out = [1,1,1,1,1,1,1,1]
InOut_out = [0,0,0,0,0,0,0,0]
Timeout_out = 0
#Positon,speed,current,command,mode,ID
Gripper_data_out = [1,1,1,1,0,0]
#######################################################################################
#######################################################################################
# Data sent from robot to PC
Position_in = [31,32,33,34,35,36]
Speed_in = [41,42,43,44,45,46]
Homed_in = [1,1,1,1,1,1,1,1]
InOut_in = [1,1,1,1,1,1,1,1]
Temperature_error_in = [1,1,1,1,1,1,1,1]
Position_error_in = [1,1,1,1,1,1,1,1]
Timeout_error = 0
# how much time passed between 2 sent commands (2byte value, last 2 digits are decimal so max value is 655.35ms?)
Timing_data_in = [0]
XTR_data =   0

#ID,Position,speed,current,status,obj_detection
Gripper_data_in = [1,1,1,1,1,1] 

qx3 = 0
qx3_STEPS = np.array([0,0,0,0,0,0])
qx3_d = np.array([0,0,0,0,0,0])
jtraj_command = 0
starter_position = np.array([0., -1.939, 2.348, -0., 0.426, 3.142])
jtraj_step = 0
executing_jtraj = 0
exec_time = (np.arange(0,5,0.01))
teleop_command = 0

prev_q1 = np.array([0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
speed_calc = np.array([0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
speed_calc_motor = np.array([0, 0, 0, 0, 0, 0])
speed_calc_motor_filter = np.array([0, 0, 0, 0, 0, 0])
q_motor = np.array([0, 0, 0, 0, 0, 0])


#Setup IP address and Simulator port
ip = "127.0.0.1" #Loopback address
port = 5001
# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((ip, port))
print(f'Start listening to {ip}:{port}')

def home_robot_key():
    global Command_out
    Command_out = 100
    print("Home robot")

def start_position():
    global jtraj_command
    jtraj_command = 1
    print("Go to start position")

def enable_robot():
    global Command_out
    Command_out = 101
    print("Enable robot")

def teleop_robot():
    global teleop_command
    global Command_out
    teleop_command = 1
    #Command_out = 123
    print("teleop robot")


keyboard.add_hotkey('h', home_robot_key)
keyboard.add_hotkey('j', start_position)
keyboard.add_hotkey('e', enable_robot)
keyboard.add_hotkey('t', teleop_robot)


Microstep = 32
steps_per_revolution=200
degree_per_step_constant = 360/(32*200) 
radian_per_step_constant = (2*pi) / (32*200)
radian_per_sec_2_deg_per_sec_const = 360/ (2*np.pi)
deg_per_sec_2_radian_per_sec_const = (2*np.pi) / 360
Joint_reduction_ratio = [6.4, 20, 20*(38/42) , 4, 4, 10] 

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
    Both these values are true values at witch MOTORS SPIN  '''

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


def Unpack_data(data_buffer_list, Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in):

    Joints = []
    Speed = []

    for i in range(0,18, 3):
        variable = data_buffer_list[i:i+3] 
        Joints.append(variable)

    for i in range(18,36, 3):
        variable = data_buffer_list[i:i+3]
        Speed.append(variable)


    for i in range(6):
        var =  b'\x00' + b''.join(Joints[i]) 
        Position_in[i] = Fuse_3_bytes(var)
        var =  b'\x00' + b''.join(Speed[i]) 
        Speed_in[i] = Fuse_3_bytes(var)
    
    Homed = data_buffer_list[36]
    IO_var = data_buffer_list[37]
    temp_error = data_buffer_list[38]
    position_error = data_buffer_list[39]
    timing_data = data_buffer_list[40:42]
    Timeout_error_var = data_buffer_list[42]
    xtr2 = data_buffer_list[43]
    device_ID = data_buffer_list[44]
    Gripper_position = data_buffer_list[45:47]
    Gripper_speed = data_buffer_list[47:49]
    Gripper_current = data_buffer_list[49:51]
    Status = data_buffer_list[51]
    object_detection = data_buffer_list[52]
    CRC_byte = data_buffer_list[53]
    endy_byte1 = data_buffer_list[54]
    endy_byte2 = data_buffer_list[55]

    logging.debug("Robot position")
    logging.debug(Joints)
    logging.debug("Robot speed")
    logging.debug(Speed)
    logging.debug("Robot homed")
    logging.debug(Homed)

    temp = Split_2_bitfield(int.from_bytes(Homed,"big"))
    for i in range(8):
        Homed_in[i] = temp[i]

    logging.debug("Robot I/O data")
    logging.debug(IO_var)

    temp = Split_2_bitfield(int.from_bytes(IO_var,"big"))
    for i in range(8):
        InOut_in[i] = temp[i]

    logging.debug("Robot temp error data")
    logging.debug(temp_error)

    temp = Split_2_bitfield(int.from_bytes(temp_error,"big"))
    for i in range(8):
        Temperature_error_in[i] = temp[i]

    logging.debug("Robot position error data")
    logging.debug(position_error)

    temp = Split_2_bitfield(int.from_bytes(position_error,"big"))
    for i in range(8):
        Position_error_in[i] = temp[i]

    logging.debug("Robot timig data")
    logging.debug(timing_data)
    logging.debug("Robot timig data fused")
    var = b'\x00' + b'\x00' + b''.join(timing_data)
    logging.debug(var)
    logging.debug("Robot timig data fused 2")
    var2 = Fuse_3_bytes(var)
    Timing_data_in[0] = var2
    logging.debug(Timing_data_in[0])
    logging.debug(var2)
    logging.debug("Timing in ms")
    logging.debug(var2 * 1.4222222e-6)
    logging.debug(var2)
    logging.debug("Robot timig error data")
    logging.debug(Timeout_error_var)

    Timeout_error = int.from_bytes(Timeout_error_var,"big")

    logging.debug("Robot additional byte 2")
    logging.debug(xtr2)

    XTR_data = int.from_bytes(xtr2,"big")

    logging.debug("Gripper device ID")
    logging.debug(device_ID)

    Gripper_data_in[0] = int.from_bytes(device_ID,"big") 

    logging.debug("Gripper position")
    logging.debug(Gripper_position)

    var =  b'\x00'+ b'\x00' + b''.join(Gripper_position) 
    Gripper_data_in[1] = Fuse_2_bytes(var)

    logging.debug("Gripper speed")
    logging.debug(Gripper_speed)


    var =  b'\x00'+ b'\x00' + b''.join(Gripper_speed) 
    Gripper_data_in[2] = Fuse_2_bytes(var)

    logging.debug("Gripper current")
    logging.debug(Gripper_current)


    var =  b'\x00'+ b'\x00' + b''.join(Gripper_current) 
    Gripper_data_in[3] = Fuse_2_bytes(var)

    logging.debug("Gripper status")
    logging.debug(Status)

    Gripper_data_in[4] = int.from_bytes(Status,"big")

    logging.debug("Gripper object detection")
    logging.debug(object_detection)

    Gripper_data_in[5] = int.from_bytes(object_detection,"big")

    logging.debug("CRC byte")
    logging.debug(CRC_byte)
    logging.debug("End byte 1")
    logging.debug(endy_byte1)
    logging.debug("End byte 2")
    logging.debug(endy_byte2)




        # Data we send to the robot


def Pack_data(Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out):

    # Len is defined by all bytes EXCEPT start bytes and len
    # Start bytes = 3
    len = 52 #1
    Position = [Position_out[0],Position_out[1],Position_out[2],Position_out[3],Position_out[4],Position_out[5]]  #18
    Speed = [Speed_out[0], Speed_out[1], Speed_out[2], Speed_out[3], Speed_out[4], Speed_out[5],] #18
    Command = Command_out#1
    Affected_joint = Affected_joint_out
    InOut = InOut_out #1
    Timeout = Timeout_out #1
    Gripper_data = Gripper_data_out #9
    CRC_byte = 228 #1
    # End bytes = 2


    test_list = []
    #print(test_list)

    #x = bytes(start_bytes)
    test_list.append((start_bytes))
    
    test_list.append(bytes([len]))


    # Position data
    for i in range(6):
        position_split = Split_2_3_bytes(Position[i])
        test_list.append(position_split[1:4])

    # Speed data
    for i in range(6):
        speed_split = Split_2_3_bytes(Speed[i])
        test_list.append(speed_split[1:4])

    # Command data
    test_list.append(bytes([Command]))

    # Affected joint data
    Affected_list = Fuse_bitfield_2_bytearray(Affected_joint[:])
    test_list.append(Affected_list)

    # Inputs outputs data
    InOut_list = Fuse_bitfield_2_bytearray(InOut[:])
    test_list.append(InOut_list)

    # Timeout data
    test_list.append(bytes([Timeout]))

    # Gripper position
    Gripper_position = Split_2_3_bytes(Gripper_data[0])
    test_list.append(Gripper_position[2:4])

    # Gripper speed
    Gripper_speed = Split_2_3_bytes(Gripper_data[1])
    test_list.append(Gripper_speed[2:4])

    # Gripper current
    Gripper_current = Split_2_3_bytes(Gripper_data[2])
    test_list.append(Gripper_current[2:4])  

    # Gripper command
    test_list.append(bytes([Gripper_data[3]]))
    # Gripper mode
    test_list.append(bytes([Gripper_data[4]]))
    # Gripper ID
    test_list.append(bytes([Gripper_data[5]]))
 
    # CRC byte
    test_list.append(bytes([CRC_byte]))

    # END bytes
    test_list.append((end_bytes))
    
    #print(test_list)
    return test_list




def Get_data(Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in):
    global input_byte 

    global start_cond1_byte 
    global start_cond2_byte 
    global start_cond3_byte 

    global end_cond1_byte 
    global end_cond2_byte 

    global start_cond1 
    global start_cond2 
    global start_cond3 

    global good_start 
    global data_len 

    global data_buffer 
    global data_counter

    while (ser.inWaiting() > 0):
        input_byte = ser.read()

        #UNCOMMENT THIS TO GET ALL DATA FROM THE ROBOT PRINTED
        #print(input_byte) 

        # When data len is received start is good and after that put all data in receive buffer
        # Data len is ALL data after it; that includes input buffer, end bytes and CRC
        if (good_start != 1):
            # All start bytes are good and next byte is data len
            if (start_cond1 == 1 and start_cond2 == 1 and start_cond3 == 1):
                good_start = 1
                data_len = input_byte
                data_len = struct.unpack('B', data_len)[0]
                logging.debug("data len we got from robot packet= ")
                logging.debug(input_byte)
                logging.debug("good start for DATA that we received at PC")
            # Third start byte is good
            if (input_byte == start_cond3_byte and start_cond2 == 1 and start_cond1 == 1):
                start_cond3 = 1
                #print("good cond 3 PC")
            #Third start byte is bad, reset all flags
            elif (start_cond2 == 1 and start_cond1 == 1):
                #print("bad cond 3 PC")
                start_cond1 = 0
                start_cond2 = 0
            # Second start byte is good
            if (input_byte == start_cond2_byte and start_cond1 == 1):
                start_cond2 = 1
                #print("good cond 2 PC ")
            #Second start byte is bad, reset all flags   
            elif (start_cond1 == 1):
                #print("Bad cond 2 PC")
                start_cond1 = 0
            # First start byte is good
            if (input_byte == start_cond1_byte):
                start_cond1 = 1
                #print("good cond 1 PC")
        else:
            # Here data goes after good  start
            data_buffer[data_counter] = input_byte
            if (data_counter == data_len - 1):

                logging.debug("Data len PC")
                logging.debug(data_len)
                logging.debug("End bytes are:")
                logging.debug(data_buffer[data_len -1])
                logging.debug(data_buffer[data_len -2])

                # Here if last 2 bytes are end condition bytes we process the data 
                if (data_buffer[data_len -1] == end_cond2_byte and data_buffer[data_len - 2] == end_cond1_byte):

                    logging.debug("GOOD END CONDITION PC")
                    logging.debug("I UNPACKED RAW DATA RECEIVED FROM THE ROBOT")
                    Unpack_data(data_buffer, Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
                    XTR_data,Gripper_data_in)
                    logging.debug("DATA UNPACK FINISHED")
                    # ako su dobri izračunaj crc
                    # if crc dobar raspakiraj podatke
                    # ako je dobar paket je dobar i spremi ga u nove variable!
                
                # Print every byte
                #print("podaci u data bufferu su:")
                #for i in range(data_len):
                    #print(data_buffer[i])

                good_start = 0
                start_cond1 = 0
                start_cond3 = 0
                start_cond2 = 0
                data_len = 0
                data_counter = 0
            else:
                data_counter = data_counter + 1



# Split data to 3 bytes 
def Split_2_3_bytes(var_in):
    y = int_to_3_bytes(var_in & 0xFFFFFF) # converts my int value to bytes array
    return y

# Splits byte to bitfield list
def Split_2_bitfield(var_in):
    #return [var_in >> i & 1 for i in range(7,-1,-1)] 
    return [(var_in >> i) & 1 for i in range(7, -1, -1)]

# Fuses 3 bytes to 1 signed int
def Fuse_3_bytes(var_in):
    value = struct.unpack(">I", bytearray(var_in))[0] # converts bytes array to int

    # convert to negative number if it is negative
    if value >= 1<<23:
        value -= 1<<24

    return value

# Fuses 2 bytes to 1 signed int
def Fuse_2_bytes(var_in):
    value = struct.unpack(">I", bytearray(var_in))[0] # converts bytes array to int

    # convert to negative number if it is negative
    if value >= 1<<15:
        value -= 1<<16

    return value

# Fuse bitfield list to byte
def Fuse_bitfield_2_bytearray(var_in):
    number = 0
    for b in var_in:
        number = (2 * number) + b
    return bytes([number])

# Check if there is element 1 in the list. 
# If yes return its index, if no element is 1 return -1
def check_elements(lst):
    for i, element in enumerate(lst):
        if element == 1:
            return i
    return -1  # Return -1 if no element is 1

previous_angle = 0 # used for j6 wrap


# Set interval
timer = Timer(interval=INTERVAL_S, warnings=False, precise=True)
while timer.elapsed_time < 1100000:
    
    time1 = time.perf_counter()
    #print(f"Loop time is: {time1-prev_time}")
    #print(f"Command out is {Command_out}")
    #print(f"inout is: {InOut_in}")
    #print(f"timing data in is: {Timing_data_in[0]* 1.4222222e-6 }")

    prev_time = time1

    if ser.is_open == True:


        s = Pack_data(Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out)
        len_ = len(s)
        try:
            for i in range(len_):
                ser.write(s[i])
        except:
            logging.debug("NO SERIAL TASK1")

        Get_data(Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
            XTR_data,Gripper_data_in)
        

        if(InOut_in[4] != 0): # if estop is not pressed
            # 123 is jog 156 is GO2POS
            if(executing_jtraj == 1):
                if jtraj_step < int(len(exec_time)):

                    for i in range(6):
                        qx3_STEPS[i] = RAD2STEPS(qx3.q[jtraj_step,i],i)
                        qx3_d[i] = SPEED_RAD2STEP(qx3.qd[jtraj_step,i],i)

                    Command_out = 156
                    Position_out = [qx3_STEPS[0],qx3_STEPS[1],qx3_STEPS[2],qx3_STEPS[3],qx3_STEPS[4],qx3_STEPS[5]]
                    Speed_out = [qx3_d[0],qx3_d[1],qx3_d[2],qx3_d[3],qx3_d[4],qx3_d[5]]

                    jtraj_step = jtraj_step + 1
                    print(f"jtraj step = {jtraj_step}, jtraj_send = {qx3_STEPS}")
                else: 
                    executing_jtraj = 0


            if(jtraj_command == 1):
                jtraj_command = 0
                pos_radians = np.array([0.0,0.0,0.0,0.0,0.0,0.0])
                for i in range(6):
                    pos_radians[i] = STEPS2RADS(Position_in[i],i)
                qx3 = roboticstoolbox.tools.trajectory.jtraj(pos_radians,starter_position,exec_time) # use array of time intervals
                print(f"position radians is: {pos_radians}")
                executing_jtraj = 1

            if(teleop_command == 1):
                Command_out = 156
                Position_out = [q_motor[0],q_motor[1],q_motor[2],q_motor[3],q_motor[4],Position_in[5]]  #q_motor[5]
                Speed_out = [speed_calc_motor[0],speed_calc_motor[1],speed_calc_motor[2],speed_calc_motor[3],speed_calc_motor[4],speed_calc_motor[5]]
                #print(f"speed is: {speed_calc_motor_filter}")
                #print(f"speed is: {speed_calc_motor}")
                #print(f"Position is: {q_motor}")


        # Safety stuff
        if(Command_out == 100):
            Command_out = 255

        if(InOut_in[4] == 0):
            Command_out = 102
            for i in range(6):
                Position_out[i] = Position_in[i]
                Speed_out[i] = 0



    else:
        try:
            if my_os == 'Linux':
                com_port = '/dev/ttyACM' + str(STARTING_PORT)
            elif my_os == 'Windows':
                com_port = 'COM' + str(STARTING_PORT)
                
            #print(com_port)
            ser.port = com_port
            ser.baudrate = 3000000
            ser.close()
            time.sleep(0.01)
            ser.open()
            time.sleep(0.01)
        except:
            time.sleep(0.01)


    ############################################################
    # Get data from UDP
    data = None
    while True:
        try:
            # Check if the socket is ready to read
            ready_to_read, _, _ = select.select([sock], [], [], 0)  # Timeout of 0 second, no blocking read instantly
            #print(ready_to_read)
            # Check if there's data available to read
            if sock in ready_to_read:
                # Receive data from the socket
                data, addr = sock.recvfrom(1024)  # data needs to be decoded                    
            else:
                #print(f"No sock in ready")
                break
        except KeyboardInterrupt:
            # Handle keyboard interrupt
            break
        except Exception as e:
            # Handle other exceptions
            print(f"Error: {e}")
            break

    if data:
        msg = data.decode()
        
        # Unpack the data
        #parts = msg.split(',')
        if msg.startswith("cart,"):
            _, x, y, z, roll, pitch, jaw, hand_status = msg.split(",")
            cart_data = np.array([float(x), float(y), float(z), float(roll), float(pitch), float(jaw)])  #0, pi/2, pi kako mora biti; 0, 0, pi is good

            """
                # Construct a matrix from given arguments, this will be needed pose
            Needed_pose = SE3.RPY([cart_data[3], cart_data[4], cart_data[5]], unit='rad',order='xyz')
            Needed_pose.t[0] = cart_data[0] 
            Needed_pose.t[1] = cart_data[1]   
            Needed_pose.t[2] = cart_data[2] 
            """
            InOut_out[2] = int(hand_status)
            InOut_out[3] = int(hand_status)
            print(InOut_out)
             # Apply moving average filter to cart_data
            for i in range(6):
                cart_data_history[i].append(cart_data[i])
                if len(cart_data_history[i]) > WINDOW_SIZE:
                    cart_data_history[i] = cart_data_history[i][-WINDOW_SIZE:]
                cart_data_filter[i] = moving_average(cart_data_history[i])

            # Construct a matrix from given arguments, this will be needed pose
            Needed_pose = SE3.RPY([cart_data_filter[3], cart_data_filter[4], cart_data_filter[5]], unit='rad',order='xyz')
            Needed_pose.t[0] = cart_data_filter[0] 
            Needed_pose.t[1] = cart_data_filter[1]   
            Needed_pose.t[2] = cart_data_filter[2] 


        q1 = robot.ik_LM(Needed_pose,method = "chan",q0 = q_t1, ilimit=25, slimit=25)
        position_in_data = q1[0]

        def unwrap_angle(angle, previous_angle):
            """
            Unwraps the given angle to prevent discontinuities.
            
            Parameters:
            angle (float): The current joint angle.
            previous_angle (float): The previous joint angle.
            
            Returns:
            float: The unwrapped angle.
            """
            while angle - previous_angle > np.pi:
                angle -= 2 * np.pi
            while angle - previous_angle < -np.pi:
                angle += 2 * np.pi
            return angle


        unwrapped_angle = unwrap_angle(position_in_data[5], previous_angle)
        previous_angle = unwrapped_angle
        #print(unwrapped_angle)

        #print(f"Joint data is {position_in_data[0]}")
        #print(f"Cart data is {cart_data}")

        q_motor[0] = RAD2STEPS(q1[0][0],0)
        q_motor[1] = RAD2STEPS(q1[0][1],1)
        q_motor[2] = RAD2STEPS(q1[0][2],2)
        q_motor[3] = RAD2STEPS(q1[0][3],3)
        q_motor[4] = RAD2STEPS(q1[0][4],4)
        q_motor[5] = RAD2STEPS(q1[0][5],5)

        #print(f"Joint data is {q_motor[0]}")

        speed_calc_motor[0] = (q_motor[0] - RAD2STEPS(prev_q1[0],0)) / 0.01
        speed_calc_motor[1] = (q_motor[1] - RAD2STEPS(prev_q1[1],1)) / 0.01
        speed_calc_motor[2] = (q_motor[2] - RAD2STEPS(prev_q1[2],2)) / 0.01
        speed_calc_motor[3] = (q_motor[3] - RAD2STEPS(prev_q1[3],3)) / 0.01
        speed_calc_motor[4] = (q_motor[4] - RAD2STEPS(prev_q1[4],4)) / 0.01
        speed_calc_motor[5] = (q_motor[5] - RAD2STEPS(prev_q1[5],5)) / 0.01

        data_file.write(f"{q_motor[0]},{q_motor[1]},{q_motor[2]},{q_motor[3]},{q_motor[4]},{q_motor[5]}\n")
        #data_file.write(f"{q1[0][0]},{q1[0][1]},{q1[0][2]},{q1[0][3]},{q1[0][4]},{q1[0][5]}\n")


        prev_q1[0] = q1[0][0]
        prev_q1[1] = q1[0][1]
        prev_q1[2] = q1[0][2]
        prev_q1[3] = q1[0][3]
        prev_q1[4] = q1[0][4]
        prev_q1[5] = q1[0][5]

        #print(f"data is: {data}")
        #print(Needed_pose)



    timer.checkpt()


