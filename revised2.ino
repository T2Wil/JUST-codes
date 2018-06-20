
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include "RTClib.h"//for rtc clock
#include <avr/pgmspace.h>
#include <SD.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>

#define chipSelect 53

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(29,31,33,35,37,39,41,43,45,47);//lcd in 8 bit mode

RTC_DS3231 rtc;
RTC_Millis rtc1;//to take time from software based on millis()and timer

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
//defining the keypad symbols
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D',}
};
byte rowPins[ROWS] = {32,34,36,38}; //connect to the row pinouts of the kpd
byte colPins[COLS]= {40,42,30,28}; //connect to the column pinouts of the kpd

//initialize an instance of class NewKeypad
Keypad keypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

SoftwareSerial mySerial(10,11);//Tx,Rx pins of mega, GREEN,YELLOW

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);



static long address;
int userCount;

static int userCounter=0;

char key;

  int i;//for iteration

 char daysOfTheWeek[7][12] = {"Sund", "Mond", "Tues", "Wedn", "Thur", "Frid", "Satu"};


//for fingerprint to eeprom
static int id=0;//takes all the users'fingerprint

static int userLoc2=60;//location starting for userents fingerprints ID's



int fileNames[20]={11,21,31,32,41,42,43,44,51,52,53,54,55,61,62,63,64,65,71,81};//20 files 
String fName;

int dayCounter;

//userent's records

struct users{
        
     
        PROGMEM int userID[30];//take the number of userents userRegisteringed consecutively
        PROGMEM long userCode[30];
        PROGMEM int prints[30];//will take the value of the fingerprint scanned
        PROGMEM int pressedTimes[30]={};//record how many times does a user attempted to attend(either 
        }user;


void setup() {

  lcd.begin(20,4);
  Serial.begin(9600);
   Serial.println("CLEARSHEET");

  pinMode(chipSelect,OUTPUT); //declare the CS as an OUTPUT

  if(!SD.begin(chipSelect))// 53 is chipselect on arduino mega
  {
    Serial.println("SD card not working");
  }
  else
  Serial.println("SD card  working");
  
  //CHECKING THE RTC MODULE ON THE BOARD
  if(rtc.begin())
  {
  Serial.println("RTC working");
    rtc1.begin(DateTime(F(__DATE__), F(__TIME__)));
   
  }
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  // set the data rate for the sensor serial port
  finger.begin(57600);
  //----------------------------------------------FINGERPRINT SCANNER PRESENCE TEST----------------------------------------------------
  if (finger.verifyPassword()) 
  {
   
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print(F("MINISTRY OF"));
    lcd.setCursor(4,1);
    lcd.print(F(" JUSTICE"));
    delay(2000);
  } else 
  {
    
    lcd.print(F("FINGERPRINT"));
    lcd.print(F(" NOT FOUND!"));
    while (1) { delay(1); }
  }

 //--------------------------------------------------------------------------------------

//----------------------RESTORE USERS PRINTS FROM EEPROM-------------------------------
 
 if(EEPROM.read(226)>60)//TO PREVENT THE LOSS OF THE INITIAL VALUES
  userLoc2=EEPROM.read(226);

          int r2=60;
 for(int z=0;z<userCounter;z++)//index counter for the userents 
      {
       
        while(r2<=userLoc2)
        {
          
          user.prints[z]=EEPROM.read(r2);
          
          break;
          
        }
      
        r2++;
       
    }
//----------------------------------------------------------------------------------------

//-----------DEVICE'S DAYS-COUNTER AFTER ACTIVATION-----------------------------------------------
dayCounter=EEPROM.read(911);//count number of days since the system was first put into operation

if(dayCounter==0)//if it is the first time use of the device..not even a day of operation
{

  //immediately create a newFile.csv=list.csv + addonHeader
  add_TimeIn_TimeOut_Comment_To_The_ListFile(dayCounter);
  EEPROM.write(911,dayCounter);//update the value of the dayCounter in its memory
}
//-------------------------------------------------------------------------------------------

option_To_UserRegistration_Attendance_Formating_Downloading();

}

void loop()
{


  display_Clock_On_The_HomeScreen();

  //userRegistering OR ATTEND
   key = keypad.getKey();
   
    switch(key)
  {
    case '1'://for registration

          registration_Process();//userRegisteringING OPTIONS
          
         
            break;
      case '2'://for attendance
            lcd.clear();
            lcd.setCursor(2,0);
            attendance_Process();
            break;
      case 'A'://format device
      delay(1);
          format_EEPROM_data();
          
          break;
      case 'D'://Download file
      writing_The_AttendanceFile_Created_To_Excel();
      break;
    default: 
          break;  
  } 
}

void option_To_UserRegistration_Attendance_Formating_Downloading()
    {
       lcd.clear();
       lcd.setCursor(0,0);  
       lcd.print(F("1.REGISTER"));
       lcd.setCursor(0,1);
       lcd.print(F("2.ATTEND"));

       //PRINTING THE IMMEDIATE TIME ON THE HOMESCREEN
  DateTime now = rtc1.now();
  
  

  
  lcd.setCursor(15,0);
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.setCursor(18,1);
  lcd.print(now.day(), DEC);
  lcd.setCursor(18,2);
  lcd.print(now.month(), DEC);
  lcd.setCursor(15,3);
  lcd.print(now.year(), DEC);

  
    }

   void registration_Process()
  {
    lcd.clear();
    lcd.setCursor(0,0);
    
    lcd.print(F("CODE:"));
    lcd.blink();
    user.userCode[userCounter]=enter_The_UserCode_In_This_Row(1);//input codes not more than 10 digits on the second row
    

    //-----------------READ the corresponding name for the entered REG.No-----------
     int returnedVal=get_Name_Of_The_UserCode(user.userCode[userCounter]);//call the function to pull the name 
     if(returnedVal)//if the returned value is 1 means the input code is wrong else continue the process
     goto END;
     
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("CODE:  ");
    lcd.print(user.userCode[userCounter]);
    lcd.setCursor(0,1);//second line
    lcd.print("NAME:");
    lcd.print(fName);//print out the first name corresponding to the entered code

    
    
    //----------------------------------------------------------------------------

    //-------------- MAKE A USER CONFIRM THE userRegisteringED INFO TO CONTINUE--------------
    lcd.setCursor(1,2);
    lcd.print("1.CONFIRM");
    lcd.setCursor(1,3);
    lcd.print("2.CANCEL");
    proceed_Or_Cancel_The_Registration();
    END:
      return 0;
    //----------------------------------------------------------------------------------
    
   }



long enter_The_UserCode_In_This_Row(int row)//will return the double codeInput
{
  
  int i;//for  for loop iteration
  char key;//get input character from the keypad
  long temp=0;
  char lastKey;

  i=0;

  //enable registration numbers less than 10 digits
   while(i<10)//for(i=0;i<10;i++)
  {
    
    lcd.setCursor(i,row);//i for the cols and j for the rows
BACK:
    char   key ='x';//trick the keypad... to let key be x
          while(key=='x')
            
            key=keypad.getKey();//let it be null
            
            while(key==NULL)//while key will be still NULL
            {
              key=keypad.getKey();//try to find a typed non NULL value
              
            }
       
           //if the input character is an alphabet do nothing and then return
           if((key=='A')||(key=='B')||(key=='C')||(key=='D'))
           goto BACK;
                         
            if(key=='*')//if ENTER IS PRESSED
            {
              
            break;
            }
            //delete key
            if(key=='#')//if DELETE key IS PRESSED
            {
              
                lcd.clear();
              lcd.setCursor(0,0);
               
               lcd.print(F("CODE:"));;
              lcd.setCursor(0,1);
              
              
              temp=temp/10;//return  back one column
         
              if(lastKey=='0' || lastKey=='1' || lastKey=='2'|| lastKey=='3'|| lastKey=='4')
              {
                if(temp>=0)//to avoid going to negative numbers
                lcd.print((double)temp,0);
              }

              if(lastKey=='5' || lastKey=='6' || lastKey=='7'|| lastKey=='8'|| lastKey=='9')
              {
                temp=temp-1;//125=12 not 13; 456=45 not 46 AFTER DELETE
                lcd.print((double)temp,0);
              }
              i--;
             continue;
            }
            lastKey=key;//to prevent 1225=123,126=13 while deleting
           lcd.print(key);
            temp=temp*10 +(key-'0');//to prevent the last null value on the strings
            
          i++;
            
         
       
  } 
    lcd.noBlink();//remove cursor blinking
           return temp;
}


//---------------------------FINGER CHECK -------------------------------------------------------
int attendance_Process()
{
   //------------------------------------SCANNING THE FINGER----------------------------
 int temp;
 int p=1;
  lcd.clear();
  //block for the time display during attendance
  DateTime now = rtc1.now();//fetch the data read from the rtc

    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    lcd.setCursor(10,0);
    lcd.print(now.year(), DEC);
    lcd.print('/');
    lcd.print(now.month(), DEC);
    lcd.print('/');
    lcd.print(now.day(), DEC);
    
    
  lcd.setCursor(0,1);
  lcd.print(F("PRESS FINGER:"));
  lcd.setCursor(0,2);
 
  while(p!=FINGERPRINT_OK)
  {
    p = finger.getImage();
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
  return -1;
  //-------------------------------SEARCHING THE CORRESPONDING FINGER TEMPLATE-----------

  p = finger.fingerFastSearch();
  
  if (p != FINGERPRINT_OK)// if finger not found
  {
    delay(10);
   lcd.clear();
   lcd.setCursor(2,0);
   lcd.print(F("NOT REGISTERED!"));
   lcd.setCursor(3,1);
   lcd.print(F("TRY AGAIN"));
   delay(2000);
   option_To_UserRegistration_Attendance_Formating_Downloading();
  return -1;
  }
 //else if the finger found in the fingerprint database
 //find its corresponding index in user.prints[] to find its all related details
 
 
 for(int i=0;i<=userCounter;i++)//userCounter starts from 0
 {
  if(user.prints[i]==finger.fingerID)
  {
    
    
    //-------------------------GET THE PERSON'S NAME FROM THE FILE-------------------
     get_Name_Of_The_UserCode(user.userCode[i]);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("NAME:");
    lcd.print(fName);
    lcd.setCursor(0,1);
    
    //recall the pressed times by the user from the EEPROM
    user.pressedTimes[i]=EEPROM.read(800+i);//800 is the 1st EEPROM address for pressedTimes
    
    //record the number of pressed made by this i per day
    user.pressedTimes[i]=user.pressedTimes[i]+1;//increment the nber of times of presses
  
    //if it is recorded once...it is a TIME IN
    if(user.pressedTimes[i]==1)
    {
      lcd.print("TIME IN: ");
     
    }
   
    //if it is recorded twice in a day.....it is TIME OUT
     if(user.pressedTimes[i]==2)
    {
      lcd.print("TIME OUT: ");
      user.pressedTimes[i]=0;
    }
     //record the pressedTimes in the EEPROM to avoid data loss
   EEPROM.write(800+i,user.pressedTimes[i]);
  //PRINT THE TIME THE ATTENDANCE IS DUE ON THE SCREEN
  print_On_Screen_Time_When_The_Attendance_Is_Due();
  record_The_UserCode_To_The_Todays_File(user.userCode[i]);
  break;
  }
  
 }
 

    delay(3000);
    option_To_UserRegistration_Attendance_Formating_Downloading();
 
}





//---------------------------FINGER ENROL-------------------------------------------------------
int fingerPrint_Enroll_To_The_Database()
{

  
  int address;//to return the address of the id(either having the lecturerer's address or student address)
  lcd.clear();
  lcd.setCursor(4,0);
  //------------------ANALYSE THE FINGERPRINT FIRST-----------------------------------
  int p = -1;
  lcd.print(F("PLACE FINGER:"));
  delay(1000);

  

  //the loop will stop iteration if FINGERPRINT_OK
  while (p != FINGERPRINT_OK)
  {
    //SCANNING THE FINGER
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      lcd.setCursor(0,1);
      lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Image taken"));
      break;
    case FINGERPRINT_NOFINGER:
       lcd.setCursor(0,1);
       lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("No Finger"));
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
       lcd.setCursor(0,1);
       lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Comm Error"));
      break;
    case FINGERPRINT_IMAGEFAIL:
       lcd.setCursor(0,1);
       lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Imaging Error"));
      break;
    default:
       lcd.setCursor(0,1);
       lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Unknown Error"));
    
    }
  }
  //IF FINGER SCANNING IS SUCCESSFULLY DONE THEN
  
  //CONVERTING THE FINGERPRINT IMAGE
  
  p = finger.image2Tz(1);//put 1st template in slot location 1 for verification
  switch (p) {
    case FINGERPRINT_OK:
      
       lcd.setCursor(0,1);
       lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Image converted"));
      break;
    case FINGERPRINT_IMAGEMESS:
        
      return p;//p is -1
    case FINGERPRINT_PACKETRECIEVEERR:
             lcd.setCursor(0,1);
             lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Comm Error"));
      return p;
    case FINGERPRINT_FEATUREFAIL:
             lcd.setCursor(0,1);
      lcd.print(F("Feature Not Found"));
      return p;
    case FINGERPRINT_INVALIDIMAGE:
                  lcd.setCursor(0,1);
                  lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Feature Not Found"));
      return p;
    default:
                  lcd.setCursor(0,1);
                  lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Unknown Error"));
      return p;
  }
  //DO A double CHECK
  
   lcd.setCursor(0,1);
   lcd.print("               ");
      lcd.setCursor(0,1);
  lcd.print(F("Remove Finger"));
  delay(500);
  p = 0;
  
  //if no finger detected then it will wait till the finger is pressed again
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  p = -1;
  //---------------------PLACE THE SAME FINGER--------------------------------------
    lcd.setCursor(0,1);
    lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Place again"));

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      
      break;
    case FINGERPRINT_NOFINGER:
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      
    
      break;
    case FINGERPRINT_IMAGEFAIL:
   
      break;
    default:
      
   
      return;
    }
  }
  // OK success!
  //convert the scanned finger-> (2)
  p = finger.image2Tz(2);//place template in the location 2 for verification
  switch (p) {
    case FINGERPRINT_OK:
     
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    option_To_UserRegistration_Attendance_Formating_Downloading();
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    option_To_UserRegistration_Attendance_Formating_Downloading();
      return p;
    case FINGERPRINT_FEATUREFAIL:
      lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    option_To_UserRegistration_Attendance_Formating_Downloading();
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    option_To_UserRegistration_Attendance_Formating_Downloading();
      return p;
    default: 
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    option_To_UserRegistration_Attendance_Formating_Downloading();
      return p;
  }
  
  p = finger.createModel();//model created
  if (p == FINGERPRINT_OK) { 
    
    //Serial.print(F("matched"));
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    option_To_UserRegistration_Attendance_Formating_Downloading();
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    ////Serial.println("mismatch");
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    option_To_UserRegistration_Attendance_Formating_Downloading();
    return p;
  } else {
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    option_To_UserRegistration_Attendance_Formating_Downloading();
    return p;
  }

  //store the model of the id
  p = finger.storeModel(id);//store calculated model for later matching
  if (p == FINGERPRINT_OK) {
    //-------------------------FINGER IS STORED-----------------------------------------
   //printId(userCount);
    lcd.setCursor(2,4);
    lcd.print(F("REGISTERED!"));
    

    
    
    
  } 
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION) {
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    
    return p;
  } 
  else if (p == FINGERPRINT_FLASHERR) {
    
    return p;
  }
  else {
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    
    return p;
  }
 
//---------------------------ENROLLING A STUDENT---------------------------------------------------
 
    user.userID[userCounter]=userCounter;
    user.prints[userCounter]=id;//save the finger//printId of a student

//---------------------STORING THE FINGERPRINT ID IN THE EEPROM------------------------
   
        EEPROM.write(userLoc2,user.prints[userCounter]);
        userLoc2++;
        EEPROM.write(226,userLoc2);//store the incremented value in EEPROM
        
//--------------------------------------------------------------------------------------



  
     userCounter++;

    //UPDATE userCounter VARIABLE IN THE EEPROM ADDRESS
    EEPROM.update(251,userCounter);
    userCount=userCounter;
    
    id++;// increase the number of overall registered users(lecturer,student,retaker)
    EEPROM.update(233,id);//save to EEPROM

  lcd.clear();
  lcd.setCursor(0,0);
lcd.print("REGISTERED");

  delay(2000);
    option_To_UserRegistration_Attendance_Formating_Downloading();
    
}

//---------------------------------------------------------------------------------------


void format_EEPROM_data()
{
    finger.emptyDatabase();  
   
   
   lcd.clear();
          lcd.setCursor(5,0);
          lcd.print("FORMATING");
   for(int z=0;z<1024;z++)//EEPROM IS OF 1K MEMORY
   {
    EEPROM.write(z,0);//clear all the EEPROM locations by writing zero to all positions
   }
   
          for(int lcdrows=1;lcdrows<4;lcdrows++)//rows
          {
            
            for(int lcdcols=0;lcdcols<20;lcdcols++)//columns
            {
            
             
              lcd.setCursor(lcdcols,lcdrows);
              lcd.print(".");
              delay(100);
             
            }
            
          }
  
 lcd.clear();
 lcd.setCursor(0,1);
 lcd.print("*******RESET*******");

}


void display_Clock_On_The_HomeScreen()
{
  //printing the time on the first line
DateTime now = rtc1.now();
//-------------PRINTING HOUR-------------------------------------------------
   if(now.hour()<10)
  {
     lcd.setCursor(0,3);
     lcd.print("0"); 
    
    lcd.setCursor(1,3);
  lcd.print(now.hour(), DEC);
    
  }
 //...
 else if(now.hour()>=10)
  {
    lcd.setCursor(0,3);
    lcd.print(now.hour(), DEC);
  }
  lcd.setCursor(2,3);
     lcd.print(":"); 
  //delay(70);
//------------------PRINTING MINUTES---------------------
if(now.minute()<10)
  {
     lcd.setCursor(3,3);
     lcd.print("0"); 
    
    lcd.setCursor(4,3);
  lcd.print(now.minute(), DEC);
    
  }
else if(now.minute()>=10)
  {
    lcd.setCursor(3,3);
    lcd.print(now.minute(), DEC);
  }
//-------------PRINTING SECONDS----------------------------

  lcd.setCursor(5,3);
  lcd.print(":");
  lcd.setCursor(6,3);
  lcd.print(now.second(), DEC);
  
 //if seconds equals 0 then empty the (7,3) location
  if(now.second()==0)
  {
    lcd.setCursor(7,3);
    lcd.print(" ");
  }
  //if minutes equals 0 then empty the(4,3)
  if(now.minute()==0)
  {
    lcd.setCursor(4,3);
    lcd.print(" ");
  }
  //if hours equals 0 then empty the(1,3)
  if(now.hour()==0)
  {
    lcd.setCursor(1,3);
    lcd.print(" ");
  }

//if at midnight then create a newFileX.csv where x is the dayCounter
  if((now.hour()==0)&&(now.minute()==0)&&(now.second()==0))
  {
    dayCounter=EEPROM.read(911);
    dayCounter++;
    EEPROM.write(911,dayCounter);
    add_TimeIn_TimeOut_Comment_To_The_ListFile(dayCounter);//add an addon on the list.csv
    
  }

}


//get the name from the files
int get_Name_Of_The_UserCode(long userCode)//return the value that tells whether the code given is wrong or good to proceed the process
{
  int wrong=0;//the variable to keep the value indicating whether the code given is wrong or good to proceed the process
   int first2Vals=0;//read the 2 values of the userCode (CODE) entered (name the file)
   int lastVals=0;//read the remained digits of the entered CODE(name the number of line in a file)
  
  
  //--------------------------get two first values of the userCode to know the filename---
  //know the registration number of the user
  long number=userCode;
  int j=3;//index counter
  long arr[4]={};//array to keep the userCode in array form..to easily extract the 2 first values....max.number of the userCode is 4
   int  count=0;
 String myString;//store the characters being read
   int presLines=1,prevLines=0;//to keep track of the number of lines while reading the file (present line number and the previous line number)
  

 //divide the number into digits and keep it into an array
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
  //get the two values.....call it first2Vals
  //-----------------------------------------read the first two values in first2Vals---------
  first2Vals=arr[0]*10+arr[1];
  //---------------------------------------------------------------------------------
  //--------------------------------------read the last values in lastVals----------------
  if(arr[3]==0)//if have only 3 values as a CODE
    lastVals=arr[2];
  else if(arr[3]!=0)
    lastVals=arr[2]*10+arr[3];
  //--------------------------------------------------------------------------------
  
  //-----check if  the two values(got from the userCode) matches one of the 20 file names
  int z;
  for(z=0;z<20;z++)
  {
    //if the two values matches one of the fileName
    if(first2Vals==fileNames[z])//if the first 2 numbers of the userCode matches the fileName
    {
      //open the matched file name
      char file[9];//to hold the name of the file
      sprintf(file,"%d.csv",first2Vals);
      //creating a stream to open the file
      File fileStream=SD.open(file);//file string contains the file name
      //if the file is available
  if(fileStream)
  {
    while(fileStream.available())
    {
      char x=fileStream.read();
      myString+=x;
      
    if((x==' ') &&(presLines>prevLines))//if the space is encountered and the number of present lines greater than the number of previous lines
    {
      //record the myString value as the First name of the name entered
      prevLines=presLines;//for the fName string to include the first name
      fName="";
      fName=myString;//keep the First name of the person
    }
    //if the Fstream.read encounters the comma....reading next person's names
     if(x==',')
     {
      
      //clear the my string to read another name
      myString="";
      
      //break if the target person's name is passed
      //the last values of the CODE determine the target name (kept in lastVals)
      if(lastVals==presLines)//johnstone busingye,
      break;
      presLines++;//increment the number of lines variable 
     } 
     
    }
    fileStream.close();//close the file after reading
  }
 //--------------------------------------------------------------------------------------
else //if file not available
  Serial.println("error opening the file");
  lcd.clear();
  lcd.print("   RETRY");
  option_To_UserRegistration_Attendance_Formating_Downloading();
  break;
    }
  }
 
  //-----------------------if the code dont  match the file.....WRONG CODE------------------------------
  if(z==20)
  {
    lcd.clear();
    lcd.setCursor(4,1);
    lcd.print("WRONG CODE");
    delay(1000);
    lcd.clear();
    wrong=1;//wrong code given
    goto END;
  }
  //------------------------------------------------------------------------------------

END:
option_To_UserRegistration_Attendance_Formating_Downloading();
return wrong;
}

//read key typed from the keypad.....1.COMFIRM 2.CANCEL
void proceed_Or_Cancel_The_Registration()
{

              char   key ='x';//trick the keypad... to let key be x
   
          while(key=='x')
            
            key=keypad.getKey();//let it be null
            
            while(key==NULL)//while key will be still NULL
            {
              key=keypad.getKey();//try to find a typed non NULL value
            }
       switch(key)
       {
        case '1'://if the user confirm the information given
    lcd.clear();
    lcd.setCursor(0,0);
    //FINGERPRINT ENROLL
    fingerPrint_Enroll_To_The_Database();//(int userbit)
    
        break;
        case '2': //if the user cancel;
        lcd.clear();
        option_To_UserRegistration_Attendance_Formating_Downloading();//back to options
        break;
        default:
        break;
       } 
       
} 

//-------------------------------------PRINT THE TIME THE ATTENDANCE IS DUE---------
void print_On_Screen_Time_When_The_Attendance_Is_Due()
{
  
    //printing the time on the first line
DateTime now = rtc1.now();
//-------------PRINTING HOUR-------------------------------------------------
   if(now.hour()<10)
  {
     lcd.setCursor(9,1);
     lcd.print("0"); 
    
    lcd.setCursor(10,1);
  lcd.print(now.hour(), DEC);
    
  }
 //...
 else if(now.hour()>=10)
  {
    lcd.setCursor(9,1);
    lcd.print(now.hour(), DEC);
  }
  delay(70);
//------------------PRINTING MINUTES---------------------
if(now.minute()<10)
  {
     lcd.setCursor(12,1);
     lcd.print("0"); 
    
    lcd.setCursor(13,1);
  lcd.print(now.minute(), DEC);
    
  }
else if(now.minute()>=10)
  {
    lcd.setCursor(12,1);
    lcd.print(now.minute(), DEC);
  }
//-------------PRINTING SECONDS----------------------------

lcd.setCursor(11,1);
  lcd.print(":");
  lcd.setCursor(12,1);
  lcd.print(now.minute(), DEC);
  lcd.setCursor(14,1);
  lcd.print(":");
  lcd.setCursor(15,1);
  lcd.print(now.second(), DEC);
  
 //if seconds equals 0 then empty the (7,3) location
  if(now.second()==0)
  {
    lcd.setCursor(16,1);
    lcd.print(" ");
  }
  //if minutes equals 0 then empty the(4,3)
  if(now.minute()==0)
  {
    lcd.setCursor(13,1);
    lcd.print(" ");
  }
  //if hours equals 0 then empty the(1,3)
  if(now.hour()==0)
  {
    lcd.setCursor(10,1);
    lcd.print(" ");
  }
    //--------------------------------------------------------------------------
    
}

//creating a daily attendance file
void record_The_UserCode_To_The_Todays_File(long userCode)
{
 char getNameExt[15];
 long dailyAttendFiles[100];
 int noFileSignal=0;
int totalFiles=0;
//get today's date as the name of the file

      char getName[8];
      DateTime now=rtc1.now();
      sprintf(getName,"%d%d%d",now.day(),now.month(),now.year());
      

  //create a today's file attendance if not found

    File attendFiles=SD.open("attFiles.txt",FILE_WRITE);
    if(attendFiles)
    {
      String temp;
      while(attendFiles.available())
      {
        char x=attendFiles.read();
        temp=x;
        if(x=" ")//if it ends reading the first name....before reading the second
        {
          if(temp==getName)
          { 
            noFileSignal=1;
            goto ATTEND;
            break;
          }
          temp="";
          }
      }
    }
    attendFiles.close();
    
    if(noFileSignal==0)
    {

      sprintf(getNameExt,"%s.csv",getName); 

     
       File todayFile;
       todayFile=SD.open(getNameExt);
       todayFile.close();
      
      //save the name into the attFiles.txt
      File attendFiles=SD.open("attFiles.txt",FILE_WRITE);
      attendFiles.print(getName);
      attendFiles.print(" ");
      attendFiles.close();
      
    }

  ATTEND: 
  File todayFile=SD.open(getNameExt,FILE_WRITE);
  todayFile.print(userCode);
  todayFile.print("(");
     
   char getTimeNow[5];//23:00..must hold 5 characters
   sprintf(getTimeNow,"%d:%d",now.hour(),now.minute());
    todayFile.print(getTimeNow);
    todayFile.print("),");
    todayFile.close();// close file  


 todayFile=SD.open(getNameExt,FILE_READ);
    if(todayFile)
    {
      while(todayFile.available())
      {
        char x=todayFile.read();
      }
      todayFile.close();
    }
    
//File attendFiles=SD.open("attFiles.txt",FILE_WRITE);
 attendFiles=SD.open("attFiles.txt",FILE_READ);
    Serial.println("reading names of the filenames");
    if(attendFiles)
    {
      while(attendFiles.available())
      {
        char x=attendFiles.read();
        Serial.print(x);
      }
      attendFiles.close();
      Serial.println("");
    }
    
    Serial.println("closing the attendFiles file");

return getNameExt;
    
}

//------------------ADDING A HEADER FILE ON THE LIST.CSV (*TIME/TIME-IN,TIME-OUT,COMMENT)---
void add_TimeIn_TimeOut_Comment_To_The_ListFile(int dayCounter)
{
  int endOLine;//counts nber of comma that makes up a line
  int commaCounter=0;//count nber of commas on the line
  int line=1;//count the nber of lines in the file

  //strings to be used to create a new file
  String tempStr={};
  String listStr={};//copy line from the list.csv
  String addStr={};//string will hold the addOn
  String downString={};//hold data to be transfered to the downloadable file


        File fileEdit1;
        
    endOLine=4+(4*dayCounter);//initially the file is madeup of 4 commas per line, 4new commas will be added at each new day

    //open the list.csv in writing mode
  char prevList[15]={};
  char fileName[15]={};
  
  if(dayCounter==0)
  {
    
    goto LIST;
  }
    

    
  if(dayCounter>=1)
  {
    dayCounter=dayCounter-1;//to get the prev day's attendance file
    sprintf(prevList,"newFile%d.csv",dayCounter);
    goto NEWFILE;
  }
  
  LIST:
     fileEdit1=SD.open("list.csv");
     goto NEXT;

  NEWFILE:
   fileEdit1=SD.open(prevList);
   
 NEXT:
    if(fileEdit1)
    {
      
      while(fileEdit1.available())
      {
        char x=fileEdit1.read();
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
              addStr=",";

              DateTime now = rtc1.now();
              addStr+=String(now.day())+"/"+String(now.month())+"/"+String(now.year());
              addStr+=",,,";
              downString=listStr+addStr;
              //Serial.print("downString1:");
              //Serial.println(downString);
              
              
              sprintf(fileName,"newFile%d.csv",dayCounter);

              //remove an existing file that might have the same name as the file to be created
              SD.remove(fileName);
              
              File newFile= SD.open (fileName,FILE_WRITE);
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
              //Serial.print("downString2:");
              //Serial.println(downString);

              File newFile=SD.open (fileName,FILE_WRITE);
              if(newFile)
              {
                newFile.print(downString);
                downString="";//clear
                tempStr="";//clear the string 
                listStr="";//
                newFile.close();
              }
              
              
             }

             if(line>2)//greater than 2 lines
             {
              listStr=tempStr;//hold first line of the list.csv
              addStr="";
              downString=listStr+addStr;
              //Serial.print("downString3:");
              //Serial.println(downString);

              File newFile=SD.open (fileName,FILE_WRITE);
              if(newFile)
              {
                newFile.print(downString);
                //Serial.println(downString);
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

void create_Attendance_File()
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
int tempLine;
String regStr;


String regFile={};
char getNameExt[15];
char download[15];
String t1={},t2={};
int count=0;

  //STEP1: get str1...line by line FROM THE newList.csv
  //----------------------------------------------------------------------------------------
  char fileName[15];
  sprintf(fileName,"newFile%d.csv",dayCounter);
  
  File fileStr1=SD.open(fileName,FILE_READ);
  Serial.print("file with addon:");
  Serial.println(fileName);
  if(fileStr1)
  {
    while(fileStr1.available())
    {
      char x=fileStr1.read();
      str1+=x;
       if(x==',')
      commaCounter++;
      
      if(commaCounter==(4*dayCounter +4))//on the first two lines
      { 
        //append to a downloadable file
  //--------------------------------------------------------------------------------------
  dayCounter=EEPROM.read(911);
  
  sprintf(download,"download%d.csv",dayCounter);
  SD.remove(download);//if any similar file first delete it...to save storage..no multiplication
        File fileEdit=SD.open(download,FILE_WRITE);
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
        tempLine=line-2;
        regStr="11"+(String)tempLine;
      }
      if((line>9)&&(line<=12))
      {
        tempLine=line-9;
        regStr="21"+(String)tempLine;
      }
      if((line>12)&&(line<=16))
      {
        tempLine=line-12;
        regStr="31"+(String)tempLine;
      }
      
      if((line>16)&&(line<=20))
      {
        tempLine=line-16;
        regStr="32"+(String)tempLine;
      }
      if((line>20)&&(line<=22))
      {
        tempLine=line-20;
        regStr="41"+(String)tempLine;
      }
      if((line>22)&&(line<=29))
      {
        tempLine=line-22;
        regStr="42"+(String)tempLine;
      }
      if((line>29)&&(line<=42))
      {
        tempLine=line-29;
        regStr="43"+(String)tempLine;
      }
      if((line>42)&&(line<=48))
      {
        tempLine=line-42;
        regStr="44"+(String)tempLine;
      }
      if((line>48)&&(line<=50))
      {
        tempLine=line-48;
        regStr="51"+(String)tempLine;
      }
      if((line>50)&&(line<=52))
      {
        tempLine=line-50;
        regStr="52"+(String)tempLine;
      }
      if((line>52)&&(line<=58))
      {
        tempLine=line-52;
        regStr="53"+(String)tempLine;
      }
      if((line>58)&&(line<=61))
      {
        tempLine=line-58;
        regStr="54"+(String)tempLine;
      }
      if((line>61)&&(line<=66))
      {
        tempLine=line-61;
        regStr="55"+(String)tempLine;
      }
      if(line==67)
      {
        tempLine=line-66;
        regStr="61"+(String)tempLine;
      }
      if(line==68)
      {
        tempLine=line-67;
        regStr="62"+(String)tempLine;
      }
      if((line>68)&&(line<=71))
      {
        tempLine=line-68;
        regStr="63"+(String)tempLine;
      }
      if((line>71)&&(line<=74))
      {
        tempLine=line-71;
        regStr="64"+(String)tempLine;
      }
      if((line>74)&&(line<=81))
      {
        tempLine=line-74;
        regStr="65"+(String)tempLine;
      }
      if((line>81)&&(line<=87))
      {
        tempLine=line-81;
        regStr="71"+(String)tempLine;
      }
      if((line>87)&&(line<=93))
      {
        tempLine=line-87;
        regStr="81"+(String)tempLine;
      }
     

    //STEP2: get str2...from the today's attendance
  //----------------------------------------------------------------------------------------
  

  //open the today's attendance file
  

  DateTime now=rtc1.now();//declare the rtc variable type to fetch date,month and year
  char getName[15];
      sprintf(getName,"%d%d%d",now.day(),now.month(),now.year());
      
  sprintf(getNameExt,"%s.csv",getName); 
  File attendFiles=SD.open(getNameExt);//both writing and reading mode
  
 
  if(attendFiles)
    {
      while(attendFiles.available())
      {
        char x=attendFiles.read();
        
       if(regFile==regStr)//if the userCode in the attendance file == to the userCode in the newList.csv..chance to find out the both time in and the time out
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
      str3=str1+str2;
      
      Serial.println(str3);
      
            //append to a downloadable file
  //--------------------------------------------------------------------------------------
        File fileEdit=SD.open(download,FILE_WRITE);
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



//----------------WRITE TO EXCEL--------------------------------------
void writing_The_AttendanceFile_Created_To_Excel()
{
  
  int commaCounter=0;
  int dayCounter= EEPROM.read(911);
  String str={};
  char ch='A';//incrememtn the columns
  int line=1;//counts number of rows
  char strC[15];

  int numCommas=0;

  create_Attendance_File();
   Serial.println("CLEARSHEET");
  
  char download[15]={};//keep downloadable file name
  dayCounter=EEPROM.read(911);
  sprintf(download,"down%d.csv",dayCounter);
  
  File downloadFile=SD.open(download,FILE_READ);

     if(downloadFile)
    {
      while(downloadFile.available())
      {
        char x=downloadFile.read();
       
       if(x==',')
        {
          delay(3);
          sprintf(strC,"CELL,SET,%c%d,",ch,line);//
          Serial.print(strC);
          Serial.println(str);
          commaCounter++;
          ch++;
          
          str="";
          continue;
        }
        dayCounter=0;
        
        if(commaCounter==(4*(dayCounter+1) +4))//if all the row is done to be read,on all lines now
        {
          
          commaCounter=0;
          delay(3);
          sprintf(strC,"CELL,SET,%c%d,",ch,line);//
          Serial.print(strC);
          Serial.println(str);
          line++;
          ch='A';
          str="";
          if(line==120)
          Serial.println("PAUSELOGGING");//after writing to excel
        }

        if(line>=3)
        {
           if(commaCounter==(4*(dayCounter) +4))//if all the row is done to be read,on all lines now
           {
            commaCounter=0;
          delay(3);
          sprintf(strC,"CELL,SET,%c%d,",ch,line);//
          Serial.print(strC);
          Serial.println(str);
          line++;
          ch='A';
          str="";
           }
       
        }
         str+=x;
         
      }
      Serial.println("PAUSELOGGING");//after writing to excel
      downloadFile.close();
    }
}



