import serial
import time
import csv

dataFromArduino = serial.Serial('COM5',9600)

msg = ''

timeIn = ' '
timeOut = ' '



def fromSerial(msg):
    while True:
        for c in dataFromArduino.read():
            msg += chr (c)
            print (msg)
            #if file already exists then delete to create a new
            with open ( 'todayList.csv','a') as file:
                file.write(chr (c))
            
            if chr (c) == ']':
                print('message:')
                print (msg)
                break;

            
 


def parseList():
    buff = ''
    line_Number = 0

    with open ( 'list.csv','r') as file:
        for line in file:
        
            if line_Number == 0:
                line2 = line[0:len(line)-2] + ',DATE'+ line[len(line)-1]
                buff += line2;
            elif line_Number == 1:
                line2 = line[0:len(line)-2] + ',TIME IN ,TIME OUT,COMMENT'+ line[len(line)-1]
                buff += line2;
            elif line_Number >1:
                generatedCode = generateCode(line_Number)

                buff += joinFiles(generatedCode,line)
                
                
            line_Number = line_Number + 1
           
        else:
            print('end of file')
        print(buff)
        file.close()

    with open ( 'list.csv','w') as file:
        file.write(buff)
        file.close()
    


def generateCode(line):
    generatedCode = 0
    if  ((2<=line) and (line <11)):
        return ('11' + str (line-1) )
    
    else:
        return str (0)
        
        

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

            line2 = listLine[0:len(listLine)-2] +','+ str (timeIn) + ','+str (timeOut) + listLine[len(listLine)-1] 
            
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


               
             
def main() :
    #fromSerial(msg)
    #readFile()
    parseList()

if __name__ =="__main__":
    main()

