################################################################################
# Copyright (C) 2012 Leap Motion, Inc. All rights reserved.                    #
# NOTICE: This developer release of Leap Motion, Inc. software is confidential #
# and intended for very limited distribution. Parties using this software must #
# accept the SDK Agreement prior to obtaining this software and related tools. #
# This software is subject to copyright.                                       #
################################################################################

import Leap, sys, time
from time import perf_counter
import socket
import time
import math
import keyboard

# Sender configuration
ip = "127.0.0.1" #Loopback address
port = 5001

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

prev_time  = 0
start = 0
starting_position = [0,0,0]
robot_offset = [0.2, 0, 0.2] #Cartesian x,y,z where the robot starts
offsets = [0,0,0]
hand_status = 0


# Open the file in append mode
data_file = open("motion_data.txt", "a")

class SampleListener(Leap.Listener):
    def __init__(self):
        super().__init__()
        
    def on_init(self, controller):
        print("Initialized")

    def on_connect(self, controller):
        print("Connected")

    def on_disconnect(self, controller):
        print("Disconnected")

    def on_exit(self, controller):
        print("Exited")

    def on_frame(self, controller):
        global prev_time
        global start
        global starting_position
        global robot_offset
        global offsets
        global hand_status

        # Get the most recent frame and report some basic information
        frame = controller.frame()
        if frame.id % 1 == 0 :
            start_time = perf_counter()
            #print(start_time - prev_time)
            prev_time = start_time
            hands = frame.hands
            numHands = len(hands)
            #print("Frame id: %d, timestamp: %d, hands: %d, fingers: %d, tools: %d" % (
                #frame.id, frame.timestamp, numHands, len(frame.fingers), len(frame.tools)))
            if numHands >= 1:
                # Get the first hand
                hand = hands[0]

                # Check if the hand has any fingers
                fingers = hand.fingers
                numFingers = len(fingers)
                if numFingers >= 1:
                    
                    # Calculate the hand's average finger tip position
                    pos = Leap.Vector()
                    for finger in fingers:
                        pos += finger.tip_position
            
                    pos = pos.__div__(numFingers)

                                # Get the hand's grab strength
                grab_strength = hand.grab_strength

                # Determine if the hand is closed based on grab strength
                hand_closed = grab_strength > 0.8

                        # Perform actions based on whether the hand is closed or open
                if hand_closed:
                    hand_status = 0
                    print("Hand closed")
                    # Perform actions for closed fist
                else:
                    hand_status = 1
                    print("Hand open")
                    # Perform actions for open fist


                # Get the palm position
                palm = hand.palm_position

                send_time = time.time()
        
                normal = hand.palm_normal
                direction = hand.direction
                scale = 2000 # bigger than 100 for scale down

                x = -palm[2]/scale
                y = -palm[0]/scale
                z = palm[1]/scale

                # Offset 
                if (start == 0):
                    starting_position[0] = x
                    starting_position[1] = y
                    starting_position[2] = z
                    offsets[0] = robot_offset[0] - starting_position[0]
                    offsets[1] = robot_offset[1] - starting_position[1]
                    offsets[2] = robot_offset[2] - starting_position[2]
                    
                x_send = offsets[0] + x
                y_send = offsets[1] + y
                z_send = offsets[2] + z

                
                # Virtual walls cm6
                
                if x_send <= 0.05:
                    x_send = 0.05
                elif x_send >= 0.45:
                    x_send = 0.45

                if y_send <= -0.30:
                    y_send = -0.30
                elif y_send >= 0.30:
                    y_send = 0.30

                if z_send <= 0:
                    z_send = 0
                elif z_send >= 0.55:
                    z_send = 0.55
                
        
                data_file.write(f"{x_send},{y_send},{z_send},{0},{0},{0}\n")

                msg = f"cart,{x_send},{y_send},{z_send},{0},{math.pi/2},{math.pi},{hand_status}".encode() 
                #msg = f"cart,{x_send},{y_send},{z_send},{0},{0},{math.pi}".encode()


                print(f"xsend {x_send}, ysend {y_send}, zsend {z_send}, roll {-normal.roll}, pitch {direction.pitch + math.pi/2}, jaw { (normal.yaw + math.pi)}")
                sock.sendto(msg, (ip, port))


def start_pos():
    global start 
    start = 1
    print("")

def main():
    # Create a sample listener and controller
    listener = SampleListener()
    controller = Leap.Controller()

    # Have the sample listener receive events from the controller
    controller.add_listener(listener)

    keyboard.add_hotkey('s', start_pos, suppress=True, trigger_on_release=True)

    # Keep this process running until Enter is pressed
    print("Press Enter to quit...")
    try :
        sys.stdin.readline()
    except KeyboardInterrupt:
        pass

    # Remove the sample listener when done
    controller.remove_listener(listener)

if __name__ == "__main__":
    main()
