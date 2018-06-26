import serial
import time
import csv
import os 
from time import *
dataFromArduino = serial.Serial('COM5',9600)

msg = ''

timeIn = ' '
timeOut = ' '


def del_previous_attendance_Record():
    attendance_Record = 'todayList.csv'

    if os.path.isfile(attendance_Record):
        print('file exists\n....so deleted')
        os.remove(attendance_Record)
             
    
def get_Attendance_Record_File_From_Serial(msg):
    attendance_Record = 'todayList.csv'
    status = True

    while (status != False):
        for c in dataFromArduino.read():
            msg += chr (c)

            print (msg)
            
            
        if (chr (c) == ']'):
            print('termination of the program')
            with open ( attendance_Record,'a') as file:
                file.write(msg[:-1])
            break


              
    
                                  
                                
     
def parseList():
    
    buff = ''
    line_Number = 0

    date_when_data_first_downloaded()

    yesterdayFile= 'list.csv'
    
    with open ( 'list.csv','r') as file:
        print('reading the file')
        for line in file:
        
            if line_Number == 0: 
                line2 = line[0:len(line)-2] + ','+today_date()+','+','+','+ line[len(line)-1]
                buff += line2;
            elif line_Number == 1:
                line2 = line[0:len(line)-2] + ' ,TIME IN ,TIME OUT,COMMENT'+ line[len(line)-1]
                buff += line2;
            elif line_Number >1:
                generatedCode = generateCode(line_Number)

                buff += joinFiles(generatedCode,line)
                
                
            line_Number = line_Number + 1
           
        else:
            print('end of file')
        print(buff)
        file.close()

    with open ( 'AttendanceRecord.csv','w') as file:
        file.write(buff)
        file.close()
    


def generateCode(line):
    generatedCode = 0
    if  ((2<=line) and (line <=10)):
        return ('11' + str (line-1) )
    
    elif ((11<=line) and (line <=13)): 
        return ('21' + str (line-10) )

    elif ((14<=line) and (line <=17)): 
        return ('31' + str (line-13) )

    elif ((18<=line) and (line <=21)): 
        return ('32' + str (line-17) )

    elif ((22<=line) and (line <=23)): 
        return ('41' + str (line-21) )

    elif ((24<=line) and (line <=30)): 
        return ('42' + str (line-23) )

    elif ((31<=line) and (line <=44)): 
        return ('43' + str (line-30) )

    elif ((45<=line) and (line <=50)): 
        return ('44' + str (line-44) )

    elif ((51<=line) and (line <=52)): 
        return ('51' + str (line-50) )

    elif ((53<=line) and (line <=54)): 
        return ('52' + str (line-52) )

    elif ((55<=line) and (line <=60)): 
        return ('53' + str (line-54) )

    elif ((61<=line) and (line <=63)): 
        return ('54' + str (line-60) )

    elif ((64<=line) and (line <=68)): 
        return ('55' + str (line-63) )

    elif (line == 69): 
        return ('61' + str (line-68) )

    elif (line == 70): 
        return ('62' + str (line-69) )

    elif ((71<=line) and (line <=73)): 
        return ('63' + str (line-70) )

    elif ((74<=line) and (line <=76)): 
        return ('64' + str (line-73) )

    elif ((77<=line) and (line <=83)): 
        return ('65' + str (line-76) )

    elif ((84<=line) and (line <=89)): 
        return ('71' + str (line-83) )

    elif ((90<=line) and (line <=95)): 
        return ('81' + str (line-89) )

    else:
      return str(0)

def joinFiles(generatedCode,listLine):
    
    count=0
    timeIn = ''
    timeOut = ''
    #print('generatedCd:' + generatedCode)
    with open('todayList.csv','r') as attFile:
         for x in attFile:
            #print(x)
            myArray=x.split(',');
            codes=getCodes(int (generatedCode),myArray)
            for code in codes:
                
                count+=1
                if(count==1):
                    timeIn=(code.split('(')[1]).split(')')[0] + ('  ')
                else:
                    timeOut=(code.split('(')[1]).split(')')[0] + ('  ')

            line2 = listLine[0:len(listLine)-2] +','+ str (timeIn) + ','+str (timeOut) +','+','+ listLine[len(listLine)-1] 
            
            return line2
            


 
def getCodes(generatedCode, array):
    i=0
    codes=[]
    for code in array:
        #print('code: '+code)
        tempCode =code.split('(')[0]
        if(code!= array[-1]):
            if(int(tempCode)== generatedCode):
                codes.append(code)
                i=i+1
    return codes


def recent_Attendance_Record_date():
    recent_download = 'dateRecord.txt'

    dateNow = ''
    
    if os.path.isfile(recent_download):
        print('file exists\n....so read the date in the file')
        with open ('dateRecord.txt','r') as datefile:
            dateNow = datefile.read()
        return dateNow

        

def today_date():
    return (strftime("%d/%m/%Y",gmtime()))

def compare_Dates(date1,date2):
    if (date1 == date2):
       return 1
    else:
        return 0
      

def date_when_data_first_downloaded():
     timeNow = today_date()
     with open ('dateRecord.txt','w') as datefile:
        datefile.write(timeNow)
        


def recorded_Date_when_data_first_downloaded():
    recent_download = 'dateRecord.txt'

    dateNow = ''
    
    if os.path.isfile(recent_download):
        print('file exists\n....so read the date in the file')
        with open ('dateRecord.txt','r') as datefile:
            dateNow = datefile.read()
        
        return dateNow
    else:
        pass
    
def main() :

    del_previous_attendance_Record()
    get_Attendance_Record_File_From_Serial(msg)
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

    
    
    
    
##    parseList()

if __name__ =="__main__":
    main()

