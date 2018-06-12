#include <EEPROM.h>
#include "RTClib.h"//for rtc clock
RTC_DS3231 rtc;
RTC_Millis rtc1;//to take time from software based on millis()and timer

#include <SD.h>
#define chipSelect 53

//declaring the variables
  char getName[8];//date made up of 2digits, month 2 digits and year 4 digits to make 8 characters


void setup()
{
  Serial.begin(9600);
  delay(10);

  //-------------SETTING SD CARD-----------------------------
  //SD card settings
  pinMode(chipSelect,OUTPUT); //declare the CS as an OUTPUT
  //SD.begin(chipSelect);
  
  if(!SD.begin(chipSelect))// 53 is chipselect on arduino mega
  {
    Serial.println("SD card not working");
  }
  else
  Serial.println("SD card  working");

  //---------------SETTING RTC CLOCK------------------------------
  if(rtc.begin())
  {
  Serial.println("RTC working");
  // following line sets the RTC to the date & time this sketch was compiled
    rtc1.begin(DateTime(F(__DATE__), F(__TIME__)));
   //or set  January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    
  }
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  for(int i=0;i<5;i++)
  Serial.println();

  
  todayDate();
  dailyAttendanceOp();//operational function on the daily attendance file...+ to list.csv
  readList();//read list.csv
  
}

void loop()
{
  
}

void readList()
{
  File listFile=SD.open("list.csv");//in both writing and reading mode
    Serial.println("LIST.CSV CONTENT");
    Serial.println("----------------");
    if(listFile)
    {
      while(listFile.available())
      {
        char x=listFile.read();
        Serial.print(x);
      }
      listFile.close();
      Serial.println("");
    }
    
    Serial.println("-----------------------");
    Serial.println("   END OF THE LIST.CSV  ");
    Serial.println("-----------------------");
    

}

void dailyAttendance()
{
  
  char getNameExt[15];//get today's file name with extension(.csv)

  todayDate();//get the today's date
  
  sprintf(getNameExt,"%s.csv",getName); 
  File attendFiles=SD.open(getNameExt);//both writing and reading mode
  Serial.print("getName:");
  Serial.println(getName);
  Serial.print("getNameExt:");
  Serial.println(getNameExt);
  
  Serial.println("today's date attendance .CSV CONTENT");
    Serial.println("----------------");
    if(attendFiles)
    {
      while(attendFiles.available())
      {
        char x=attendFiles.read();
        Serial.print(x);
      }
      attendFiles.close();
    }
    
    Serial.println("-----------------------");
    Serial.println("   END OF THE today's date attendance.CSV  ");
    Serial.println("-----------------------");
}

//save the today's date (day,date and year) into a string variable
void todayDate()
{
  
      DateTime now=rtc1.now();//declare the rtc variable type to fetch date,month and year
      sprintf(getName,"%d%d%d",now.day(),now.month(),now.year());
      
      
}


//------WITH THE DAILY ATTENDANCE FILE WE GOING BE
//-------GETTING THE LINE NUMBER , FINGERPRINT PRESSED TIMES AND THE TIME THE FINGER WAS PRESSED
//-------------IF FINGER DETECTED ONCE IN THE dailyAttendance file MEANS TIME IN
//-------------IF TWICE DETECTED THEN TIME OUT
//-------------ELSE IGNORE THE OTHER TIMES....ONLY TWO FIRST PRESSES ARE CONSIDERED

void dailyAttendanceOp()//some operations on the dailyAttendance file
{
  long attRegNo[150]={};//keep track of the users attended today
  int pressCounter[150]={};//keep number of times users have pressed their fingerprint while attending to know if it is time in or time out
  String str={},str2={};//to hold the regNo and the time attendend
  int counter=0;//count number of the regNo...registration numbers
  int lastVals;//hold the last digits of the regNo
  
  //-------open the today's attendance(contains the regNo the time the attendance was due----------------------------------
  //get today's date as the name of the file
   char getNameExt[15];//get today's file name with extension(.csv)
   String recTime,recRegNo;//keep recorded time and recorded registration number
int depStart[20]={0,9,22,26,30,32,39,53,59,61,63,69,72,77,78,79,82,85,92,98};//which line each department starts from in the file list.csv
   int lineNber;

  todayDate();//get the today's date
    sprintf(getNameExt,"%s.csv",getName);
    
    File todayFile=SD.open(getNameExt);//open the file in both mode
      Serial.print("getName:");
      Serial.println(getName);
      Serial.print("getNameExt:");
      Serial.println(getNameExt);
      if(todayFile)//if the file stream is well opened
      while(todayFile.available())//while the file is availble to be read....End not yet reached
      {
        char x=todayFile.read();//read character per character
        Serial.print(x);
        
        //read the regNo before "(" and save it into str1; 
        if(x=='(')
        {
          //convert the string into numbers...into long datatype
          Serial.print("value of str");
          Serial.println(str);
          long regNo=str.toInt();
          Serial.print("regNo got from the today's file:");
          Serial.println(regNo);
          //means the regNo is registered in the str
          //compare it with the arleady attended regNo
          for(int i=0;i<150;i++)
          {
            if(attRegNo[i]==regNo)//means this time is the TIME-OUT time
            {
              if(pressCounter[i]>2)//as 2 means TIME-OUT and 1 TIME-IN
              {
                
                break;
              }
              
              
              pressCounter[i]=pressCounter[i]+1;
              Serial.print("pressed Times:");
              Serial.println(pressCounter[i]);
              break;
            }
             if(i==149)//means no match found for the regNo in the attRegNo[150]...so record this too
            {
              attRegNo[counter]=regNo;
              counter++;
              pressCounter[counter]=1;//has attended once
              Serial.print("pressed Times:");
              Serial.println(pressCounter[counter]);
            }
          }

//------------------GET THE FIRST AND THE LASTS OBTAINED regNo..ex:111 or 4314---------------

         //create an array type to take apart the digits in the regNo
         int arr[4]={};//as 4 is the max. digits that made up regNo
         long number=regNo;
         int j=3;//index counter
         int  count=0;

   while(number!=0)
  {
    arr[j]=number%10;//keep the remainder
    number=(int)number/10;
    j--;
    count++;//count the number of digits in a number
    
  }
  if(count==3)//with 3 inputs we will need to adjust to fit in the 4 size array;
  {
    arr[0]=arr[1];
    arr[1]=arr[2];
    arr[2]=arr[3];
    arr[3]=0;
  } 
  //else arr[3] has something other than the random value    

    //--------GET THE LAST 2 VALUES---------------------------------------------------------
    if(count==3)//for the 3 digits regNo
    lastVals=arr[2];
    
    else if(count>3)//for the 4 digits regNo
    lastVals=arr[2]*10+arr[3];//signifies the line nber of the person in the department file

    Serial.print("first value");
    Serial.println(arr[0]);
    Serial.print("last vals");
    Serial.println(lastVals);
    //---------------------------------------------------------------------------------------
   //------KNOW WHICH LINE NUMBER IN THE list.csv IS THE PERSON WITH regNO=str found?
   int startLine=3;//which line in the list.csv, the names starts to appear
   
   lineNber=startLine+depStart[arr[0]-1]+lastVals;  
   Serial.print("lineNber:");
   Serial.println(lineNber); 
   str="";//clear the str 
   continue;//do not record the '(' character into string str  
        }
        
        //reading the time enclose btn"(" and ")"
        if(x==')')
        {
         str2=str;
         Serial.print("time:");
         Serial.println(str2);
         str="";//clear the str
        }
        if(x==',')
        {
          str="";//clear the str
        continue;
        }
        str+=x;
      }
 //---------------------------------------------------------------------------------------------


  //now pass the line number of the user in list.csv, pressed times and the time has attended to create a complete daily file
  downloadableFile(lineNber,pressCounter[counter-1],str2);
 }

//------MAKE USE OF THE DATA GOTTEN FROM THE dailyAttendanceOp() TO CREATE A DOWNLOADABLE FILE
//....(LINE NUMBER,PRESSED TIMES AND THE ATTENDANCE WAS DUE)
//---------- ATTACHING THE dailyAttendanceop() data to the list.csv

void downloadableFile(int line,int pressedTimes,String Time)
{
  int commaCounter=0;
 // int dayCounter=EEPROM.read(911);
  int dayCounter=0;//suppose a one day attendance file
  int endOLine=4+(4*dayCounter);//initially the file is madeup of 4 commas per line, 4new commas will be added at each new day
  int lineCounter=0;

  Serial.print("-----------------------REACHED INSIDE------------");
  //open the list.csv
  File fileEdit = SD.open("list.csv");

  if(fileEdit)
  {
    Serial.println("FILE well opened");
    while(fileEdit.available())
    {
      char x=fileEdit.read();
      if(x==',')
      {
        commaCounter++;
        if(commaCounter==endOLine)//end of the line must have commas
        {
         
          
          if((lineCounter==line)&&(pressedTimes==1))//TIME-IN
          {
            //how many pressedTimes?
          
            fileEdit.print(",");
            fileEdit.print(Time);
            fileEdit.print(",");
            }
        }
        
        if((commaCounter=(endOLine+2))&&(pressedTimes==2)&&(lineCounter==line))//TIME-OUT
          {
            fileEdit.print(Time);
          fileEdit.print(",,");
          }
      }
      
    }
    fileEdit.close();
    Serial.println("FILE CLOSED");
    
      
  }
}

