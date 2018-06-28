#include <EEPROM.h>
#include "RTClib.h"//for rtc clock
RTC_DS3231 rtc;
RTC_Millis rtc1;//to take time from software based on millis()and timer

#include <SD.h>
#define chipSelect 53

//declaring the variables
  char getName[8];//date made up of 2digits, month 2 digits and year 4 digits to make 8 characters


 String regFile={};//will be holding the characters read from the attendance file
char getNameExt[15];//get today's file name with extension(.csv)
   String t1={},t2={};//to hold the TIME-IN AND THE TIME OUT recorded
   int count=0;


 
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
downloadFile();
}

void loop() {
  // put your main code here, to run repeatedly:

}

void downloadFile()
{
  //DOWNLOADABLE FILE=STR3=STR1+STR2
  //STR1 IS GOTTEN FROM THE newList.csv
  //STR2 IS GOTTEN FROM THE today's attendance file.csv...(time in + time out)
  //STR3 =STR1+STR2 makes a line of the new desirable file downFile.csv

String str1={};
String str2={};
String str3={};

int commaCounter=0;
int dayCounter=1;
int line=1;
String regStr;

Serial.println("for testing");
  //STEP1: get str1...line by line FROM THE newList.csv
  //----------------------------------------------------------------------------------------
  File fileStr1=SD.open("newList.csv",FILE_READ);
  if(fileStr1)
  {
    while(fileStr1.available())
    {
      char x=fileStr1.read();
      str1+=x;
      //Serial.print(x);
      if(x==',')
      commaCounter++;
      
      if(commaCounter==(4*dayCounter +4))//on the first two lines
      { 
        //append to a newFile3.csv
  //--------------------------------------------------------------------------------------
        File fileEdit=SD.open("newFile3.csv",FILE_WRITE);
    if(fileEdit)
    {
      fileEdit.print(str1);
      str1="";//clear the str1
      fileEdit.close();
      
    }
 //---------------------------------------------------------------------------------------------   
        commaCounter=0;
        line++;
      }
       if((commaCounter==(4*dayCounter))&&(line>2))//on the remained lines
      { 
      

      if((line>2)&&(line<=9))
      {
        //line=line-2;
        regStr="11"+(String)line;
      }
      if((line>9)&&(line<=12))
      {
        
        regStr="21"+(String)line;
      }
      if((line>12)&&(line<=16))
      {
        regStr="31"+(String)line;
      }
      
      if((line>16)&&(line<=20))
      {
        regStr="32"+(String)line;
      }
      if((line>20)&&(line<=22))
      {
        regStr="41"+(String)line;
      }
      if((line>22)&&(line<=29))
      {
        regStr="42"+(String)line;
      }
      if((line>29)&&(line<=42))
      {
        regStr="43"+(String)line;
      }
      if((line>42)&&(line<=48))
      {
        regStr="44"+(String)line;
      }
      if((line>48)&&(line<=50))
      {
        regStr="51"+(String)line;
      }
      if((line>50)&&(line<=52))
      {
        regStr="52"+(String)line;
      }
      if((line>52)&&(line<=58))
      {
        regStr="53"+(String)line;
      }
      if((line>58)&&(line<=61))
      {
        regStr="54"+(String)line;
      }
      if((line>61)&&(line<=66))
      {
        regStr="55"+(String)line;
      }
      if(line==67)
      {
        regStr="61"+(String)line;
      }
      if(line==68)
      {
        regStr="62"+(String)line;
      }
      if((line>68)&&(line<=71))
      {
        regStr="63"+(String)line;
      }
      if((line>71)&&(line<=74))
      {
        regStr="64"+(String)line;
      }
      if((line>74)&&(line<=81))
      {
        regStr="65"+(String)line;
      }
      if((line>81)&&(line<=87))
      {
        regStr="71"+(String)line;
      }
      if((line>87)&&(line<=93))
      {
        regStr="81"+(String)line;
      }

      Serial.print("regStr");
      Serial.println(regStr);
     

    //STEP2: get str2...from the today's attendance
  //----------------------------------------------------------------------------------------
  

  //open the today's attendance file
  
  todayDate();//get the today's date
  
  sprintf(getNameExt,"%s.csv",getName); 
  File attendFiles=SD.open(getNameExt);//both writing and reading mode

 
  if(attendFiles)
    {
      while(attendFiles.available())
      {
        char x=attendFiles.read();
        
       if(regFile==regStr)//if the regNo in the attendance file == to the regNo in the newList.csv..chance to find out the both time in and the time out
       {
        
        regFile="";//empty the string to store the time attended
        count++;
       }
       
       if((x=')')&&(count==1))
       {
        t1=regFile;
        regFile="";
       }
       
       if((x=')')&&(count>1))//get to know when he/she was lastly attended
        {
        
          t2=regFile;
          regFile="";
          count++;
        }
         
             

        regFile+=x;
      }
      attendFiles.close();

      //Now getting the STR2
      str2=","+
      t1+","+ t2+",,";
      Serial.println("str2=t1+t2");
      Serial.print("t1:");
      Serial.println(t1);
      Serial.print("t2:");
      Serial.println(t2);
      Serial.print("str2");
      Serial.println(str2);
      str3=str1+str2;
      
      //append to a newFile3.csv
            //append to a newFile3.csv
  //--------------------------------------------------------------------------------------
        File fileEdit=SD.open("newFile3.csv",FILE_WRITE);
    if(fileEdit)
    {
      fileEdit.print(str3);
      fileEdit.close();
     }

     str2="";
      str1="";
      str3="";

      
 //---------------------------------------------------------------------------------------------   
    
    }
  
 //-----------------------------------------------------------------------------------------
      line++;
      commaCounter=0;
      }
    }
    fileStr1.close();
  }
  //----------------------------------------------------------------------------------------

  
  
}


//save the today's date (day,date and year) into a string variable
void todayDate()
{
  
      DateTime now=rtc1.now();//declare the rtc variable type to fetch date,month and year
      sprintf(getName,"%d%d%d",now.day(),now.month(),now.year());
      
      
}
