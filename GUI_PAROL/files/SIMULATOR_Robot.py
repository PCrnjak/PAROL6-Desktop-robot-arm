import tkinter
import tkinter.messagebox
import customtkinter
import matplotlib.pyplot as plt
import time
import roboticstoolbox as rp
import numpy as np
import platform
import os
import PIL
from PIL import Image, ImageTk
import logging
import tkinter as tk
from tkinter import ttk
from tkinter.messagebox import showinfo
import random
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.animation as animation
from visual_kinematics.RobotSerial import *
import numpy as np
from math import pi
import PAROL6_ROBOT

logging.basicConfig(level = logging.DEBUG,
    format='%(asctime)s.%(msecs)03d %(levelname)s:\t%(message)s',
    datefmt='%H:%M:%S'
)
#logging.disable(logging.DEBUG)


# Finds out where the program and images are stored
my_os = platform.system()
if my_os == "Windows":
    Image_path = os.path.join(os.path.dirname(os.path.realpath(__file__)))
    logging.debug("Os is Windows")
else:
    Image_path = os.path.join(os.path.dirname(os.path.realpath(__file__)))
    logging.debug("Os is Linux")
    
logging.debug(Image_path)

text_size = 14

customtkinter.set_appearance_mode("Dark")  # Modes: "System" (standard), "Dark", "Light"
customtkinter.set_default_color_theme("blue")  # Themes: "blue" (standard), "green", "dark-blue"


def GUI(Position_out,Position_in,Position_Sim):

    def show_robot(var):
        #print(var)
        x = random.random()
        y = random.random()
        z = random.random()
        theta = np.array([y, z, -0.25 * pi, 0., x, 0.])
        #theta = np.array([0, -pi/2, pi, 0., 0, pi])
        f = robot.forward(theta)      
        robot.draw()

    scale_factor = 2.5
    a1 = PAROL6_ROBOT.a1 * scale_factor
    a2 = PAROL6_ROBOT.a2 * scale_factor
    a3 = PAROL6_ROBOT.a3 * scale_factor
    a4 = PAROL6_ROBOT.a4 * scale_factor
    a5 = PAROL6_ROBOT.a5 * scale_factor
    a6 = PAROL6_ROBOT.a6 * scale_factor
    a7 = PAROL6_ROBOT.a7 * scale_factor

    alpha_DH = PAROL6_ROBOT.alpha_DH


    dh_params = np.array([[a1, a2, alpha_DH[0], 0.],
                          [0., a3, alpha_DH[1], 0],
                          [0., -a4, alpha_DH[2], 0],
                          [-a5, 0., alpha_DH[3], 0],
                          [0., 0.,alpha_DH[4], 0.],
                          [-a6, -a7, alpha_DH[5], 0]])
    
    robot = RobotSerial(dh_params)

    fig = plt.figure(figsize=(7,7))
    #ax = fig.add_subplot(111, projection="3d")
    #print(plt.margins())
    #fig.margins(3,4)

    robot.init_custum_frame(fig)
    random.seed(5)
    np.set_printoptions(precision=3, suppress=True)
    theta = np.array([0., 0., -0.25 * pi, 0., 0., 0.])
    f = robot.forward(theta)

    app = customtkinter.CTk()
    logging.debug("I RUN")
        # configure window
    app.title("Simulator.py")
    app.geometry(f"{900}x{780}")
    app.attributes('-topmost',False)

    # Add app icon
    logo = (os.path.join(Image_path, "logo.ico"))
    app.iconbitmap(logo)

    # configure grid layout (4x4) wight 0 znači da je fixed, 1 znači da scale radi?
    app.grid_columnconfigure((1,2), weight=1)
    app.grid_columnconfigure((0), weight=1)
    app.grid_rowconfigure((0), weight=0)
    app.grid_rowconfigure((1), weight=0)
    app.grid_rowconfigure((2), weight=1)
    app.grid_rowconfigure((3), weight=0) 


    Top_frame = customtkinter.CTkFrame(app ,height = 0,width=150, corner_radius=0, )
    Top_frame.grid(row=0, column=0, columnspan=4, padx=(5,5), pady=(5,5),sticky="new")
    Top_frame.grid_columnconfigure(0, weight=0)
    Top_frame.grid_rowconfigure(0, weight=0)

    def Top_frame():
            Top_frame = customtkinter.CTkFrame(app ,height = 0,width=150, corner_radius=0, )
            Top_frame.grid(row=0, column=0, columnspan=4, padx=(5,5), pady=(5,5),sticky="new")
            Top_frame.grid_columnconfigure(0, weight=0)
            Top_frame.grid_rowconfigure(0, weight=0)

        
            Control_button = customtkinter.CTkButton( Top_frame,text="Pause", width= 50, font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command=Pause)
            Control_button.grid(row=0, column=0, padx=20,pady = (5,5),sticky="news")

            Config_button = customtkinter.CTkButton( Top_frame,text="Run", width= 50, font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command=Run)
            Config_button.grid(row=0, column=1, padx=20,pady = (5,5),sticky="news")

            Setup_button = customtkinter.CTkButton( Top_frame,text="Sync", width= 50, font = customtkinter.CTkFont(size=15, family='TkDefaultFont'))
            Setup_button.grid(row=0, column=2, padx=20,pady = (5,5),sticky="news")

    def Pause():
        ani.event_source.stop()
        


    def Run():
        ani.event_source.start()
    
    Top_frame()

    canvas = FigureCanvasTkAgg(fig, master=app)
    canvas.draw()
    canvas.get_tk_widget().grid(row=1, column=1, padx=20,pady = (5,5),sticky="news")


    ani = animation.FuncAnimation(fig, show_robot, frames=100, interval=100)


    app.mainloop() 



if __name__ == "__main__":
    var = 150
    Position_out = [1,11,111,1111,11111,10]
    Position_in = [31,32,33,34,35,36]
    Position_Sim = [1,2,3,4,5,6]
    GUI(Position_out,Position_in,Position_Sim)
