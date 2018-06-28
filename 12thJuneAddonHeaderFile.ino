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

       
SD.remove("newList.csv");



fileAddon();
}



void loop()
{
  
}



void fileAddon()
{
  int endOLine;//counts nber of comma that makes up a line
  int dayCounter=0;//count nber of days since the device was in operation....to know how many addons were added
  int commaCounter=0;//count nber of commas on the line
  int line=1;//count the nber of lines in the file

  //strings to be used to create a new file
  String tempStr={};
  String listStr={};//copy line from the list.csv
  String addStr={};//string will hold the addOn
  String downString={};//hold data to be transfered to the downloadable file


  //knowing the end of the line....using the number of commas presented in line
    //read the dayCounter from the EEPROM memory
   // dayCounter=EEPROM.read(911);
    endOLine=4+(4*dayCounter);//initially the file is madeup of 4 commas per line, 4new commas will be added at each new day

    //open the list.csv in writing mode
  
   File fileEdit1=SD.open("list.csv");
    Serial.print("size1:");
    Serial.println(fileEdit1.size());
    if(fileEdit1)
    {
      
      while(fileEdit1.available())
      {
        char x=fileEdit1.read();
       Serial.print(x);
        tempStr+=x;
        if(x==',')
          {
           commaCounter++;
           //if the file reaches its ends then add the new day's addon
           if(commaCounter==endOLine)
           {
            
            
             if(line ==1)
             {
              //ADD ",date,,comment,"

              listStr=tempStr;//hold first line of the list.csv
              addStr=",TIME,,,";
              downString=listStr+addStr;
              Serial.print("downString1:");
              Serial.println(downString);
              
              File newFile= SD.open ("newList.csv",FILE_WRITE);
              if(newFile)
              {
              
                newFile.print(downString);
                downString="";//clear
                tempStr="";//clear the string 
                listStr="";//
                newFile.close();
              }
              
              
              
             }
             if(line==2)
             {
             
             
              listStr=addStr=downString="";//clear the strings
              
              listStr=tempStr;//hold first line of the list.csv
              addStr=",TIME IN,TIME OUT,COMMENT,";
              downString=listStr+addStr;

              File newFile=SD.open ("newList.csv",FILE_WRITE);
              if(newFile)
              {
                newFile.print(downString);
                downString="";//clear
                tempStr="";//clear the string 
                listStr="";//
                newFile.close();
              }
              
              
             }
             
             line++;
             commaCounter=0;
           }
           
          }
      }
      //fileEdit.flush();//make sure the changes were saved
      fileEdit1.close();


      
    

        dayCounter++;
}
}
