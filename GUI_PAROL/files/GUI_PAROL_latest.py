import tkinter
import tkinter.messagebox
import customtkinter
import matplotlib.pyplot as plt
import time
import roboticstoolbox as rp
import numpy as np
import platform
import os
from tkinter import filedialog
import PIL
from PIL import Image, ImageTk
import logging
import tkinter as tk
from tkinter import ttk
from tkinter.messagebox import showinfo
from tkinter import messagebox
import random
import multiprocessing
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

# Globals
current_menu = "Jog"
Wrf_Trf = "WRF"
Current_Custom_pose_select = "Current"
Robot_sim = True
Real_robot = True
left_right_select = "Left"
Now_open_txt = ''


#customtkinter.set_appearance_mode("Light")  # Modes: "System" (standard), "Dark", "Light"
customtkinter.set_appearance_mode("Dark")  # Modes: "System" (standard), "Dark", "Light"
customtkinter.set_default_color_theme("blue")  # Themes: "blue" (standard), "green", "dark-blue"

left_jog_buttons = [0,0,0,0,0,0]
right_jog_buttons  =[0,0,0,0,0,0]
translation_buttons = [0,0,0,0,0,0]
rotation_buttons = [0,0,0,0,0,0]

left_frames_width = 2000

prev_positions = np.array([0,0,0,0,0,0])
robot_pose = [0,0,0,0,0,0] #np.array([0,0,0,0,0,0])

padx_top_bot = 20
def GUI(Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out,
         Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in,
        Joint_jog_buttons,Cart_jog_buttons,Jog_control,General_data,Buttons):
    


    app = customtkinter.CTk()

    #logging.debug(left_jog_buttons)
    #logging.debug(Joint_jog_buttons)
    logging.debug("I RUN")
        # configure window
    app.title("Source controller.py")
    app.geometry(f"{1700}x{1100}")
    app.attributes('-topmost',False)
    # Add app icon  
    logo = (os.path.join(Image_path, "logo.ico"))
    app.iconbitmap(logo)

    # configure grid layout (4x4) wight 0 znači da je fixed, 1 znači da scale radi?
    app.grid_columnconfigure((1,2), weight=1)
    app.grid_columnconfigure((0,3), weight=0)
    app.grid_rowconfigure((0,3,2), weight=0)
    app.grid_rowconfigure((1), weight=1) 
    app.grid_rowconfigure((4), weight=0) 
    
    #images

    # dodaj plot koji je popup, dodaj help button, I/O je isto popup zasad i tamo je i gripper
    def top_frames():

        # frames for top panel mode selection section
        app.menu_select_frame = customtkinter.CTkFrame(app,height = 0,width=150, corner_radius=0, )
        app.menu_select_frame.grid(row=0, column=0, columnspan=4, padx=(5,5), pady=5,sticky="new")
        app.menu_select_frame.grid_columnconfigure(0, weight=0)
        app.menu_select_frame.grid_rowconfigure(0, weight=0)

        # Move button
        app.move_mode_select_button = customtkinter.CTkButton(app.menu_select_frame,text="Move", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = raise_frame_jog)
        app.move_mode_select_button.grid(row=0, column=0, padx=(padx_top_bot,0),pady = 5,sticky="nw")

        # I/O button
        app.I0_mode_select_button = customtkinter.CTkButton(app.menu_select_frame,text="I/O", font = customtkinter.CTkFont(size=15, family='TkDefaultFont' ),command = raise_frame_IO)
        app.I0_mode_select_button.grid(row=0, column=1, padx=(padx_top_bot,0),pady = 5,sticky="nw")

        # Settings button
        app.Settings_mode_select_button = customtkinter.CTkButton(app.menu_select_frame,text="Settings", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = raise_frame_setting)
        app.Settings_mode_select_button.grid(row=0, column=2, padx=(padx_top_bot,0) ,pady = 5,sticky="nw")

            # Calibrate button
        app.Calibrate_button = customtkinter.CTkButton(app.menu_select_frame,text="Calibrate", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = raise_calibrate_frame)
        app.Calibrate_button.grid(row=0, column=3, padx=(padx_top_bot,0) ,pady = 5,sticky="nw")

        # Plot button
        app.Plot_button = customtkinter.CTkButton(app.menu_select_frame,text="Plot", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = raise_calibrate_frame)
        app.Plot_button.grid(row=0, column=4, padx=(padx_top_bot,0) ,pady = 5,sticky="nw")

        # Gripper button
        app.Plot_button = customtkinter.CTkButton(app.menu_select_frame,text="Gripper", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'))
        app.Plot_button.grid(row=0, column=4, padx=(padx_top_bot,0) ,pady = 5,sticky="nw")

        app.fw_label = customtkinter.CTkLabel(app.menu_select_frame, text="Source controller fw version: 1.0.0", font=customtkinter.CTkFont(size=12))
        app.fw_label.grid(row=0, column=5, padx=(300,10), pady=5 ,sticky="ne")

        # help button
        help_image =Image.open(os.path.join(Image_path, "help.png"))
        app.help_button_image = customtkinter.CTkImage(help_image, size=(25, 25))
        
        app.help_button = customtkinter.CTkButton(app.menu_select_frame, corner_radius=0, height=1, border_spacing=10,
                                                fg_color="transparent", text_color=("gray10", "gray90"),
                                                image=app.help_button_image, anchor="CENTER",text = "",hover = 0,command = Open_help) #hover = 0
        app.help_button.grid(row=0, column=6, padx=(120,0), sticky="news")


    def bottom_frames():

        #frames for bottom panel section
        app.bottom_select_frame = customtkinter.CTkFrame(app,height = 100, corner_radius=0, )
        app.bottom_select_frame.grid(row=4, column=0, columnspan=4, padx=(5,5), pady=2, sticky="sew")
        app.bottom_select_frame.grid_columnconfigure(0, weight=0)

        # radio button left
        app.radio_button_sim = customtkinter.CTkRadioButton(master=app.bottom_select_frame, text="Simulator",  value=2,command = Select_simulator)
        app.radio_button_sim.grid(row=3, column=3, pady=10, padx=padx_top_bot, sticky="e")
        app.radio_button_sim.select()

        app.radio_button_real = customtkinter.CTkRadioButton(master=app.bottom_select_frame, text="Real robot",  value=2,command = Select_real_robot)
        app.radio_button_real.grid(row=3, column=4, pady=10, padx=padx_top_bot, sticky="e")
        app.radio_button_real.select()

        app.Connect_button = customtkinter.CTkButton(app.bottom_select_frame,text="Connect", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'))
        app.Connect_button.grid(row=3, column=8, padx=padx_top_bot,pady = 10,sticky="e")

        app.Clear_error = customtkinter.CTkButton(app.bottom_select_frame,text="Clear error", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'))
        app.Clear_error.grid(row=3, column=9, padx=padx_top_bot,pady = 10,sticky="e")

        #app.enable_disable = customtkinter.CTkButton(app.bottom_select_frame,text="Enable", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'))
        #app.enable_disable.grid(row=3, column=9, padx=padx_top_bot,pady = 10,sticky="e")

        #app.home = customtkinter.CTkButton(app.bottom_select_frame,text="Home", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = Home_robot)
        #app.home.grid(row=3, column=10, padx=padx_top_bot,pady = 10,sticky="e")

        app.estop_status = customtkinter.CTkLabel(app.bottom_select_frame, text="Estop activated", font=customtkinter.CTkFont(size=15))
        app.estop_status.grid(row=3, column=11, padx=(0,0), pady=10 ,sticky="ne")


    app.jog_frame = customtkinter.CTkFrame(app,height = 100, width = left_frames_width, corner_radius=0, )
    app.jog_frame.grid(row=1, column=0, columnspan=1,rowspan=2, padx=(5,0), pady=5, sticky="news")
    app.jog_frame.grid_columnconfigure(0, weight=0)
    app.jog_frame.grid_rowconfigure(0, weight=0)


    def joint_jog_frames():
        #jog frame
        app.joint_jog = customtkinter.CTkButton(app.jog_frame,text="Joint jog", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = raise_frame_jog)
        app.joint_jog.grid(row=0, column=0, padx=20,pady = (10,20),sticky="news")

        app.cart_jog = customtkinter.CTkButton(app.jog_frame,text="Cartesian jog", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = raise_frame_cart)
        app.cart_jog.grid(row=0, column=1, padx=20,pady = (10,20),sticky="news")

        joint_names = ['Base', 'Shoulder', 'Elbow', 'Wrist 1', 'Wrist 2', 'Wrist 3']

        def button_press_left(event=None, var = 0):
            left_jog_buttons[var] = 1
            Joint_jog_buttons[var] = 1
            logging.debug(left_jog_buttons)
            logging.debug("Joint jog press " + str(list(Joint_jog_buttons)))
            

        def button_rel_left(event=None, var = 0):
            left_jog_buttons[var] = 0
            Joint_jog_buttons[var] = 0
            logging.debug(left_jog_buttons)
            logging.debug("Joint jog release " + str(list(Joint_jog_buttons)))
            

        def button_press_right(event=None, var = 0):
            right_jog_buttons[var] = 1
            Joint_jog_buttons[var + 6] = 1
            logging.debug(right_jog_buttons)
            logging.debug("Joint jog press " + str(list(Joint_jog_buttons)))
            

        def button_rel_right(event=None, var = 0):
            right_jog_buttons[var] = 0
            Joint_jog_buttons[var + 6] = 0
            logging.debug(right_jog_buttons)
            logging.debug("Joint jog release " + str(list(Joint_jog_buttons)))
            

        true_image =Image.open(os.path.join(Image_path, "button_arrow_1.png"))
        rotated_image = true_image.rotate(90)
        rotated_image2 = true_image.rotate(270)

        app.move_arrow = {}
        app.move_arrow_right = {}
        app.progress_bar_joints = {}

        for y in range(0,6):

            def make_lambda1(x):
                return lambda ev:button_press_left(ev,x)

            def make_lambda2(x):
                return lambda ev:button_rel_left(ev,x)

            def make_lambda3(x):
                return lambda ev:button_press_right(ev,x)

            def make_lambda4(x):
                return lambda ev:button_rel_right(ev,x)


            # Labels of joints
            app.Base_label = customtkinter.CTkLabel(app.jog_frame, text=joint_names[y], anchor="w",font=customtkinter.CTkFont(size=20))
            app.Base_label.grid(row=y+1, column=0, padx=(5, 0))

            #progress bars for joints 
            app.progress_bar_joints[y] = customtkinter.CTkProgressBar(app.jog_frame, orientation="horizontal",height = 5)
            app.progress_bar_joints[y].grid(row=1 + y, column=2 , rowspan=1, padx=(20, 20), pady=(35, 35), sticky="ns")
        

            app.move_arrow_image = customtkinter.CTkImage(rotated_image, size=(65, 65))
            app.move_arrow[y] = customtkinter.CTkButton(app.jog_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    image=app.move_arrow_image, anchor="CENTER",text = "",hover = 0) #hover = 0
            app.move_arrow[y].grid(row=1 + y, column=1, sticky="news")
            app.move_arrow[y].bind('<ButtonPress-1>',make_lambda1(y))
            app.move_arrow[y].bind('<ButtonRelease-1>',make_lambda2(y))

            app.move_arrow_image_right = customtkinter.CTkImage(rotated_image2, size=(65, 65))
            app.move_arrow_right[y]  = customtkinter.CTkButton(app.jog_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    anchor="CENTER",text = "",image=app.move_arrow_image_right,hover = 0) #hover = 0
            app.move_arrow_right[y].grid(row=1 + y, column=3, sticky="news")
            app.move_arrow_right[y].bind('<ButtonPress-1>',make_lambda3(y))
            app.move_arrow_right[y].bind('<ButtonRelease-1>',make_lambda4(y))


    app.cart_frame = customtkinter.CTkFrame(app,height = 100, width = left_frames_width, corner_radius=0, )
    app.cart_frame.grid(row=1, column=0, columnspan=1, rowspan=2, padx=(5,0), pady=5, sticky="news")
    app.cart_frame.grid_columnconfigure(0, weight=0)
    app.cart_frame.grid_rowconfigure(0, weight=0)


    app.settings_frame = customtkinter.CTkFrame(app,height = 100, width = left_frames_width, corner_radius=0, )
    app.settings_frame.grid(row=1, column=0, columnspan=1,rowspan=2,  padx=(5,0), pady=5, sticky="news")
    app.settings_frame.grid_columnconfigure(0, weight=0)
    app.settings_frame.grid_rowconfigure(0, weight=0)

    app.IO_frame = customtkinter.CTkFrame(app,height = 100, width = left_frames_width, corner_radius=0, )
    app.IO_frame.grid(row=1, column=0, columnspan=1,rowspan=2,  padx=(5,0), pady=5, sticky="news")
    app.IO_frame.grid_columnconfigure(0, weight=0)
    app.IO_frame.grid_rowconfigure(0, weight=0)

    app.Calibrate_frame = customtkinter.CTkFrame(app,height = 100, width = left_frames_width, corner_radius=0, )
    app.Calibrate_frame.grid(row=1, column=0, columnspan=1,rowspan=2,  padx=(5,0), pady=5, sticky="news")
    app.Calibrate_frame.grid_columnconfigure(0, weight=0)
    app.Calibrate_frame.grid_rowconfigure(0, weight=0)


    def settings_frame():
        app.demo1 = customtkinter.CTkButton(app.settings_frame,text="demo1", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'))
        app.demo1.grid(row=0, column=0, padx=20,pady = (10,20),sticky="news")

        app.scaling_label = customtkinter.CTkLabel(app.settings_frame, text="UI Scaling:", anchor="w")
        app.scaling_label.grid(row=0, column=1, padx=(5, 0))

        app.scaling_optionemenu = customtkinter.CTkOptionMenu(app.settings_frame, values=["80%", "90%", "100%", "110%", "120%","150%"],
                                                            command=change_scaling_event)
        app.scaling_optionemenu.grid(row=0, column=2,padx=(5, 0) )

        app.appearance_mode_label = customtkinter.CTkLabel(app.settings_frame, text="Appearance Mode:", anchor="w")
        app.appearance_mode_label.grid(row=0, column=3, padx=(5, 0))
        app.appearance_mode_optionemenu = customtkinter.CTkOptionMenu(app.settings_frame, values= ["Dark","Light"],
                                                                    command=change_appearance_mode_event)
        app.appearance_mode_optionemenu.grid(row=0, column=4, padx=(5, 0))

    def IO_frame():
        app.demo2 = customtkinter.CTkButton(app.IO_frame,text="io", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'))
        app.demo2.grid(row=0, column=1, padx=20,pady = (10,20),sticky="news")

    def Calibrate_frame():
        app.demo2 = customtkinter.CTkButton(app.Calibrate_frame,text="calibrate", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'))
        app.demo2.grid(row=0, column=1, padx=20,pady = (10,20),sticky="news")

    def translation_press(event=None,var=0, var2 = 0):
        translation_buttons[var2] = var
        Cart_jog_buttons[var2] = var
        logging.debug(translation_buttons)
        logging.debug("CART JOG PRESS " + str(list(Cart_jog_buttons)))

    def translation_release(event=None,var=0, var2 = 0):
        translation_buttons[var2] = var
        Cart_jog_buttons[var2] = var
        logging.debug(translation_buttons)
        logging.debug("CART JOG RELEASE " + str(list(Cart_jog_buttons)))

    def make_lambda_press(x,var2):
        return lambda ev:translation_press(ev,x,var2)        

    def make_lambda_release(x,var2):
        return lambda ev:translation_release(ev,x,var2)   

    def rotation_press(event=None,var=0, var2 = 0):
        rotation_buttons[var2] = var
        Cart_jog_buttons[var2+6] = var
        logging.debug(rotation_buttons)
        logging.debug("CART JOG PRESS " + str(list(Cart_jog_buttons)))


    def rotation_release(event=None,var=0, var2 = 0):
        rotation_buttons[var2] = var
        Cart_jog_buttons[var2+6] = var
        logging.debug(rotation_buttons)
        logging.debug("CART JOG RELEASE " + str(list(Cart_jog_buttons)))

    def make_lambda_press_rot(x,var2):
        return lambda ev:rotation_press(ev,x,var2)        

    def make_lambda_release_rot(x,var2):
        return lambda ev:rotation_release(ev,x,var2)   


    def cart_jog_frame():


        app.WRF_select = customtkinter.CTkRadioButton(master=app.cart_frame, text="WRF",  value=0,command = WRF_button)
        app.WRF_select.grid(row=0, column=2, pady=10, padx=20, sticky="we")

        app.TRF_select = customtkinter.CTkRadioButton(master=app.cart_frame, text="TRF",  value=2,command = TRF_button)
        app.TRF_select.grid(row=0, column=3, pady=10, padx=20, sticky="we")

        app.joint_jog = customtkinter.CTkButton(app.cart_frame,text="Joint jog", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = raise_frame_jog)
        app.joint_jog.grid(row=0, column=0, padx=20,pady = (10,20),sticky="news")

        app.cart_jog = customtkinter.CTkButton(app.cart_frame,text="Cartesian jog", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = raise_frame_cart)
        app.cart_jog.grid(row=0, column=1, padx=20,pady = (10,20),sticky="news")

        app.WRF_select.select()
        #cart z up and down            
        z_up =Image.open(os.path.join(Image_path, "cart_z.png"))
        z_down = z_up.rotate(180)

        app.z_up = customtkinter.CTkImage(z_up, size=(80, 80))
        app.z_up_button = customtkinter.CTkButton(app.cart_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    image=app.z_up, anchor="CENTER",text = "",hover = 0) #hover = 0
        app.z_up_button.place(x = 530, y = 80)
        app.z_up_button.bind('<ButtonPress-1>',make_lambda_press(1,4))
        app.z_up_button.bind('<ButtonRelease-1>',make_lambda_release(0,4))

        app.z_down = customtkinter.CTkImage(z_down, size=(80, 80))
        app.z_down_button = customtkinter.CTkButton(app.cart_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    image=app.z_down, anchor="CENTER",text = "",hover = 0) #hover = 0
        app.z_down_button.place(x = 530, y = 260)
        app.z_down_button.bind('<ButtonPress-1>',make_lambda_press(1,5))
        app.z_down_button.bind('<ButtonRelease-1>',make_lambda_release(0,5))


        #cart x up
        x_up =Image.open(os.path.join(Image_path, "cart_x_up.png"))
        app.x_up = customtkinter.CTkImage(x_up, size=(80, 80))
        app.x_up_button = customtkinter.CTkButton(app.cart_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    image=app.x_up, anchor="CENTER",text = "",hover = 0) #hover = 0
        app.x_up_button.place(x = 170, y = 60)
        app.x_up_button.bind('<ButtonPress-1>',make_lambda_press(1,0))
        app.x_up_button.bind('<ButtonRelease-1>',make_lambda_release(0,0))

        #cart x down
        x_down =Image.open(os.path.join(Image_path, "cart_x_down.png"))
        app.x_down = customtkinter.CTkImage(x_down, size=(120, 120))
        app.x_down_button = customtkinter.CTkButton(app.cart_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    image=app.x_down, anchor="CENTER",text = "",hover = 0) #hover = 0
        app.x_down_button.place(x = 160, y = 260)
        app.x_down_button.bind('<ButtonPress-1>',make_lambda_press(1,1))
        app.x_down_button.bind('<ButtonRelease-1>',make_lambda_release(0,1))

        #cart y left right
        y_left =Image.open(os.path.join(Image_path, "cart_y.png"))
        y_right = y_left.transpose(PIL.Image.Transpose.FLIP_LEFT_RIGHT)  

        app.y_left = customtkinter.CTkImage(y_left, size=(90, 90))
        app.y_left_button = customtkinter.CTkButton(app.cart_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    image=app.y_left, anchor="CENTER",text = "",hover = 0) #hover = 0
        app.y_left_button.place(x = 40, y = 150)
        app.y_left_button.bind('<ButtonPress-1>',make_lambda_press(1,2))
        app.y_left_button.bind('<ButtonRelease-1>',make_lambda_release(0,2))



        app.y_right = customtkinter.CTkImage(y_right, size=(90, 90))
        app.y_right_button = customtkinter.CTkButton(app.cart_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    image=app.y_right, anchor="CENTER",text = "",hover = 0) #hover = 0
        app.y_right_button.place(x = 300, y = 150)
        app.y_right_button.bind('<ButtonPress-1>',make_lambda_press(1,3))
        app.y_right_button.bind('<ButtonRelease-1>',make_lambda_release(0,3))


        #cart rot x + x-
        rot_x_pos =Image.open(os.path.join(Image_path, "arrow_x_rot.png"))
        app.rot_x_pos = customtkinter.CTkImage(rot_x_pos, size=(90, 90))
        app.rot_x_pos_button = customtkinter.CTkButton(app.cart_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    image=app.rot_x_pos, anchor="CENTER",text = "",hover = 0) #hover = 0
        app.rot_x_pos_button.place(x = 400, y = 490)
        app.rot_x_pos_button.bind('<ButtonPress-1>',make_lambda_press_rot(1,0))
        app.rot_x_pos_button.bind('<ButtonRelease-1>',make_lambda_release_rot(0,0))

        rot_x_neg = rot_x_pos.transpose(PIL.Image.Transpose.FLIP_LEFT_RIGHT) 
        rot_x_neg = rot_x_neg.rotate(180)
        app.rot_x_neg = customtkinter.CTkImage(rot_x_neg, size=(90, 90))
        app.rot_x_neg_button = customtkinter.CTkButton(app.cart_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    image=app.rot_x_neg, anchor="CENTER",text = "",hover = 0) #hover = 0
        app.rot_x_neg_button.place(x = 400, y = 330)
        app.rot_x_neg_button.bind('<ButtonPress-1>',make_lambda_press_rot(1,1))
        app.rot_x_neg_button.bind('<ButtonRelease-1>',make_lambda_release_rot(0,1))


        #cart rot y + y-
        rot_y_pos =Image.open(os.path.join(Image_path, "arrow_y_rot.png"))
        app.rot_y_pos = customtkinter.CTkImage(rot_y_pos, size=(90, 90))
        app.rot_y_pos_button = customtkinter.CTkButton(app.cart_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    image=app.rot_y_pos, anchor="CENTER",text = "",hover = 0) #hover = 0
        app.rot_y_pos_button.place(x = 300, y = 410)
        app.rot_y_pos_button.bind('<ButtonPress-1>',make_lambda_press_rot(1,2))
        app.rot_y_pos_button.bind('<ButtonRelease-1>',make_lambda_release_rot(0,2))


        rot_y_neg = rot_y_pos.transpose(PIL.Image.Transpose.FLIP_LEFT_RIGHT) 
        app.rot_y_neg = customtkinter.CTkImage(rot_y_neg, size=(90, 90))
        app.rot_y_neg_button = customtkinter.CTkButton(app.cart_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    image=app.rot_y_neg, anchor="CENTER",text = "",hover = 0) #hover = 0
        app.rot_y_neg_button.place(x = 500, y = 410)
        app.rot_y_neg_button.bind('<ButtonPress-1>',make_lambda_press_rot(1,3))
        app.rot_y_neg_button.bind('<ButtonRelease-1>',make_lambda_release_rot(0,3))


        #cart rot z + z -
        rot_z_pos =Image.open(os.path.join(Image_path, "arrow_z_rot.png"))
        rot_z_pos_rot = rot_z_pos.rotate(270)
        app.rot_z_pos = customtkinter.CTkImage(rot_z_pos_rot, size=(90, 90))
        app.rot_z_pos_button = customtkinter.CTkButton(app.cart_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    image=app.rot_z_pos, anchor="CENTER",text = "",hover = 0) #hover = 0
        app.rot_z_pos_button.place(x = 50, y = 490)
        app.rot_z_pos_button.bind('<ButtonPress-1>',make_lambda_press_rot(1,5))
        app.rot_z_pos_button.bind('<ButtonRelease-1>',make_lambda_release_rot(0,5))

        rot_z_neg = rot_z_pos_rot.transpose(PIL.Image.Transpose.FLIP_TOP_BOTTOM)
        app.rot_z_neg = customtkinter.CTkImage(rot_z_neg, size=(90, 90))
        app.rot_z_neg_button = customtkinter.CTkButton(app.cart_frame, corner_radius=0, height=10, border_spacing=10,
                                                    fg_color="transparent", text_color=("gray10", "gray90"),
                                                    image=app.rot_z_neg, anchor="CENTER",text = "",hover = 0) #hover = 0
        app.rot_z_neg_button.place(x = 50, y = 370)
        app.rot_z_neg_button.bind('<ButtonPress-1>',make_lambda_press_rot(1,4))
        app.rot_z_neg_button.bind('<ButtonRelease-1>',make_lambda_release_rot(0,4))

    def robot_positions_frames():

        #robot positions frame
        app.joint_positions_frame = customtkinter.CTkFrame(app, width = left_frames_width,height = 300, corner_radius=0, )
        app.joint_positions_frame.grid(row=3, column=0, columnspan=1, padx=(5,0), pady=5, sticky="news")
        app.joint_positions_frame.grid_columnconfigure(0, weight=0)
        app.joint_positions_frame.grid_rowconfigure(0, weight=0)
        app.joint_positions_frame.grid_columnconfigure(0, weight=0)


        # Tool positions
        app.tools_positions = customtkinter.CTkLabel(app.joint_positions_frame, text="Tools positions:", font=customtkinter.CTkFont(size=16))
        app.tools_positions.grid(row=0, column=0, padx=(10,10), pady=5, sticky="nsew")

        app.x_pos = customtkinter.CTkLabel(app.joint_positions_frame, text="X: "+ str(1929).rjust(7, ' '), font=customtkinter.CTkFont(size=text_size))
        app.x_pos.grid(row=1, column=0, padx=(10,10), pady=5, sticky="nsew")

        app.y_pos = customtkinter.CTkLabel(app.joint_positions_frame, text="Y: "+ str(1929).rjust(7, ' '), font=customtkinter.CTkFont(size=text_size))
        app.y_pos.grid(row=2, column=0, padx=(10,10), pady=5, sticky="nsew")

        app.z_pos = customtkinter.CTkLabel(app.joint_positions_frame, text="Z: "+ str(1929).rjust(7, ' '), font=customtkinter.CTkFont(size=text_size))
        app.z_pos.grid(row=3, column=0, padx=(10,10), pady=5, sticky="nsew")

        app.Rx_pos = customtkinter.CTkLabel(app.joint_positions_frame, text="Rx: "+ str(1929).rjust(7, ' '), font=customtkinter.CTkFont(size=text_size))
        app.Rx_pos.grid(row=4, column=0, padx=(10,10), pady=5, sticky="nsew")

        app.Ry_pos = customtkinter.CTkLabel(app.joint_positions_frame, text="Ry: "+ str(1929).rjust(7, ' '), font=customtkinter.CTkFont(size=text_size))
        app.Ry_pos.grid(row=5, column=0, padx=(10,10), pady=5, sticky="nsew")

        app.Rz_pos = customtkinter.CTkLabel(app.joint_positions_frame, text="Rz: "+ str(1929).rjust(7, ' '), font=customtkinter.CTkFont(size=text_size))
        app.Rz_pos.grid(row=6, column=0, padx=(10,10), pady=5, sticky="nsew")

        # Joint positions
        app.joint_positions = customtkinter.CTkLabel(app.joint_positions_frame, text="Joint positions:", font=customtkinter.CTkFont(size=16))
        app.joint_positions.grid(row=0, column=1, padx=(70,0), pady=5, sticky="nsew")

        app.theta1 = customtkinter.CTkLabel(app.joint_positions_frame, text="θ1: " + str(1929).rjust(7, ' '), font=customtkinter.CTkFont(size=text_size))
        app.theta1.grid(row=1, column=1, padx=(70,0), pady=5, sticky="nsew")

        app.theta2 = customtkinter.CTkLabel(app.joint_positions_frame, text="θ2: " + str(1929).rjust(7, ' '), font=customtkinter.CTkFont(size=text_size))
        app.theta2.grid(row=2, column=1, padx=(70,0), pady=5, sticky="nsew")

        app.theta3 = customtkinter.CTkLabel(app.joint_positions_frame, text="θ3: " + str(1929).rjust(7, ' '), font=customtkinter.CTkFont(size=text_size))
        app.theta3.grid(row=3, column=1, padx=(70,0), pady=5, sticky="nsew")

        app.theta4 = customtkinter.CTkLabel(app.joint_positions_frame, text="θ4: " + str(199).rjust(7, ' '), font=customtkinter.CTkFont(size=text_size))
        app.theta4.grid(row=4, column=1, padx=(70,0), pady=5, sticky="nsew")

        app.theta5 = customtkinter.CTkLabel(app.joint_positions_frame, text="θ5: " + str(1929).rjust(7, ' '), font=customtkinter.CTkFont(size=text_size))
        app.theta5.grid(row=5, column=1, padx=(70,0), pady=5, sticky="nsew")

        app.theta6 = customtkinter.CTkLabel(app.joint_positions_frame, text="θ6: " + str(1929).rjust(7, ' '), font=customtkinter.CTkFont(size=text_size))
        app.theta6.grid(row=6, column=1, padx=(70,0), pady=5, sticky="nsew")


        #app.dummy_label1 = customtkinter.CTkLabel(app.joint_positions_frame,text="a", font = customtkinter.CTkFont(size=16, family='TkDefaultFont'))
        app.dummy_label2 = customtkinter.CTkLabel(app.joint_positions_frame,text="", font = customtkinter.CTkFont(size=16, family='TkDefaultFont'))
        app.dummy_label3 = customtkinter.CTkLabel(app.joint_positions_frame,text="", font = customtkinter.CTkFont(size=16, family='TkDefaultFont'))
        #app.dummy_label1.grid(row=0, column=3,  padx=(50,20) ,pady = (5,5),sticky="news")
        app.dummy_label2.grid(row=0, column=4,  padx=(50,20) ,pady = (5,5),sticky="news")
        app.dummy_label3.grid(row=0, column=5,  padx=(50,20) ,pady = (5,5),sticky="news")

        app.Velocity_label = customtkinter.CTkLabel(app.joint_positions_frame,text="JOG velocity", font = customtkinter.CTkFont(size=16, family='TkDefaultFont'))
        app.Velocity_label.grid(row=1, column=3,  padx=(50,20) ,pady = (5,5),sticky="news")

        app.slider1 = customtkinter.CTkSlider(app.joint_positions_frame,from_ = 0, to = 100,number_of_steps=100)
        app.slider1.grid(row=1, column=4,columnspan=1, padx=(20, 10), pady=(5, 5), sticky="news")

        app.Velocity_percent = customtkinter.CTkLabel(app.joint_positions_frame,text="100%", font = customtkinter.CTkFont(size=18, family='TkDefaultFont'))
        app.Velocity_percent.grid(row=1, column=5, padx=5,pady = (5,5),sticky="news")

        app.Accel_label = customtkinter.CTkLabel(app.joint_positions_frame,text="JOG acceleration", font = customtkinter.CTkFont(size=16, family='TkDefaultFont'))
        app.Accel_label.grid(row=2, column=3, padx=(50,20),pady = (5,5),sticky="news")

        app.slider2 = customtkinter.CTkSlider(app.joint_positions_frame,from_ = 0, to = 100,number_of_steps=100)
        app.slider2.grid(row=2, column=4,columnspan=1, padx=(20, 10), pady=(5, 5), sticky="news")

        app.Accel_percent = customtkinter.CTkLabel(app.joint_positions_frame,text="100%", font = customtkinter.CTkFont(size=18, family='TkDefaultFont'))
        app.Accel_percent.grid(row=2, column=5, padx=5,pady = (5,5),sticky="news")

        app.Incremental_jog = customtkinter.CTkLabel(app.joint_positions_frame,text="Incremental jog", font = customtkinter.CTkFont(size=16, family='TkDefaultFont'))
        app.Incremental_jog.grid(row=3, column=3, padx=5,pady = (5,5),sticky="news")

        app.Incremental_jog_button = customtkinter.CTkRadioButton(master=app.joint_positions_frame, text="",  value=2)
        app.Incremental_jog_button.grid(row=3, column=4, pady=10, padx=20, sticky="we")

        app.Incremental_jog_step = customtkinter.CTkLabel(app.joint_positions_frame,text="Step", font = customtkinter.CTkFont(size=16, family='TkDefaultFont'))
        app.Incremental_jog_step.grid(row=4, column=3, padx=5,pady = (5,5),sticky="new")

        app.Step_entry = customtkinter.CTkEntry(app.joint_positions_frame, width= 50)
        app.Step_entry.grid(row=4, column=4, padx=(0, 0),pady=(3,3),sticky="new")

        app.enable_disable = customtkinter.CTkButton(app.joint_positions_frame,text="Enable", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = Enable_press)
        app.enable_disable.grid(row=5, column=3, padx=padx_top_bot,pady = 10,sticky="e")

        app.enable_disable_2 = customtkinter.CTkButton(app.joint_positions_frame,text="Disable", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = Disable_press)
        app.enable_disable_2.grid(row=6, column=3, padx=padx_top_bot,pady = 10,sticky="e")

        app.home = customtkinter.CTkButton(app.joint_positions_frame,text="Home", font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = Home_robot)
        app.home.grid(row=5, column=4, padx=padx_top_bot,pady = 10,sticky="e")


        
    def highlight_words_program(event):
        app.textbox_program.tag_config("green", foreground="green")
        words = ["Delay", "JointMove", "CartLinVelSet","CartAccSet","Loop"]
        for word in words:
            start = "1.0"
            while True:
                start = app.textbox_program.search(word, start, stopindex=tk.END)
                if not start:
                    break
                end = f"{start}+{len(word)}c"
                app.textbox_program.tag_add("green", start, end)
                start = end

    def highlight_words_response(event):
        app.textbox_response.tag_config("red", foreground="red")
        words = ["Warrning", "Error", "Log"]
        for word in words:
            start = "1.0"
            while True:
                start = app.textbox_response.search(word, start, stopindex=tk.END)
                if not start:
                    break
                end = f"{start}+{len(word)}c"
                app.textbox_response.tag_add("red", start, end)
                start = end


    # dodaj slikice kao iz meca studio
    def program_frames():
        #program frame
        app.program_frame = customtkinter.CTkFrame(app,height = 400, width = 550, corner_radius=0, )
        app.program_frame.grid(row=1, column=1, columnspan=2, padx=(5,0), pady=5, sticky="nsew")
        app.program_frame.grid_columnconfigure(0, weight=1)
        app.program_frame.grid_rowconfigure(1, weight=0)
        app.program_frame.grid_rowconfigure(1, weight=1)

        app.program_label = customtkinter.CTkLabel(app.program_frame, text="Program:", font=customtkinter.CTkFont(size=16))
        app.program_label.grid(row=0, column=0, padx=(10,10), pady=5, sticky="w")

        app.textbox_program = customtkinter.CTkTextbox(app.program_frame ,font = customtkinter.CTkFont(size=18, family='TkDefaultFont'))
        app.textbox_program.grid(row=1, column=0,columnspan=2, padx=(20, 20), pady=(5, 20), sticky="nsew")

        app.textbox_program.bind("<KeyRelease>", highlight_words_program)


    def start_stop_frame():
        app.start_stop_frame = customtkinter.CTkFrame(app,height = 30, corner_radius=0, )
        app.start_stop_frame.grid(row=2, column=1, columnspan=2, padx=(5,0), pady=5, sticky="nsew")
        app.start_stop_frame.grid_columnconfigure(0, weight=0)
        #app.response_frame.grid_rowconfigure(1, weight=0)
        #app.response_frame.grid_rowconfigure(1, weight=1)

        app.start = customtkinter.CTkButton(app.start_stop_frame,text="Start",width= 35, font = customtkinter.CTkFont(size=15, family='TkDefaultFont'))
        app.start.grid(row=0, column=0, padx=2,pady = 5,sticky="w")

        app.stop = customtkinter.CTkButton(app.start_stop_frame,text="Stop", width= 35,font = customtkinter.CTkFont(size=15, family='TkDefaultFont'))
        app.stop.grid(row=0, column=1, padx=2,pady = 5,sticky="w")

        app.save = customtkinter.CTkButton(app.start_stop_frame,text="Save",width= 35, font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = save_txt)
        app.save.grid(row=0, column=2, padx=2,pady = 5,sticky="w")

        app.save_as = customtkinter.CTkButton(app.start_stop_frame,text="Save as", width= 35,font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = save_as_txt)
        app.save_as.grid(row=0, column=3, padx=2,pady = 5,sticky="w")

        app.open = customtkinter.CTkButton(app.start_stop_frame,text="Open",width= 35, font = customtkinter.CTkFont(size=15, family='TkDefaultFont'),command = open_txt)
        app.open.grid(row=0, column=4, padx=2,pady = 5,sticky="w")


    # dodaj slikice kao iz meca studio
    def response_log_frames():
        #response log frame
        app.response_frame = customtkinter.CTkFrame(app,height = 100, corner_radius=0, )
        app.response_frame.grid(row=3, column=1, columnspan=2, padx=(5,0), pady=5, sticky="nsew")
        app.response_frame.grid_columnconfigure(0, weight=1)
        app.response_frame.grid_rowconfigure(1, weight=0)
        app.response_frame.grid_rowconfigure(1, weight=1)


        app.program_label = customtkinter.CTkLabel(app.response_frame, text="Response log:", font=customtkinter.CTkFont(size=16))
        app.program_label.grid(row=0, column=0, padx=(10,10), pady=5, sticky="w")

        app.Show_rec_frame = customtkinter.CTkButton(app.response_frame,text="Show received frame", font = customtkinter.CTkFont(size=16, family='TkDefaultFont'))
        app.Show_rec_frame.grid(row=0, column=1, padx=20,pady = 5,sticky="w")

        app.textbox_response = customtkinter.CTkTextbox(app.response_frame,font = customtkinter.CTkFont(size=18, family='TkDefaultFont'))
        app.textbox_response.grid(row=1, column=0,columnspan=2, padx=(20, 20), pady=(5, 20), sticky="nsew")

        app.textbox_response.bind("<KeyRelease>", highlight_words_response)


    #app.textbox_program.bind("<KeyRelease>", highlight_words)

    def commands_frames():
        #commands frame
        app.commands_frame = customtkinter.CTkFrame(app,height = 100,width = 180, corner_radius=0, )
        app.commands_frame.grid(row=1, column=3, rowspan = 3, padx=(5,5), pady=5, sticky="news")
        app.commands_frame.grid_columnconfigure(0, weight=0)

        app.select_current_position = customtkinter.CTkRadioButton(master=app.commands_frame, text="Current position/Pose",  value=2,command = Current_position)
        app.select_current_position.grid(row=0, column=0, pady=10, padx=20, sticky="we")

        app.select_custom_position = customtkinter.CTkRadioButton(master=app.commands_frame, text="Custom positon/Pose",  value=2,command = Custom_position)
        app.select_custom_position.grid(row=1, column=0, pady=10, padx=20, sticky="we")

        app.select_current_position.select()
        #app.commands_frame.grid_propagate(False)
        # https://github.com/TomSchimansky/CustomTkinter/discussions/431
        style = ttk.Style()
        style.theme_use("default")

        style.configure("Treeview",
                        background="#333333",
                        foreground="white",
                        rowheight=40,
                        fieldbackground="#333333",
                        bordercolor="#333333",
                        borderwidth=0,
                        font=('TkDefaultFont',15))
        
        style.map('Treeview', background=[('selected', '#777777')])

        style.configure("Treeview.Heading",
                        background="#1f6aa5",
                        foreground="white",
                        relief="flat",
                        font=('TkDefaultFont',15),
                        )

        
        style.map("Treeview.Heading",
                    background=[('active', '#777777')])

        app.add_menu_display211 = customtkinter.CTkFrame(master=app.commands_frame,
                                                    corner_radius=15,
                                                    height=100,
                                                    width=0)
        
        app.add_menu_display211.grid(pady=3, padx=5, sticky="nws")

        columns = ( 'item')

        app.table = ttk.Treeview(master=app.add_menu_display211,
                        columns=columns,
                        height=30,
                        selectmode='browse',
                        show='headings')
        
        app.table.column("#0", anchor="c", minwidth=200, width=250)
        app.table.column("#1", anchor="c", minwidth=200, width=250)

        app.table.heading('#0', text='test')
        app.table.heading('#1', text='Commands')
        

        Commands_list = ["Home","Delay","End","Loop","JointVelSet","JointAccSet","JointMove","PoseMove","JoinTVelMove",
                            "CartAccSet","CartLinVelSet","CartAngVelSet","CartMove","CartVelMoveTRF","CartVelMoveWRF"]
        # Commands
        Joint_space = app.table.insert(parent = '', index ='end',iid = 0,text="Parent",values="Joint_space")
        Cart_space = app.table.insert(parent = '', index ='end',iid = 1,text="Parent",values="Cartesian_space")
        Conditional_statements = app.table.insert(parent = '', index ='end',iid = 2,text="Parent",values="Conditional_stetements")
        Home = app.table.insert(parent = '', index ='end',iid = 3,text="Parent",values="Home")
        Delay = app.table.insert(parent = '', index ='end',iid = 4,text="Parent",values="Delay")
        End = app.table.insert(parent = '', index ='end',iid = 5,text="Parent",values="End")
        Loop = app.table.insert(parent = '', index ='end',iid = 6,text="Parent",values="Loop")
        IO_var = app.table.insert(parent = '', index ='end',iid = 7,text="Parent",values="IO")
        Gripper = app.table.insert(parent = '', index ='end',iid = 8,text="Parent",values="Gripper")
        Get_data = app.table.insert(parent = '', index ='end',iid = 9,text="Parent",values="Get_data")
        Timeouts = app.table.insert(parent = '', index ='end',iid = 10,text="Parent",values="Timeouts")

        # Joint space commands
        JointVelSet = app.table.insert(Joint_space, index ='end',iid = 100,open = True, text="Child",values="JointVelSet")
        JointAccSet = app.table.insert(Joint_space, index ='end',iid = 101,open = True, text="Child",values="JointAccSet")
        JointMove = app.table.insert(Joint_space, index ='end',iid = 102,open = True, text="Child",values="JointMove")
        PoseMove = app.table.insert(Joint_space, index ='end',iid = 103,open = True, text="Child",values="PoseMove")
        JoinTVelMove = app.table.insert(Joint_space, index ='end',iid = 104,open = True, text="Child",values="JointVelMove")

        # Cart space commands
        CartAccSet = app.table.insert(Cart_space, index ='end',iid = 110,open = True, text="Child",values="CartAccSet")
        CartLinVelSet = app.table.insert(Cart_space, index ='end',iid = 120,open = True, text="Child",values="CartVelSet")
        CartAngVelSet = app.table.insert(Cart_space, index ='end',iid = 130,open = True, text="Child",values="CartAngVelSet")
        CartMove = app.table.insert(Cart_space, index ='end',iid = 140,open = True, text="Child",values="CartMove")
        CartVelMoveTRF = app.table.insert(Cart_space, index ='end',iid = 150,open = True, text="Child",values="CartVelMoveTRF")
        CartVelMoveWRF = app.table.insert(Cart_space, index ='end',iid = 160,open = True, text="Child",values="CartVelMoveWRF")

        # Gripper commands

        app.table.grid(row=2, column=0, sticky='nsew', padx=8, pady=10)

        def select(e):
            selected = app.table.focus()
            logging.debug(selected)
            value = app.table.item(selected,'values')
            logging.debug(value[0])

        app.table.bind('<ButtonRelease-1>', select)


    def Enable_press():
        logging.debug("Enable press")

    def Disable_press():
        logging.debug("Disable press")

    def raise_frame_cart():
        app.cart_frame.tkraise() 
        current_menu = "Cart"
        logging.debug(current_menu)
        
    def raise_frame_jog():
        app.jog_frame.tkraise()
        current_menu = "Jog"
        logging.debug(current_menu)

    def WRF_button():
        app.TRF_select.deselect()
        global Wrf_Trf
        Wrf_Trf = "WRF"
        Jog_control[2] = 1
        logging.debug(Wrf_Trf)

    def TRF_button():
        app.WRF_select.deselect()
        global Wrf_Trf
        Wrf_Trf = "TRF"
        Jog_control[2] = 0
        logging.debug(Wrf_Trf)

    def Current_position():
        app.select_custom_position.deselect()
        global Current_Custom_pose_select
        Current_Custom_pose_select = "Current"
        logging.debug(Current_Custom_pose_select)


    def Custom_position():
        app.select_current_position.deselect()
        global Current_Custom_pose_select
        Current_Custom_pose_select = "Custom"
        logging.debug(Current_Custom_pose_select)

    # save u neki temp file?
    def open_txt():

        global Now_open_txt
        logging.debug("Open txt")
        logging.debug(Now_open_txt)
        app.textbox_program.delete('1.0', tk.END)
        text_file = filedialog.askopenfilename(initialdir = Image_path + "/Programs",title = "open text file", filetypes= (("Text Files",".txt"),))
        logging.debug(text_file)
        Now_open_txt = text_file
        text_file = open(text_file,'r+')
        temp_var = text_file.read()
        app.textbox_program.insert(tk.END,temp_var)
        text_file.close()

    def save_txt():
        logging.debug("Save txt")
        global Now_open_txt
        logging.debug(Now_open_txt)
        if Now_open_txt != '':
            #print("done")
            text_file = open(Now_open_txt,'w+')
            text_file.write(app.textbox_program.get(1.0,tk.END))
            text_file.close()
        else:
            Now_open_txt = Image_path + "/Programs/execute_script.txt"
            text_file = open(Now_open_txt,'w+')
            text_file.write(app.textbox_program.get(1.0,tk.END))
            text_file.close() 
        
    def save_as_txt():
        logging.debug("Save as txt")

        file_path = filedialog.asksaveasfilename(defaultextension=".txt")
        if file_path:
            with open(file_path, "w") as file:
                content = app.textbox_program.get("1.0", tk.END)
                file.write(content)

    def Select_simulator():
        global Robot_sim
        Robot_sim = not Robot_sim 
        if(Robot_sim == 0):
            app.radio_button_sim.deselect()
            Buttons[5] = 0
        else:
            app.radio_button_sim.select()
            Buttons[5] = 1
        logging.debug(Robot_sim)

    def Select_real_robot():
        global Real_robot
        Real_robot  = not Real_robot
        if(Real_robot == 0):
            app.radio_button_real.deselect()
            Buttons[4] = 0
        else:
            app.radio_button_real.select()
            Buttons[4] = 1
        logging.debug(Real_robot)

    def raise_frame_setting():
        app.settings_frame.tkraise()
        current_menu = "Settings"
        logging.debug(current_menu)

    def raise_frame_IO():
        app.IO_frame.tkraise()
        current_menu = "I/O"
        logging.debug(current_menu)

    def raise_calibrate_frame():
        app.Calibrate_frame.tkraise()
        current_menu = "Calibrate"
        logging.debug(current_menu)

    def show_warrning():
        messagebox.showwarning("test","test2")

    def show_error():
        messagebox.showerror("test","test2")

    def show_info():
        messagebox.showinfo("test","test2")

    def Home_robot():
        logging.debug("Home button pressed")
        Buttons[0] = 1

    def Open_help():
        messagebox.showwarning("test","test2")
        messagebox.showerror("test","test2")
        messagebox.showinfo("test","test2")
        logging.debug(Position_in)

    def change_scaling_event(new_scaling: str):
        new_scaling_float = int(new_scaling.replace("%", "")) / 100
        customtkinter.set_widget_scaling(new_scaling_float)

    def change_appearance_mode_event(new_appearance_mode: str):
        customtkinter.set_appearance_mode(new_appearance_mode)

        if(new_appearance_mode == "Dark"):
            style = ttk.Style()
            style.theme_use("default")

            style.configure("Treeview",
                            background="#333333",
                            foreground="white",
                            rowheight=40,
                            fieldbackground="#333333",
                            bordercolor="#333333",
                            borderwidth=0,
                            font=('TkDefaultFont',16))
            
            style.map('Treeview', background=[('selected', '#777777')])

            style.configure("Treeview.Heading",
                            background="#1f6aa5",
                            foreground="white",
                            relief="flat",
                            font=('TkDefaultFont',20),
                            )
            
            style.map("Treeview.Heading",
                        background=[('active', '#777777')])
            
        if(new_appearance_mode == "Light"):
            style = ttk.Style()
            style.theme_use("default")

            style.configure("Treeview",
                            background="#cfcfcf",
                            foreground="Black",
                            rowheight=40,
                            fieldbackground="#cfcfcf",
                            bordercolor="#cfcfcf",
                            borderwidth=0,
                            font=('TkDefaultFont',16))
            
            style.map('Treeview', background=[('selected', '#777777')])

            style.configure("Treeview.Heading",
                            background="#1f6aa5",
                            foreground="white",
                            relief="flat",
                            font=('TkDefaultFont',20),
                            )
            
            style.map("Treeview.Heading",
                        background=[('active', '#777777')])



    # This function periodically updates elements of the GUI that need to be updated
    def Stuff_To_Update():
        
        logging.debug("test")
        
    # Tool positions
    # Use ik to calculate 

        q1 = np.array([PAROL6_ROBOT.STEPS2RADS(Position_in[0],0),
                       PAROL6_ROBOT.STEPS2RADS(Position_in[1],1),
                       PAROL6_ROBOT.STEPS2RADS(Position_in[2],2),
                       PAROL6_ROBOT.STEPS2RADS(Position_in[3],3),
                       PAROL6_ROBOT.STEPS2RADS(Position_in[4],4),
                       PAROL6_ROBOT.STEPS2RADS(Position_in[5],5),])
        #print(q1)
        T = PAROL6_ROBOT.robot.fkine(q1)
        b = T.t # get translation component
        print(b*1000)
        robot_pose[0] = b[0] * 1000
        robot_pose[1] = b[1] * 1000
        robot_pose[2] = b[2] * 1000
        robot_pose[3:] = T.rpy('deg','xyz') # get rotation component
  
        app.x_pos.configure(text="X: "+ str(round(robot_pose[0],3)).rjust(7, ' '))	
        app.y_pos.configure( text="Y: "+ str(round(robot_pose[1],3)).rjust(7, ' '))	
        app.z_pos.configure(text="Z: "+ str(round(robot_pose[2],3)).rjust(7, ' '))	
        app.Rx_pos.configure(text="Rx: "+ str(round(robot_pose[3],3)).rjust(7, ' '))	
        app.Ry_pos.configure(text="Ry: "+ str(round(robot_pose[4],3)).rjust(7, ' '))	
        app.Rz_pos.configure(text="Rz: "+ str(round(robot_pose[5],3)).rjust(7, ' '))	

    # Joint positions
        app.theta1.configure(text="θ1: " + str(round(PAROL6_ROBOT.STEPS2DEG(Position_in[0],0),3)).rjust(7, ' '))
        app.theta2.configure(text="θ2: " + str(round(PAROL6_ROBOT.STEPS2DEG(Position_in[1],1),3)).rjust(7, ' ')) 
        app.theta3.configure(text="θ3: " + str(round(PAROL6_ROBOT.STEPS2DEG(Position_in[2],2),3)).rjust(7, ' '))
        app.theta4.configure(text="θ4: " + str(round(PAROL6_ROBOT.STEPS2DEG(Position_in[3],3),3)).rjust(7, ' '))
        app.theta5.configure(text="θ5: " + str(round(PAROL6_ROBOT.STEPS2DEG(Position_in[4],4),3)).rjust(7, ' '))
        app.theta6.configure(text="θ6: " + str(round(PAROL6_ROBOT.STEPS2DEG(Position_in[5],5),3)).rjust(7, ' '))
        
        prev_positions[0] = Position_in[0]
        prev_positions[1] = Position_in[1]
        prev_positions[2] = Position_in[2]
        prev_positions[3] = Position_in[3]
        prev_positions[4] = Position_in[4]
        prev_positions[5] = Position_in[5]

        # Sliders
        v1 = app.slider1.get()
        v2 = app.slider2.get()
        #Jog_control[0] = v1
        #Jog_control[1] = v2
        #logging.debug(Jog_control)
        app.Velocity_percent.configure(text= ""+ str(v1).rjust(4, ' ')+ "%")
        app.Accel_percent.configure(text= "" + str(v2).rjust(4, ' ') + "%")


        # If tab is joint jog
        # Update joint sliders
        for y in range(0,6):
            app.progress_bar_joints[y].set(0.1)
            None
    
        app.after(66,Stuff_To_Update) # Update data every 66 ms ( 15 frames per second)



    #Stuff_To_Update(self)
    top_frames()
    bottom_frames()
    robot_positions_frames()
    program_frames()
    response_log_frames()
    commands_frames()
    cart_jog_frame()
    joint_jog_frames()
    settings_frame()
    IO_frame()
    start_stop_frame()
    Calibrate_frame()
    app.jog_frame.tkraise()

    Stuff_To_Update()



    app.mainloop() 


    


if __name__ == "__main__":

    #DUMMY DATA
    
    # Data sent by the PC to the robot
    Position_out = [1,11,111,1111,11111,10]
    Speed_out = [2,21,22,23,24,25]
    Command_out = 69
    Affected_joint_out = [1,1,1,1,1,1,1,1]
    InOut_out = [1,1,1,1,1,1,1,1]
    Timeout_out = 123
    #Positon,speed,current,command,mode,ID
    Gripper_data_out = [1,1,1,1,1,1]

    # Data sent from robot to PC
    Position_in = [31,32,33,34,35,36]
    Speed_in = [41,42,43,44,45,46]
    Homed_in = [1,1,1,1,1,1,1,1]
    InOut_in = [1,1,1,1,1,1,1,1]
    Temperature_error_in = [1,1,1,1,1,1,1,1]
    Position_error_in = [1,1,1,1,1,1,1,1]
    Timeout_error = 123
    # how much time passed between 2 sent commands (2byte value, last 2 digits are decimal so max value is 655.35ms?)
    Timing_data_in = 123
    XTR_data =   123

    #ID,Position,speed,current,status,obj_detection
    Gripper_data_in = [110,120,130,140,150,160]

    # GUI control data
    Joint_jog_buttons = [0,0,0,0,0,0,0,0,0,0,0,0]
    Cart_jog_buttons = [0,0,0,0,0,0,0,0,0,0,0,0]
    # Speed slider, acc slider, WRF/TRF
    Jog_control = [0,0,1,0]
    # COM PORT, BAUD RATE, 
    General_data = [8,3000000]
    # Home,Enable,Disable,Clear error,Real_robot,Sim_robot
    Buttons = [0,0,0,0,1,1]
    

    GUI(Position_out,Speed_out,Command_out,Affected_joint_out,InOut_out,Timeout_out,Gripper_data_out,
         Position_in,Speed_in,Homed_in,InOut_in,Temperature_error_in,Position_error_in,Timeout_error,Timing_data_in,
         XTR_data,Gripper_data_in,
        Joint_jog_buttons,Cart_jog_buttons,Jog_control,General_data,Buttons)
    

