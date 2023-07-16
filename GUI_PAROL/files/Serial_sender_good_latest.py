from oclock import Timer, loop, interactiveloop
import time, random, threading
import multiprocessing
import serial
import time
import roboticstoolbox as rp
import struct
import logging
import GUI_PAROL_latest
import SIMULATOR_Robot
import PAROL6_ROBOT 

print("run this")
logging.basicConfig(level = logging.DEBUG,
    format='%(asctime)s.%(msecs)03d %(levelname)s:\t%(message)s',
    datefmt='%H:%M:%S'
)
logging.disable(logging.DEBUG)

try:
    ser = serial.Serial(port='COM8', baudrate=3000000, timeout=0)
    None
except:
    None
#ser.open()

# in big endian machines, first byte of binary representation of the multibyte data-type is stored first. 
int_to_3_bytes = struct.Struct('>I').pack # BIG endian order

# data for output string (data that is being sent to the robot)
#######################################################################################
#######################################################################################
start_bytes =  [0xff,0xff,0xff] 
start_bytes = bytes(start_bytes)

end_bytes =  [0x01,0x02] 
end_bytes = bytes(end_bytes)

# Data for testing
#######################################################################################

data_len_output = [0x05]
data_len_output = bytes(data_len_output)

test_data =  [0x9,0x4,0x5] 
test_data = bytes(test_data)
#######################################################################################


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

# Set interval
INVERVAL_S = 0.01

# Task for sending data every x ms and performing all calculations, kinematics GUI control logic...
def Task1(Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out,
         Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in,
        Joint_jog_buttons,Cart_jog_buttons,Jog_control,General_data,Buttons):
    timer = Timer(INVERVAL_S, warnings=False, precise=True)
    cnt = 0

    while timer.elapsed_time < 110000:

        if ser.is_open == True:
            logging.debug("Task 1 alive")
            logging.debug("Data that PC will send to the robot is: ")
            time1 = time.time()
            #s = Pack_data_test()
            # This function packs data that we will send to the robot
            s = Pack_data(Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out)
            logging.debug(s)
            logging.debug("END of data sent to the ROBOT")
            
            len_ = len(s)
            try:
                for i in range(len_):
                    ser.write(s[i])
            except:
                logging.debug("NO SERIAL TASK1")
                    # This function packs data that we will send to the robot
    
            time2 = time.time()

            cnt = cnt +1
            if(cnt == 300):
                print("elapsed time is: ",end= "")
                print(time2- time1)
                cnt = 0

            # Check if any of jog buttons is pressed
            result_joint_jog = check_elements(list(Joint_jog_buttons))
            result_cart_jog = check_elements(list(Cart_jog_buttons))


            if result_joint_jog != -1: # JOINT JOG (regular speed control) 0x123
                Command_out.value = 123
                # if real send to real
                # if sim send to sim 
                # if both send to both
                #print(result_joint_jog)

            elif result_cart_jog != -1: # CART JOG 0x123?
                Command_out.value = 123 #?
                #print(result_cart_jog)           

            elif Buttons[0] == 1: # HOME COMMAND 0x100
                Command_out.value = 100
                Buttons[0] = 0
            
            elif Buttons[1] == 1: # ENABLE COMMAND 0x101
                Command_out.value = 101 
                Buttons[1] = 0

            elif Buttons[2] == 1: # DISABLE COMMAND 0x102
                Command_out.value = 102
                Buttons[2] = 0

            elif Buttons[3] == 1: # CLEAR ERROR COMMAND 0x103
                Command_out.value = 103
                Buttons[3] = 0

            else: # If nothing else is done send dummy data 0x255
                dummy_data(Position_out,Speed_out,Command_out,Position_in)
            
        else:
            try:
                
                ser.port = 'COM8'
                ser.baudrate = 3000000
                ser.close()
                time.sleep(0.5)
                ser.open()
                time.sleep(0.5)
            except:
                time.sleep(0.5)
                logging.debug("no serial available, reconnecting!")   

        timer.checkpt()


def dummy_data(Position_out,Speed_out,Command_out,Position_in):
    Command_out.value = 255
    for i in range(6):
        Position_out[i] = Position_in[i]
        Speed_out[i] = 0


# Task that receives data and saves to the multi proc array
def Task2(Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in):
    while 1:

        #  PYTHON
        # https://pyserial.readthedocs.io/en/latest/pyserial_api.html#serial.Serial.in_waiting
        # https://stackoverflow.com/questions/17553543/pyserial-non-blocking-read-loop
        # inWaiting govori koliko imamo bytes u serial input bufferu.
        # Pošto šaljem serial bez pauze uvijek će biti nešto
        # time on čita dok ima nečekga; a to čita sa .read
        # .read prima kao parametar koliko bytes da čita i u tome loopa
        # pošto prima inwaiting pročitati će ih sve koji su trenutačno u bufferu
        # npr ako dolaze pre sporo neće ih biti u bufferu i kod ide dalje
        # i kada se opet vrati i vidi da nečega ima čita to


        # ARDUINO
        # https://forum.arduino.cc/t/sending-command-over-serial-64-bytes-128-bytes/121598/3
        # https://github.com/stm32duino/Arduino_Core_STM32/wiki/HardwareTimer-library
        # http://www.gammon.com.au/serial


        # ovako jako slično radi i od arduina
        # serial.available govori koliko imamo bytes u bufferu
        # serial.read čita samo JEDAN byte (tu je velika razlika jer moram sam onda spremati u buffer)
        # zato se stavi onaj while(serial.availabe) i onda u loopu ide serial.read i spremanje u buffer
        # kada se pojavi ili neki naš znak tipa /n ili duljina buffera parasa se to i gleda da li je dobro
        # isto kao i gore ako je data pre spor serial.available će javiti da nema ničega i idemo dalje 
        # javiti će to makar je tamo while petlja. ako bi bila if petlja onda bi očitao jedan i radio ostatak koda
        # pa se vratio pročitao jedan itd. tako bi možda pre sporo primali serial ako bi ostatak koda bio spor
        try:
            Get_data(Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in)
            #Get_data_old()
        except:
            try:
                
                ser.port = 'COM8'
                ser.baudrate = 3000000
                ser.close()
                time.sleep(0.5)
                ser.open()
                time.sleep(0.5)
            except:
                time.sleep(0.5)
                logging.debug("no serial available, reconnecting!")                    
        #Get_data_old()
        #print("Task 2 alive")
        #time.sleep(2)


# Treba mi bytes format za slanje, nije baš user readable je pretvori iz hex u ascii
# ako trebam gledati vrijednosti koristi hex() funkciju

# Dummy test task
# Best used to show data that we get from the robot and data we get from GUI
def Task3(Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out,
         Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in,
        Joint_jog_buttons,Cart_jog_buttons,Jog_control,General_data,Buttons):
    while(1):
        
        start_bytes =  [0xff,0xff,0xff] 
        print(start_bytes)
        start_bytes = bytes(start_bytes)
        print(start_bytes)
        a = Split_2_bitfield(123)
        print(a)
        b = Split_2_3_bytes(-235005) 
        print(b)
        print(hex(b[0]))
        print(hex(b[1]))
        print(hex(b[2]))
        print(bytes([b[0]]))
        print(bytes([b[1]]))
        print(bytes([b[2]]))
        print("fuesd")
        c = Fuse_3_bytes(b)
        print(c)
        d = Fuse_bitfield_2_bytearray(a)
        print(d)
        test_list = [10]*50
        elements = list(range(60))
        Unpack_data_test(elements)
        print("$$$$$$$$$$$$$$$$")
        Pack_data_test()
    	
        print("")

        print("ROBOT DATA: ")
        print("Position from robot is: ",end="")
        print(Position_in[:])
        print("Speed from robot is: ",end="")
        print(Speed_in[:])
        print("Robot homed status is: ",end="")
        print(Homed_in[:])
        print("Robot Input/Output status is: ",end="")
        print(InOut_in[:])
        print("Robot temperature error status is: ",end="")
        print(Temperature_error_in[:])
        print("Robot temperature error status is: ",end="")
        print(Position_error_in[:])
        print("Timeout_error is: ",end="")
        print(Timeout_error.value)
        print("Time between 2 commands raw is: ",end="")
        print(Timing_data_in.value)
        print("Time between 2 commands in ms is: ",end="")
        print(Timing_data_in.value*1.42222222e-6)
        print("XTR_DATA byte is: ",end="")
        print(XTR_data.value)
        print("Gripper ID is: ",end="")
        print(Gripper_data_in[0])
        print("Gripper position is: ",end="")
        print(Gripper_data_in[1])
        print("Gripper speed is: ",end="")
        print(Gripper_data_in[2])
        print("Gripper current is: ",end="")
        print(Gripper_data_in[3])
        print("Gripper status is: ",end="")
        print(Gripper_data_in[4])
        print("Gripper object detection is: ",end="")
        print(Gripper_data_in[5])

        print("")

        print("GUI DATA: ")
        print("Joint jog buttons: ",end="")
        print(list(Joint_jog_buttons))
        print("Cart jog buttons: ",end="")
        print(list(Cart_jog_buttons))
        print("Home button state:",end="")
        print(Buttons[0])
        print("Enable button state:",end="")
        print(Buttons[1])
        print("Disable button state:",end="")
        print(Buttons[2])
        print("Clear error button state:",end="")
        print(Buttons[3])
        print("Real robot state: ",end="")
        print(Buttons[4])
        print("Simulator robot state: ",end="")
        print(Buttons[5])



        time.sleep(2)






# Data that we receive from the robot
#Input is data buffer list
#Output is saved to multiproc arrays and variables
## joints(3byte)x6,speed(3byte)x6,homed(byte),I/O(byte),temp_error(byte),position_error(byte),timing_data(2byte),Timeout_error(byte),xtr2(byte)
# Gripper data == Position(2byte),speed(2byte),current(2byte),status(byte),obj_detection(byte),ID(byte)
## CRC(byte),end1(byte),end2(byte)
# Last 2 bytes are end bytes but we dont unpack then since we chech their validity elsewhere
def Unpack_data_test(data_buffer_list):

    Joints = []
    Speed = []

    for i in range(0,18, 3):
        variable = data_buffer_list[i:i+3]
        Joints.append(variable)

    for i in range(18,36, 3):
        variable = data_buffer_list[i:i+3]
        Speed.append(variable)
    
    Homed = data_buffer_list[36]
    IO_var = data_buffer_list[37]
    temp_error = data_buffer_list[38]
    position_error = data_buffer_list[39]
    timing_data = data_buffer_list[40:42]
    Timeout_error = data_buffer_list[42]
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
    logging.debug("Robot I/O data")
    logging.debug(IO_var)
    logging.debug("Robot temp error data")
    logging.debug(temp_error)
    logging.debug("Robot position error data")
    logging.debug(position_error)
    logging.debug("Robot timig data")
    logging.debug(timing_data)
    logging.debug("Robot timig error data")
    logging.debug(Timeout_error)
    logging.debug("Robot additional byte 2")
    logging.debug(xtr2)
    logging.debug("Gripper device ID")
    logging.debug(device_ID)
    logging.debug("Gripper position")
    logging.debug(Gripper_position)
    logging.debug("Gripper speed")
    logging.debug(Gripper_speed)
    logging.debug("Gripper current")
    logging.debug(Gripper_current)
    logging.debug("Gripper status")
    logging.debug(Status)
    logging.debug("Gripper object detection")
    logging.debug(object_detection)
    logging.debug("CRC byte")
    logging.debug(CRC_byte)
    logging.debug("End byte 1")
    logging.debug(endy_byte1)
    logging.debug("End byte 2")
    logging.debug(endy_byte2)


# Data that we receive from the robot
#Input is data buffer list
#Output is saved to multiproc arrays and variables
## joints(3byte)x6,speed(3byte)x6,homed(byte),I/O(byte),temp_error(byte),position_error(byte),timing_data(2byte),Timeout_error(byte),xtr2(byte)
# Gripper data == Position(2byte),speed(2byte),current(2byte),status(byte),obj_detection(byte),ID(byte)
## CRC(byte),end1(byte),end2(byte)
# Last 2 bytes are end bytes but we dont unpack then since we chech their validity elsewhere
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
    Timing_data_in.value = var2
    logging.debug(var2)
    logging.debug("Timing in ms")
    logging.debug(var2 * 1.4222222e-6)
    logging.debug(var2)
    logging.debug("Robot timig error data")
    logging.debug(Timeout_error_var)

    Timeout_error.value = int.from_bytes(Timeout_error_var,"big")

    logging.debug("Robot additional byte 2")
    logging.debug(xtr2)

    XTR_data.value = int.from_bytes(xtr2,"big")

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


# Just read data and print it
def Get_data_old():
    while (ser.inWaiting() > 0):
        data_str = ser.read(ser.inWaiting()) #.decode('utf-8') 
        print(data_str)
        print("\\+\\") 
        time.sleep(0.01)    



# Data we send to the robot
# Inputs are multiproc arrays and variables
# Outputs is list of bytes objects? that need to be send by the serial
# Position is needed robot position - it is list of 6 joint position elements and packs to 3 bytes each
# Speed is needed robot position - it is list of 6 joint speed elements and packs to 3 bytes each
# Commad is single byte
# Affected joint is single byte 
# InOut is byte where 
# Timeout is byte  
# Gripper data is list of gripper elements like speed, positon, ID...
# Positon packs to 2 bytes, speed to 2 bytes, current to 2 bytes, command to 1 byte, mode 1 byte, ID to 1  byte

# First 3 bytes are start condition 
# After that is data len
# After that is data buffer
# After that is CRC 
# After that is 2 end bytes
# Whole string is packed in one list of data

def Pack_data(Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out):

    # Len is defined by all bytes EXCEPT start bytes and len
    # Start bytes = 3
    len = 52 #1
    Position = [Position_out[0],Position_out[1],Position_out[2],Position_out[3],Position_out[4],Position_out[5]]  #18
    Speed = [Speed_out[0], Speed_out[1], Speed_out[2], Speed_out[3], Speed_out[4], Speed_out[5],] #18
    Command = Command_out.value#1
    Affected_joint = Affected_joint_out
    InOut = InOut_out #1
    Timeout = Timeout_out.value #1
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


# Data we send to the robot for testing
def Pack_data_test():

    # Len is defined by all bytes EXCEPT start bytes and len
    # Start bytes = 3
    len = 52 #1
    Position = [255,255,255,255,255,255]  #18
    Speed = [255,255,255,255,255,255]  #18
    Command = 123 #1 
    Affected_joint = [1,1,1,1,1,1,1,1] #1
    InOut = [0,0,0,0,0,0,0,0] #1
    Timeout = 247 #1
    Gripper_data = [-222,-223,-224,225,226,123]  #9
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
    Affected_list = Fuse_bitfield_2_bytearray(Affected_joint)
    test_list.append(Affected_list)

    # Inputs outputs data
    InOut_list = Fuse_bitfield_2_bytearray(InOut)
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
            # Here data goes after good start
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

def Main(Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out,
         Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in,
        Joint_jog_buttons,Cart_jog_buttons,Jog_control,General_data,Buttons,): 

    t1 = threading.Thread(target = Task1, args = (Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out,
         Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in,
        Joint_jog_buttons,Cart_jog_buttons,Jog_control,General_data,Buttons))
    
    t2 = threading.Thread(target = Task2, args = ( Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in,))
    
    t3 = threading.Thread(target = Task3,args = ( Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out,
         Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in,
        Joint_jog_buttons,Cart_jog_buttons,Jog_control,General_data,Buttons))

    t1.start()
    t2.start()
    t3.start()


def GUI_process(Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out,
         Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in,
        Joint_jog_buttons,Cart_jog_buttons,Jog_control,General_data,Buttons):

        GUI_PAROL_latest.GUI(Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out,
         Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in,
        Joint_jog_buttons,Cart_jog_buttons,Jog_control,General_data,Buttons)


def SIMULATOR_process(Position_out,Position_in,Position_Sim):
    SIMULATOR_Robot.GUI(Position_out,Position_in,Position_Sim)

# u PROCES kao argumenti idu multi proc arrays tu dolje u initi
# Gore u thredovima i funkcijama to nazovem kako oćem i pozivam stvari iz toga i tjt
if __name__ == '__main__':

    print("running")
    time.sleep(0.01)

    try:
        ser.close()
    except:
        None

    # Data sent by the PC to the robot
    Position_out = multiprocessing.Array("i",[1,11,111,1111,11111,10], lock=False) 
    Speed_out = multiprocessing.Array("i",[2,21,22,23,24,25], lock=False) 
    Command_out = multiprocessing.Value('i',0) 
    Affected_joint_out = multiprocessing.Array("i",[1,1,1,1,1,1,1,1], lock=False) 
    InOut_out = multiprocessing.Array("i",[1,1,1,1,1,1,1,1], lock=False)
    Timeout_out = multiprocessing.Value('i',0) 
    #Positon,speed,current,command,mode,ID
    Gripper_data_out = multiprocessing.Array("i",[1,1,1,1,1,1], lock=False)

    # Data sent from robot to PC
    Position_in = multiprocessing.Array("i",[31,32,33,34,35,36], lock=False) 
    Speed_in = multiprocessing.Array("i",[41,42,43,44,45,46], lock=False) 
    Homed_in = multiprocessing.Array("i",[1,1,1,1,1,1,1,1], lock=False) 
    InOut_in = multiprocessing.Array("i",[1,1,1,1,1,1,1,1], lock=False)
    Temperature_error_in = multiprocessing.Array("i",[1,1,1,1,1,1,1,1], lock=False) 
    Position_error_in = multiprocessing.Array("i",[1,1,1,1,1,1,1,1], lock=False) 
    Timeout_error = multiprocessing.Value('i',0) 
    # how much time passed between 2 sent commands (2byte value, last 2 digits are decimal so max value is 655.35ms?)
    Timing_data_in = multiprocessing.Value('i',0) 
    XTR_data =   multiprocessing.Value('i',0)

    #ID,Position,speed,current,status,obj_detection
    Gripper_data_in = multiprocessing.Array("i",[1,1,1,1,1,1], lock=False)  

    # GUI control data
    Homed_out = multiprocessing.Array("i",[1,1,1,1,1,1], lock=False) 

    #General robot vars
    Robot_GUI_mode =   multiprocessing.Value('i',0)

    # Robot jogging vars
    Joint_jog_buttons = multiprocessing.Array("i",[0,0,0,0,0,0,0,0,0,0,0,0], lock=False) 
    Cart_jog_buttons = multiprocessing.Array("i",[0,0,0,0,0,0,0,0,0,0,0,0], lock=False)

    # Joint jog/Cart jog, TRF/WRF, Jog velocity, Jog acceleration 
    Jog_control = multiprocessing.Array("i",[0,0,0,0], lock=False) 

    # COM PORT, BAUD RATE, 
    General_data =  multiprocessing.Array("i",[8,3000000], lock=False) 

    # Home,Enable,Disable,Clear error,Real_robot,Sim_robot
    Buttons =  multiprocessing.Array("i",[0,0,0,0,1,1], lock=False) 

    # Positions for robot simulator
    Position_Sim =  multiprocessing.Array("i",[0,0,0,0,0,0], lock=False) 

    # Process
    process1 = multiprocessing.Process(target=Main,args=[Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out,
         Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in,
        Joint_jog_buttons,Cart_jog_buttons,Jog_control,General_data,Buttons,])
    
    process2 = multiprocessing.Process(target=GUI_process,args=[Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out,
         Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in,
        Joint_jog_buttons,Cart_jog_buttons,Jog_control,General_data,Buttons,])
    

    process3 = multiprocessing.Process(target=SIMULATOR_process,args =[Position_out,Position_in,Position_Sim])


    process1.start()
    time.sleep(1)
    process2.start()
    time.sleep(1)
    process3.start()
    process1.join()
    process2.join()
    process3.join()

    process1.terminate()
    process2.terminate()
    process3.terminate()





