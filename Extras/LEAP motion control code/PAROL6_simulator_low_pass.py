import customtkinter
import matplotlib.pyplot as plt
import numpy as np
import platform
import os
import logging
import random
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.animation as animation
from s_visual_kinematics.RobotSerial import *
import numpy as np
from math import pi
import socket
import select
import time
import re
from roboticstoolbox import DHRobot, RevoluteDH, ERobot, ELink, ETS
import roboticstoolbox as rp
from spatialmath import *
import keyboard

#Setup IP address and Simulator port
ip = "127.0.0.1" #Loopback address
port = 5001

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((ip, port))
print(f'Start listening to {ip}:{port}')

#
# graphical scale factor
scale_factor = 1
# robot length values (metres)
a1 = 110.50 / 1000  * scale_factor
a2 = 23.42 / 1000 * scale_factor
a3 = 180 / 1000 * scale_factor
a4 = 43.5 / 1000 * scale_factor
a5 = 176.35 / 1000 * scale_factor
a6 = 0 #62.8 / 1000 * scale_factor
a7 = 0 #45.25 / 1000 * scale_factor

alpha_DH = [-pi / 2,pi,pi/2,-pi/2,pi/2,pi]

"""
robot_tb = DHRobot(
    [
        RevoluteDH(d=a1, a=a2, alpha=alpha_DH[0]),
        RevoluteDH(a=a3,d = 0,alpha=alpha_DH[1]),
        RevoluteDH(alpha= alpha_DH[2], a= -a4 ,qlim=[1.7, 4.4] ),
        RevoluteDH(d=-a5, a=0, alpha=alpha_DH[3],qlim=[-np.pi/2, np.pi]),
        RevoluteDH(a=0,d=0,alpha=alpha_DH[4]),
        RevoluteDH(alpha=alpha_DH[5], a = -a7,d = -a6,qlim=[1.047, 5.236]),
    ],
    name="PAROL6",
)
"""

robot_tb = DHRobot(
    [
        RevoluteDH(d=a1, a=a2, alpha=alpha_DH[0]),
        RevoluteDH(a=a3,d = 0,alpha=alpha_DH[1]),
        RevoluteDH(alpha= alpha_DH[2], a= -a4 ),
        RevoluteDH(d=-a5, a=0, alpha=alpha_DH[3]),
        RevoluteDH(a=0,d=0,alpha=alpha_DH[4]),
        RevoluteDH(alpha=alpha_DH[5], a = -a7,d = -a6),
    ],
    name="PAROL6",
)


# Elbow up config
q_t1 = np.array([np.pi / 3, -5 * np.pi / 12, 31 * np.pi / 36, 0, -np.pi / 3, np.pi])
q_t1 = np.array([np.pi / 2, -5 * np.pi / 12, 31 * np.pi / 36, 0, -np.pi / 8, np.pi])
q_t1 = np.array([0, -5 * np.pi / 15, 31 * np.pi / 36, 2*np.pi, -np.pi / 8, np.pi])

# Elbow down config
# None

text_size = 14
customtkinter.set_appearance_mode("Dark")  # Modes: "System" (standard), "Dark", "Light"
customtkinter.set_default_color_theme("blue")  # Themes: "blue" (standard), "green", "dark-blue"

position_in_data = np.array([ 0.,    -1.939,  2.348, -0.,     0.426, 3.142])
cart_data = np.array([0, 0, 0, 0, 0, 0])
prev_position_in_data = None
max_position_change_threshold = 0.5  # Adjust this threshold as needed
# Here save last valid joint angles
good_q = np.array([ 0,0,0,0,0,0])
last_ema = None
previous_angle = 0

# Moving average filter settings
cart_window_size = 3  # Size of the moving average window
cart_position_buffer = np.zeros((cart_window_size, 6))  # Buffer to store recent Cartesian data
cart_buffer_index = 0

# Use to play around with the initial set of joint coordinates for inverse kinematics
def start_pos():
    global q_t1
    q_t1 = position_in_data
    print(f"change q_t1 to {position_in_data}")

def print_q():
    global q_t1
    print(f" q_t1 is {q_t1}")


keyboard.add_hotkey('c', start_pos, suppress=True, trigger_on_release=True)
keyboard.add_hotkey('p', print_q, suppress=True, trigger_on_release=True)


# moving avg low pass filter - Use mean
def cartesian_moving_average(buffer):
    return np.mean(buffer, axis=0)


# moving avg low pass filter - Use convolve # better for large data sets, in our case it is slower
def cartesian_moving_average_2(buffer):
    window = np.ones(cart_window_size) / cart_window_size
    return np.array([np.convolve(buffer[:, i], window, mode='valid')[-1] for i in range(6)])


# Use EMA low pass filter
def cartesian_moving_average_EMA(new_data, last_ema, alpha=0.8):
    """
    Calculate the Exponential Moving Average for Cartesian data.
    
    :param new_data: New data point (array of 6 values for x, y, z, roll, pitch, yaw)
    :param last_ema: The last EMA value (array of 6 values)
    :param alpha: The smoothing factor (0 < alpha <= 1)
    :return: The new EMA value (array of 6 values)
    """
    if last_ema is None:
        return new_data
    return alpha * np.array(new_data) + (1 - alpha) * last_ema


def GUI(Position_in):
    global cart_position_buffer
    global cart_buffer_index
    global cart_window_size

    def show_robot(var):
        global position_in_data
        global cart_data
        global prev_position_in_data
        global good_q
        global q_t1
        global cart_position_buffer
        global cart_buffer_index
        global previous_angle
        global last_ema
        received_data = None
        data = None

        time1 = time.perf_counter()

        def check_elbow_configuration(q):
            straight_pose_j3 = pi / 2
            if q[2] > straight_pose_j3:
                return "Elbow Down"
            elif q[2] < straight_pose_j3:
                return "Elbow Up"

        while True:
            try:
                ready_to_read, _, _ = select.select([sock], [], [], 0)
                if sock in ready_to_read:
                    data, addr = sock.recvfrom(1024)
                else:
                    break
            except KeyboardInterrupt:
                break
            except Exception as e:
                print(f"Error: {e}")
                break

        if data:
            received_data = data.decode()
            if received_data.startswith("pos,"):
                pattern = re.compile(
                    r'^pos,([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?),([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?),([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?),([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?),([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?),([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)$')
                match = pattern.match(received_data)

                if match:
                    position_in_data = np.array([float(match.group(i)) for i in range(1, 7)])
                    print("Received position data:", position_in_data)
                else:
                    print("Error: Invalid message format")
            elif received_data.startswith("cart,"):
                _, x, y, z, roll, pitch, jaw = received_data.split(",")
                cart_data = np.array([float(x), float(y), float(z), float(roll), float(pitch), float(jaw)])
                cart_data2 = np.array([float(x) * 1000, float(y) * 1000, float(z) * 1000, float(roll), float(pitch), float(jaw)])

                # Update the buffer with the new Cartesian data
                cart_position_buffer[cart_buffer_index] = cart_data
                cart_buffer_index = (cart_buffer_index + 1) % cart_window_size

                """
                # Calculate the moving average of the buffer
                filtered_cart_data = cartesian_moving_average(cart_position_buffer)

                Needed_pose = SE3.RPY([filtered_cart_data[3], filtered_cart_data[4], filtered_cart_data[5]], unit='rad', order='xyz')
                Needed_pose.t[0] = filtered_cart_data[0]
                Needed_pose.t[1] = filtered_cart_data[1]
                Needed_pose.t[2] = filtered_cart_data[2]
                """

                def unwrap_angle(angle):
                    return (angle + np.pi) % (2 * np.pi) - np.pi

                def maintain_angle_continuity(new_angle, previous_angle):
                    return previous_angle + unwrap_angle(new_angle - previous_angle)

                last_ema = cartesian_moving_average_EMA(cart_data, last_ema)
                # Use last_ema instead of filtered_cart_data
                Needed_pose = SE3.RPY([last_ema[3], last_ema[4], last_ema[5]], unit='rad', order='xyz')
                Needed_pose.t[0] = last_ema[0]
                Needed_pose.t[1] = last_ema[1]
                Needed_pose.t[2] = last_ema[2]


                prev_position_in_data = position_in_data

                q1 = robot_tb.ik_LM(Needed_pose, method="chan", q0=q_t1, ilimit=59, slimit=59)

                if (q1[1] == 0):
                    position_in_data = good_q
                if (q1[1] == 1):
                    good_q = q1[0]
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
                elbow_config = check_elbow_configuration(position_in_data)
                print(f"Joint data is {position_in_data}")
                #print(f"Cart data is {cart_data}")

                if prev_position_in_data is not None:
                    position_change = np.abs(position_in_data - prev_position_in_data)
                    large_position_change_indices = np.where(position_change > max_position_change_threshold)[0]
                    if len(large_position_change_indices) > 0:
                        None
                        #print("Error: Position change is too quick for joints:", large_position_change_indices)
                        #print("Position change values:", position_change[large_position_change_indices])

            else:
                print("Error: Unknown message format")

        f = robot.forward(position_in_data)
        robot.draw()

    dh_params = np.array([[a1, a2, alpha_DH[0], 0.],
                          [0., a3, alpha_DH[1], 0],
                          [0., -a4, alpha_DH[2], 0],
                          [-a5, 0., alpha_DH[3], 0],
                          [0., 0., alpha_DH[4], 0.],
                          [-a6, -a7, alpha_DH[5], 0]])

    robot = RobotSerial(dh_params)

    fig = plt.figure(figsize=(7, 7))

    robot.init_custum_frame(fig)
    random.seed(5)
    np.set_printoptions(precision=3, suppress=True)
    theta = np.array([0., -1.939, 2.348, -0., 0.426, 3.142])
    f = robot.forward(theta)

    app = customtkinter.CTk()
    app.lift()
    app.attributes('-topmost', True)
    logging.debug("Simulator running!")
    app.title("Simulator.py")
    app.geometry(f"{750}x{680}")
    app.wm_attributes('-topmost', False)

    app.grid_columnconfigure((1, 2), weight=1)
    app.grid_columnconfigure((0), weight=1)
    app.grid_rowconfigure((0), weight=0)
    app.grid_rowconfigure((1), weight=0)
    app.grid_rowconfigure((2), weight=1)
    app.grid_rowconfigure((3), weight=0)

    Top_frame = customtkinter.CTkFrame(app, height=0, width=150, corner_radius=0, )
    Top_frame.grid(row=0, column=0, columnspan=4, padx=(5, 5), pady=(5, 5), sticky="new")
    Top_frame.grid_columnconfigure(0, weight=0)
    Top_frame.grid_rowconfigure(0, weight=0)

    def Top_frame():
        Top_frame = customtkinter.CTkFrame(app, height=0, width=150, corner_radius=0, )
        Top_frame.grid(row=0, column=0, columnspan=4, padx=(5, 5), pady=(5, 5), sticky="new")
        Top_frame.grid_columnconfigure(0, weight=0)
        Top_frame.grid_rowconfigure(0, weight=0)

        Control_button = customtkinter.CTkButton(Top_frame, text="Pause", width=50, fg_color="#313838",
                                                 font=customtkinter.CTkFont(size=15, family='TkDefaultFont'),
                                                 command=Pause)
        Control_button.grid(row=0, column=0, padx=20, pady=(5, 5), sticky="news")

        Config_button = customtkinter.CTkButton(Top_frame, text="Run", width=50, fg_color="#313838",
                                                font=customtkinter.CTkFont(size=15, family='TkDefaultFont'), command=Run)
        Config_button.grid(row=0, column=1, padx=20, pady=(5, 5), sticky="news")

        Setup_button = customtkinter.CTkButton(Top_frame, text="Sync", width=50, fg_color="#313838",
                                               font=customtkinter.CTkFont(size=15, family='TkDefaultFont'))
        Setup_button.grid(row=0, column=2, padx=20, pady=(5, 5), sticky="news")

    def Pause():
        ani.event_source.stop()

    def Run():
        ani.event_source.start()

    Top_frame()

    canvas = FigureCanvasTkAgg(fig, master=app)
    canvas.draw()
    canvas.get_tk_widget().grid(row=1, column=1, padx=20,pady = (5,5),sticky="news")

    ani = animation.FuncAnimation(fig, show_robot, interval=85,frames=30)

    app.mainloop() 


if __name__ == "__main__":

    Position_in = [0,0,0,0,0,0]
    GUI(Position_in)
