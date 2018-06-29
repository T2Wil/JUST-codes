import os 
from  interfaceFile import *
from tkinter import *

import os
import serial
import csv
from time import *
comm_port = ''


def create_text_box(root):
    
    global data
    data = Entry(root)
    data.pack()
    data.focus_set()


def get_input_from_text_box_button():
    global comm_port
    
    comm_port= data.get()# reading from line 1 char 0 to end
    del_previous_attendance_Record()
    msg = ' '
    get_Attendance_Record_File_From_Serial(msg,comm_port)

    recorded_date = recorded_Date_when_data_first_downloaded()
    today_Date = today_date()
    print('recorded_date:',recorded_date)
    print('today_date:',today_Date)
    returnedComparison = compare_Dates(recorded_date,today_Date)
    print('returnedcomparison:',returnedComparison)
    if (returnedComparison == 0):
        parseList()
    elif (returnedComparison == 1):
        pass

def write_title(root,title):
    root.title (title)

def write_text_label(root,input_text,label_side):
    label = Label(root, text = input_text)
    label.pack(side = label_side)

def create_input_button(root,button_name): #function:get_input_from_text_box_button

    button = Button(root, text=button_name, command = get_input_from_text_box_button)
    button.pack()

def get_the_file_directory(file_name):
    cwd = ''
    
    cwd = os.getcwd()
    cwd = cwd + '\\'+ file_name
    return cwd

def open_the_attendance_file(directory):
    os.system(directory)

def open_the_record_button_function():

    file_name = 'AttendanceRecord.csv'
    directory = get_the_file_directory(file_name)
    open_the_attendance_file(directory)
    
    
def get_the_attendance_record_button(root,button_name):
    
    button = Button(root, text=button_name, command = open_the_record_button_function)
    button.pack()

def main() :
    
    root = Tk()
    
    title = "RECORDED ATTENDANCE"
    write_title(root,title)

    message_label= 'COM PORT:'
    message_label_side = 'top'
    write_text_label(root,message_label,message_label_side)

    create_text_box(root)

    button_name = 'CONNECT THE DEVICE WITH THE PC'
    create_input_button(root,button_name)
##-------------------------------------------------------------------------------
##    del_previous_attendance_Record()
##    get_Attendance_Record_File_From_Serial(msg)
    
##------------------------------------------------------------------------------
    button_name = 'OPEN THE RECORD'
    get_the_attendance_record_button(root,button_name)

    
    root.mainloop()
    
    
    print("reahced")
    

    
    
   
if __name__ =="__main__":
    main()

