#include <EEPROM.h>
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(29,31,33,35,37,39,41,43,45,47);//lcd in 8 bit mode
//rs,e,d0 to d7

#include <Wire.h>

#include <avr/pgmspace.h>

#include <SD.h>


#include <Keypad.h>

#include <Adafruit_Fingerprint.h>
//#include <LiquidCrystal_PCF8574.h>

#define chipSelect 53
//LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display

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





//fingerprint to serial communication
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10,11);//Tx,Rx pins of mega, GREEN,YELLOW

//defined functions
void homeScreen();
void studentReg();// for the worker's registration
void enterToHomeScreen();
long codeInput(int row,int who );//enables to type the registration code
int attendance();//while attendance
int getFingerprintEnroll();
void printId(int id);//output the user's number
void deleteAllPrints();
void toHome();
void EEPROMWritelong(int address, long value);
long EEPROMReadlong(int address);

void EEPROMWrite16bits(int address, int value);
uint16_t EEPROMRead16bits(int address);
void printToXl();
void attendanceToXl(long valToXl);

void getTime();
void initPressedTimes();
int getName(long regNo);
void switchKey();
void timeNow();
void dailyAttendanceFile(long regNo);
void fileDownload();
void fileAddon();
void downloadableFile(int line,int pressedTimes,String Time);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int getFingerprintEnroll();
int attendance();
//void writingToExcel();
void deleteAllPrints();
long EEPROMReadlong(int address);
void EEPROMWritelong(int address, long value);//int stores 16 bits
void regOptions();
//void lecturerReg();
void studentReg();
void homeScreen();
void printToXl();
void attendanceToXl(long);
void EEPROMWrite16bits(int address, int value);
uint16_t EEPROMRead16bits(int address);

//declaring excel variables
static int inc=3;//increments as number of students on excelsheet is increasing
static char incChar2='C';//keep track of the column of the sessions
 static int session=1;//count number of sessions
 


static long address;


int arrCode[10];//array to keep every input data in its array
char key;

//for fingerprint to eeprom

  int i;//for iteration

 //for the rtc to print the exact day in words
 char daysOfTheWeek[7][12] = {"Satu","Sund", "Mond", "Tues", "Wedn", "Thur", "Frid"};

    
//creating  file pointer
File myfiles1;
File myfiles2;

//for fingerprint to eeprom
static int id=0;//takes all the users'fingerprint


//FOR EEPROM
static int studCounter=0;
int regNo_starting_loc = 3;
int fprint_starting_loc = 404;
int pressed_time_startIng_loc = 505;
int id_loc = 605;
int RTC_setting_loc=606;

int fileCode[10]={};//fileCode will hold the entered MINIJUST CODE
int fileNames[20]={11,21,31,32,41,42,43,44,51,52,53,54,55,61,62,63,64,65,71,81};//20 files 
String fName;

int dayCounter;
char getNameExt[15];

//RTC variables
char Time[]     = "  :  :  ",
     calendar[] = "      /  /20  ";

  byte  second, minute, hour, day, date, month, year;


//student's records


struct students{
        
     
        PROGMEM int studID[100];//take the number of students registered consecutively
        PROGMEM long regNo[100];
        PROGMEM int prints[100];//will take the value of the fingerprint scanned
        PROGMEM int pressedTimes[100]={};//record how many times does a user attempted to attend(either 
        }stud;


void downloadFile();
void toXl();
void setup() {

  lcd.begin(20,4);
  //lcd.clear();
  Serial.begin(9600);
  delay(10);

 //clears all data on activeSheet of excel if available
  //Serial.println("CLEARSHEET");

  
    
  //SD card settings
  pinMode(chipSelect,OUTPUT); //declare the CS as an OUTPUT
  //SD.begin(chipSelect);
  
  if(!SD.begin(chipSelect))// 53 is chipselect on arduino mega
  {
    //Serial.println("SD card not working");
  }
  else
  //Serial.println("SD card  working");
  
//-----------------------RTC CLOCK SETTINGS-------------------------------------------

Wire.begin();
DS3231_read();
DS3231_display();


//----------------------------------------------------------------
  
  // set the data rate for the sensor serial port
  finger.begin(57600);
  //----------------------------------------------FINGERPRINT SCANNER PRESENCE TEST----------------------------------------------------
  if (finger.verifyPassword()) //
  {
    ////Serial.println("Found fingerprint sensor!");
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print(F("  THE GIANTS"));
    lcd.setCursor(4,2);
    lcd.print(F(" ELECTRONICS"));
    //Serial.println(" fingerprint working");

    //if working then empty the fingerprints to restart over
   //finger.emptyDatabase();
    delay(3000);
  } else //if finger.verifyPassword() returns zero
  {
    ////Serial.println("Did not find fingerprint sensor :(");
    lcd.print(F("FINGERPRINT"));
    lcd.print(F(" NOT FOUND!"));
    delay(2000);
    while (1) { delay(1); }
  }

 //------------------RESTORE id value to know how many are in fprint memory-------------------------------------
      reading_fingerprint_id_locations_in_EEPROM();
 //------------------------------RESTORE studCounter VALUE------------------------------------
   reading_studCounter_value_in_EEPROM();

 //----------------------RESTORE STUDENTS REG.No FROM EEPROM-------------------------------
  reading_student_regNo_saved_in_EEPROM();

//----------------------RESTORE STUDENTS PRINTS FROM EEPROM-------------------------------
  reading_student_fingerprints_saved_in_EEPROM();
//----------------------------------------------------------------------------------------

//--------------------------RESTORE STUDENTS PRESSED TIMES------------------
  reading_pressed_times_saved_in_EEPROM();
//-------------------------------------------------------------------------------------------
//delete_File();
homeScreen();
downloadFile();
}

void loop()
{

  DS3231_read();                                // Read time and calendar parameters from DS3231 RTC
  DS3231_display(); // Display time & calendar
  delay(50);
  getTime();
 //....
  //REGISTER OR ATTEND
   key = keypad.getKey();
   
    switch(key)
  {
    case '1'://for registration

          studentReg();//REGISTERING OPTIONS
          
         
            break;
      case '2'://for attendance
            lcd.clear();
            lcd.setCursor(2,0);
            //lcd.print(F("no attend"));
            attendance();
            break;
      case 'A'://format device
      delay(1);
          deleteAllPrints();
          
          break;
      case 'D'://Download file
      daily_Attendance_To_PySerial();
      break;
    default: 
          break;  
  } 
}

void homeScreen()
    {
       lcd.clear();
       lcd.setCursor(0,0);  
       lcd.print(F("1.REGISTER"));
       lcd.setCursor(0,1);
       lcd.print(F("2.ATTEND"));

       //PRINTING THE IMMEDIATE TIME ON THE HOMESCREEN
  //DateTime now = rtc1.now();
  
  

  
  lcd.setCursor(15,0);
  lcd.print(daysOfTheWeek[day]);
  lcd.setCursor(18,1);
  lcd.print(date, DEC);
  lcd.setCursor(18,2);
  lcd.print(month, DEC);
  lcd.setCursor(15,3);
  lcd.print("20");
  lcd.setCursor(17,3);
  lcd.print(year, DEC);

  
    }

   void studentReg()
  {
    lcd.clear();
    lcd.setCursor(0,0);
    
    lcd.print(F("CODE:"));
    lcd.blink();

    studCounter = EEPROM.read(0);
    stud.regNo[studCounter]=codeInput(1,3);//input codes not more than 10 digits on the second row
    Serial.print("stud.regNo[studCounter] input:");
    Serial.print(stud.regNo[studCounter]);
    //save the stud.regNo to EEPROM
    EEPROMWrite16bits((regNo_starting_loc+(studCounter*2)),stud.regNo[studCounter]);
    //-----------------READ the corresponding name for the entered REG.No-----------
     int returnedVal;
     returnedVal= getName(stud.regNo[studCounter]);//call the function to pull the name 
     if(returnedVal)//if the returned value is 1 means the input code is wrong else continue the process
     goto END;
     
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("CODE:  ");
    lcd.print(stud.regNo[studCounter]);
    lcd.setCursor(0,1);//second line
    lcd.print("NAME:");
    lcd.print(fName);//print out the first name corresponding to the entered code
//    Serial.print("FFFFFFFFFFFFFFFFFFFFNAMEMMMMMMMMMMMMMMMMMM");
//    Serial.println(fName);
    
    
    //----------------------------------------------------------------------------

    //-------------- MAKE A USER CONFIRM THE REGISTERED INFO TO CONTINUE--------------
    lcd.setCursor(1,2);//move on the third line
    lcd.print("1.CONFIRM");//TO proceed
    lcd.setCursor(1,3);
    lcd.print("2.CANCEL");//back to homeScreen()
    //switch the input cases
    switchKey();
    END:
      return 0;
    //----------------------------------------------------------------------------------
    
    

    
    
   
  }

  void enterToHomeScreen()//check if ENTER and output the input characters 
{

              char   key ='x';//trick the keypad... to let key be x
   
          while(key=='x')
            
            key=keypad.getKey();//let it be null
            
            while(key==NULL)//while key will be still NULL
            {
              key=keypad.getKey();//try to find a typed non NULL value
            }
            if(key=='*')//if ENTER pressed
            {
              homeScreen();
            } 
}



long codeInput(int row,int who )//will return the double codeInput
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
       
           //if the input character is an alphabet then return
           if((key=='A')||(key=='B')||(key=='C')||(key=='D'))
           goto BACK;
                         
            if(key=='*')//if ENTER IS PRESSED
            {
              
            break;
            }
            //delete key
            if(key=='#')//if DELETE key IS PRESSED
            {
              
              //if @ student Reg.No:....3
              if(who==3)
              {
                lcd.clear();
              lcd.setCursor(0,0);
               
               lcd.print(F("CODE:"));;
              lcd.setCursor(0,1);
              }
              
              temp=temp/10;//return  back one column
              ////Serial.println("new temp value:");
              ////Serial.println(temp);
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
int attendance()
{
   //------------------------------------SCANNING THE FINGER----------------------------
 int temp;
 int p=1;
  lcd.clear();
  //block for the time display during attendance
  //DateTime now = rtc1.now();//fetch the data read from the rtc

    lcd.print(daysOfTheWeek[day]);
    lcd.setCursor(10,0);
    lcd.print("20");
    lcd.print(year, DEC);
    lcd.print('/');
    lcd.print(month, DEC);
    lcd.print('/');
    lcd.print(date, DEC);
    
    
  lcd.setCursor(0,1);
  lcd.print(F("PRESS FINGER:"));
  lcd.setCursor(0,2);
 
  while(p!=FINGERPRINT_OK)
  {
    Serial.println("fingerPrint OK");
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
   homeScreen();
  return -1;
  }
 //else if the finger found in the fingerprint database
 //find its corresponding index in stud.prints[] to find its all related details
 
 
 for(int i=0;i<=studCounter;i++)//studCounter starts from 0
 {
  if(stud.prints[i]==finger.fingerID)
  {
    
    //-------------------------GET THE PERSON'S NAME FROM THE FILE-------------------
    //retrieve his/her registration number that we may read the name from his/her file department
    getName(stud.regNo[i]);
    Serial.print("stud.regNo[i]:");
    Serial.println(stud.regNo[i]);
    Serial.print("i:");
    Serial.println(i);
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("NAME:");
    lcd.print(fName);
    lcd.setCursor(0,1);
    
    //recall the pressed times by the user from the EEPROM
    stud.pressedTimes[i]=EEPROM.read(pressed_time_startIng_loc+i);
    
    //record the number of pressed made by this i per day
    stud.pressedTimes[i]=stud.pressedTimes[i]+1;//increment the nber of times of presses
  
    //if it is recorded once...it is a TIME IN
    if(stud.pressedTimes[i]==1)
    {
      lcd.print("TIME IN: ");
     
    }
   
    //if it is recorded twice in a day.....it is TIME OUT
     if(stud.pressedTimes[i]==2)
    {
      lcd.print("TIME OUT: ");
      stud.pressedTimes[i]=0;//number of times per day is finished
    }
     //record the pressedTimes in the EEPROM to avoid data loss
   EEPROM.write(pressed_time_startIng_loc+i,stud.pressedTimes[i]);//pressedTimes EEPROM memory starts from(800-910)
    
  //PRINT THE TIME THE ATTENDANCE IS DUE ON THE SCREEN
  timeNow();
  dailyAttendanceFile(stud.regNo[i]);//keep the attendance record in the daily attendance file
   break;//if the target PERSON's data processed...... then break 
  }
  
 }
 

    
    
    //know how many times has a one fingerprint been recorded
    //who is the student?
    //his location
    //save it to EEPROM to prevent data loss
    
   
    Serial.println("the finger id detected:");
    Serial.println(finger.fingerID);
    
    END:
    delay(3000);
    homeScreen();
 
}





//---------------------------FINGER ENROL-------------------------------------------------------
int getFingerprintEnroll()
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
    //delay(5000);
    switch (p)
    {
    case FINGERPRINT_OK:
      //Serial.println("Image taken");
      //lcd.clear();
      ////printId(userCount);
      lcd.setCursor(0,1);
      lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Image taken"));
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println("No Finger");
      //lcd.clear();
     // //printId(userCount);
       lcd.setCursor(0,1);
       lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("No Finger"));
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      //lcd.clear();
     // //printId(userCount);
       lcd.setCursor(0,1);
       lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Comm Error"));
      break;
    case FINGERPRINT_IMAGEFAIL:
      //Serial.println("Imaging error");
      //lcd.clear();
     // //printId(userCount);
       lcd.setCursor(0,1);
       lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Imaging Error"));
      break;
    default:
      //Serial.println("Unknown error");
      // lcd.clear();
      ////printId(userCount);
       lcd.setCursor(0,1);
       lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Unknown Error"));
      //deleted the unprocessed RegNo
      EEPROMWrite16bits((regNo_starting_loc+(i*2)),0);
      //break;
    }
  }
  //IF FINGER SCANNING IS SUCCESSFULLY DONE THEN
  
  //CONVERTING THE FINGERPRINT IMAGE
  
  p = finger.image2Tz(1);//put 1st template in slot location 1 for verification
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      //lcd.clear();
      //printId(userCount);
      
       lcd.setCursor(0,1);
       lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Image converted"));
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
       //lcd.clear();
       //printId(userCount);
        
      return p;//p is -1
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
            //lcd.clear();
            //printId(userCount);
             lcd.setCursor(0,1);
             lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Comm Error"));
      return p;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
           // lcd.clear();
           //printId(userCount);
             lcd.setCursor(0,1);
      lcd.print(F("Feature Not Found"));
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
                 // lcd.clear();
                 //printId(userCount);
                  lcd.setCursor(0,1);
                  lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Feature Not Found"));
      return p;
    default:
      //Serial.println("Unknown error");
                  //lcd.clear();
                  //printId(userCount);
                  lcd.setCursor(0,1);
                  lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Unknown Error"));
      return p;
  }
  //DO A double CHECK
  
  //Serial.println("Remove finger");
  //lcd.clear();
  //printId(userCount);
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
//Serial.println("ID "); 
  //////Serial.println(id);
  p = -1;
  //---------------------PLACE THE SAME FINGER--------------------------------------
  //Serial.println("Place same finger again");
   //lcd.clear();
   //printId(userCount);
    lcd.setCursor(0,1);
    lcd.print("               ");
      lcd.setCursor(0,1);
      lcd.print(F("Place again"));
      //lcd.setCursor(0,1);
      //lcd.print(F("   Again"));
   
   //wait for getting finger to scan
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      ////Serial.println("Communication error");
      
    
      break;
    case FINGERPRINT_IMAGEFAIL:
      ////Serial.println("Imaging error");
      
   
      break;
    default:
      ////Serial.println("Unknown error");
      
   
      return;
    }
  }
  // OK success!
  //convert the scanned finger-> (2)
  p = finger.image2Tz(2);//place template in the location 2 for verification
  switch (p) {
    case FINGERPRINT_OK:
      ////Serial.println("Image converted");
     
      break;
    case FINGERPRINT_IMAGEMESS:
      ////Serial.println("Image too messy");
      lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    homeScreen();
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      ////Serial.println("Communication error");
      lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    homeScreen();
      return p;
    case FINGERPRINT_FEATUREFAIL:
      ////Serial.println("Could not find fingerprint features");
      lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    enterToHomeScreen();
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      ////Serial.println("Could not find fingerprint features");
      lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    homeScreen();
      return p;
    default: 
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    homeScreen();
      ////Serial.println("Unknown error");
      return p;
  }
  // OK converted!
  
  //take a two print feature template and create a model
  ////Serial.println("Creating model for #");  
  ////Serial.println(id);
  
  p = finger.createModel();//model created
  if (p == FINGERPRINT_OK) { 
    //////Serial.println("Prints matched!");
    lcd.print(F("matched"));
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    ////Serial.println("Communication error");
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    homeScreen();
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    ////Serial.println("mismatch");
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    homeScreen();
    return p;
  } else {
    ////Serial.println("Unknown error");
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    delay(500);
    homeScreen();
    return p;
  }
  ////Serial.println("ID "); 
  ////Serial.println(id);


  
  //store the model inside the id
  //read the id value from the EEPROM
  id= EEPROM.read(id_loc);
  p = finger.storeModel(id);//store calculated model for later matching
  if (p == FINGERPRINT_OK) {
    
//    lcd.setCursor(2,4);
//    lcd.print(F("REGISTERED!"));
//    

    
    
    
  } 
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    ////Serial.println("Communication error");
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION) {
    ////Serial.println("Could not store in that location");
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    
    return p;
  } 
  else if (p == FINGERPRINT_FLASHERR) {
    ////Serial.println("Error writing to flash");
    
    return p;
  }
  else {
    ////Serial.println("Unknown error");
    lcd.print(F("               "));
    lcd.print(F("TRY AGAIN"));
    
    return p;
  }
 
////------------------------SAVING THE FINGERPRINT ID TO EEPROM------------------------------------
     studCounter=EEPROM.read(0);
    Serial.print("studCounter read from EEPROM:");
    Serial.println(studCounter);
    
     stud.prints[studCounter]=id;
     EEPROM.write ((fprint_starting_loc+studCounter),stud.prints[studCounter]);
     Serial.print("stud.prints[studCounter]");
    Serial.println(stud.prints[studCounter]);

//-----------------------writing entered registration number to EEPROM----------------
    
    
    EEPROMWrite16bits((regNo_starting_loc+(studCounter*2)),stud.regNo[studCounter]);
    Serial.print("Reg.No written to EEPROM:");
    Serial.println(EEPROMRead16bits(stud.regNo[studCounter]));
    Serial.print("studCounter:");
    Serial.print(studCounter);

       
//--------------------------------------------------------------------------------------



    //UPDATE studCounter VARIABLE IN THE EEPROM ADDRESS
    studCounter++;
    EEPROM.update(0,studCounter);
  
  id++;// increase the number of overall registered users(lecturer,student,retaker)
    EEPROM.update(id_loc,id);//save to EEPROM
  
  lcd.clear();
  lcd.setCursor(0,0);
lcd.print("REGISTERED");

  delay(2000);
    homeScreen();
  
}

//---------------------------------------------------------------------------------------

//---------------------NUMBER OF ALL USERS---UNUSED-----------------------------------
void printId(int id)//output the user's number
{
        lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("No:"));
  //writing the value kept in the eeprom
  lcd.print(id);
}

void deleteAllPrints()
{
   //delete all fingerprint templates registered in the sensor
   finger.emptyDatabase();  
   
   
   //delete the EEPROM data
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

//in between interrupt to homeScreen
void toHome()
{

char   key ='x';//trick the keypad... to let key be x
          while(key=='x')
            
            key=keypad.getKey();//let it be null
            
            while(key==NULL)//while key will be still NULL
            {
              key=keypad.getKey();//try to find a typed non NULL value
            }

            if(key=='A')//if return to home btn is pressed 
            {
              
            return -1;
            }
}

//---------------------------EEPROM Read Write Long functions------------------------------
//This function will write a 4 byte (32bit) long to the eeprom at
//the specified address to address + 3.
void EEPROMWritelong(int address, long value)
      {
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
      }
 //This function will return a 4 byte (32bit) long from the eeprom
//at the specified address to address + 3.
long EEPROMReadlong(int address)
      {
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
      }

void EEPROMWrite16bits(int address, int value)
      {
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> two = Least significant byte
      byte two = (value & 0xFF);
      byte one = ((value >> 8) & 0xFF);
      

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, two);
      EEPROM.write(address + 1, one);
      
      }

uint16_t EEPROMRead16bits(int address)
      {
      //Read the 4 bytes from the eeprom memory.
      uint16_t two = EEPROM.read(address);
      uint16_t one = EEPROM.read(address + 1);
      

      //Return the recomposed long by using bitshift.
      return ((two << 0) & 0xFF) + ((one << 8)& 0xFFFF) ;
      }
  


//------------------------PRINT OUT ALL REGISTERED STUDENTS TO EXCEL----------------------
void printToXl()
{
  inc=3;//counts from the second row of col A and B 
   char strA[30];//for holding the changes on the code for stud.regNo
  char strB[30];//for ret.regNo
  char strC[30];//for no
  int no=1;//counts the numbers from 1 respectively
  //sessionsNum represents number of cols
  static int incChar='C';
  //variables for DATE at each session
  char str3[30];
 
   
   char str4[30];
  char str5[30];
  static int sessionsNum=1;//count number of attendance done to open another session

  //Serial.println("LABEL,REG.No");
  //  Serial.println("CLEARSHEET"); // clears starting at row 1
  
  delay(10);
  Serial.println("CELL,SET,B1,ID");
  
  
for(int a=0;a<studCounter;a++)
  {
////////////PRINT OUT NO////////////////////////////////////////////////////////
   Serial.println("CELL,SET,A1,No");
  sprintf(strC,"CELL,SET,A%d,",inc);//create a string for the data direction
  Serial.print(strC);
  Serial.println(no);//student number reached
////////////PRINT THE REG.No ON THE NEXT COL////////////////////////////////////////////////
     sprintf(strA,"CELL,SET,B%d,",inc);//create a string for the data direction
  Serial.print(strA);
  Serial.println(stud.regNo[a]);//DATA to be displayed
   delay(10);
   inc++;
   no++;
  }
//-------------------WRITING SESSIONS AT THE TOP-----------------------------------

  sprintf(str4,"CELL,SET,%c1,",incChar);//date are on the second line
  sprintf(str5,"SESSION%d",session);
  Serial.print(str4);//command to write in a col of row1
  Serial.println(str5);//print the session number to excel
  delay(10);
  incChar++;//increment to the next col
  session++;
///////////////////////////////////////////////////////////////////////////////////////////

 //----------------------PRINT THE DATE THE SESSION IS DUE--------------------------------
 sprintf(str3,"CELL,SET,%c2,",incChar2);//date are on the second line
  Serial.print(str3);
  Serial.println("DATE");
  delay(10);
  incChar2++;
  
}
//-----------------------------passing the Reg.No to excel for attendance records-------------
void attendanceToXl(long valToXl)
{
  char str7[30];
  char str6[30];

  delay (10);
    //inc 
  for(int Brows=inc;Brows>=1;Brows--)//inc helps navigate all the rows of B column
   {
   
    sprintf(str7,"CELL,GET,B%d",Brows);//create a string for the data direction
    Serial.println(str7);//goes to the B column of row Brows to read the value contained
    
    long valFromXl=Serial.readStringUntil(10).toInt();//reading string till its end(10 meaning in ASCII) and convert it
    
    //if the value pointed by B,Brows correspons to that of the pressed finger then √ attended
    if(valToXl==valFromXl)
    {
      //check today's session column and the row of valFromXl
      if(valToXl==valFromXl)
    {
      //check today's session column and the row of valFromXl
      
      sprintf(str6,"CELL,SET,%c%d,",incChar2-1,Brows);//date are on the second line
  Serial.print(str6);
  Serial.write("ATTENDED");
  Serial.println();
  delay(10);
  break;
      
      
    }
      
    }
    
  
}
 
}

void getTime()
{
  //printing the time on the first line
//DateTime now = rtc1.now();
//-------------PRINTING HOUR-------------------------------------------------
   if(hour<10)
  {
     lcd.setCursor(0,3);
     lcd.print("0"); 
    
    lcd.setCursor(1,3);
  lcd.print(hour, DEC);
    
  }
 //...
 else if(hour>=10)
  {
    lcd.setCursor(0,3);
    lcd.print(hour, DEC);
  }
  lcd.setCursor(2,3);
     lcd.print(":"); 
  //delay(70);
//------------------PRINTING MINUTES---------------------
if(minute<10)
  {
     lcd.setCursor(3,3);
     lcd.print("0"); 
    
    lcd.setCursor(4,3);
  lcd.print(minute, DEC);
    
  }
else if(minute>=10)
  {
    lcd.setCursor(3,3);
    lcd.print(minute, DEC);
  }
//-------------PRINTING SECONDS----------------------------

//lcd.setCursor(2,3);
 // lcd.print(":");
  //lcd.setCursor(3,3);
  //lcd.print(minute, DEC);
  lcd.setCursor(5,3);
  lcd.print(":");
  lcd.setCursor(6,3);
  lcd.print(second, DEC);
  
 //if seconds equals 0 then empty the (7,3) location
  if(second==0)
  {
    lcd.setCursor(7,3);
    lcd.print(" ");
  }
  //if minutes equals 0 then empty the(4,3)
  if(minute==0)
  {
    lcd.setCursor(4,3);
    lcd.print(" ");
  }
  //if hours equals 0 then empty the(1,3)
  if(hour==0)
  {
    lcd.setCursor(1,3);
    lcd.print(" ");
  }

//if at midnight then reset the pressed times during attendance
  if((hour==0)&&(minute==0)&&(second==0))
  {
    reset_pressed_times();
    
  }

}

void eepromWriteBytes(int address,int value)
{
  EEPROM.write(address,value);
}
void eepromReadBytes(int address)
{
  EEPROM.read(address);
}

//creating spaces in EEPROM memory for the stud.pressedTimes[]...110 spaces
//assing the EEPROM addresses to zeros
void initPressedTimes()
{
  for(int i=0;i<110;i++)
  {
    eepromWriteBytes(800+i,0);
  }
}


//get the name from the files
int getName(long regNo)//return the value that tells whether the code given is wrong or good to proceed the process
{
  int wrong=0;//the variable to keep the value indicating whether the code given is wrong or good to proceed the process
   int first2Vals=0;//read the 2 values of the regNo (CODE) entered (name the file)
   int lastVals=0;//read the remained digits of the entered CODE(name the number of line in a file)
  
  
  //--------------------------get two first values of the regNo to know the filename---
  //know the registration number of the user
  long number=regNo;
  int j=3;//index counter
  long arr[4]={};//array to keep the regNo in array form..to easily extract the 2 first values....max.number of the regNo is 4
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
    arr[3]=' ';
  }
  //get the two values.....call it first2Vals
  //-----------------------------------------read the first two values in first2Vals---------
  first2Vals=arr[0]*10+arr[1];
  //---------------------------------------------------------------------------------
  //--------------------------------------read the last values in lastVals----------------
  if(arr[3]== ' ')//if have only 3 values as a CODE
    lastVals=arr[2];
  else if(arr[3] != ' ')
    lastVals=arr[2]*10+arr[3];
  //--------------------------------------------------------------------------------
  
  //-----check if  the two values(got from the regNo) matches one of the 20 file names
  int z;
  for(z=0;z<20;z++)
  {
    //if the two values matches one of the fileName
    if(first2Vals==fileNames[z])//if the first 2 numbers of the regNo matches the fileName
    {
      //open the matched file name
      char file[9];//to hold the name of the file
      sprintf(file,"%d.csv",first2Vals);
//      Serial.println("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFILE SSSCV");
//      Serial.println(file);
      //creating a stream to open the file
      File fileStream=SD.open(file);//file string contains the file name
      //if the file is available
  if(fileStream)
  {
    while(fileStream.available())
    {
      char x=fileStream.read();
       //record to the string
      myString+=x;

    //Serial.print("prevlines:");
    //   Serial.println(prevLines);
     //  Serial.print("presLines:");
     //  Serial.println(presLines);
    if((x==' ') &&(presLines>prevLines))//if the space is encountered and the number of present lines greater than the number of previous lines
    {
      //record the myString value as the First name of the name entered
      prevLines=presLines;//for the fName string to include the first name
      Serial.println(fName);
      fName="";//empty the previous person's first name for the next person
      fName=myString;//keep the First name of the person
      //Serial.println(fName);
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
  homeScreen();
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
homeScreen();
return wrong;
}

//read key typed from the keypad.....1.COMFIRM 2.CANCEL
void switchKey()
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
        //get the user's fingerprint registered
        //if enter is pressed
    lcd.clear();
    lcd.setCursor(0,0);
    //lcd.print(F("FingerPrint:"));
    //FINGERPRINT ENROLL
    getFingerprintEnroll();//(int studbit)
    
        break;
        case '2': //if the user cancel;
        lcd.clear();
        homeScreen();//back to homeScreen
        break;
        default://waiting for either 1 or 2
        break;
       } 
       
} 

//-------------------------------------PRINT THE TIME THE ATTENDANCE IS DUE---------
void timeNow()
{
  
    //printing the time on the first line
//DateTime now = rtc1.now();
//-------------PRINTING HOUR-------------------------------------------------
   if(hour<10)
  {
     lcd.setCursor(9,1);
     lcd.print("0"); 
    
    lcd.setCursor(10,1);
  lcd.print(hour, DEC);
    
  }
 //...
 else if(hour>=10)
  {
    lcd.setCursor(9,1);
    lcd.print(hour, DEC);
  }
  delay(70);
//------------------PRINTING MINUTES---------------------
if(minute<10)
  {
     lcd.setCursor(12,1);
     lcd.print("0"); 
    
    lcd.setCursor(13,1);
  lcd.print(minute, DEC);
    
  }
else if(minute>=10)
  {
    lcd.setCursor(12,1);
    lcd.print(minute, DEC);
  }
//-------------PRINTING SECONDS----------------------------

lcd.setCursor(11,1);
  lcd.print(":");
  lcd.setCursor(12,1);
  lcd.print(minute, DEC);
  lcd.setCursor(14,1);
  lcd.print(":");
  lcd.setCursor(15,1);
  lcd.print(second, DEC);
  
 //if seconds equals 0 then empty the (7,3) location
  if(second==0)
  {
    lcd.setCursor(16,1);
    lcd.print(" ");
  }
  //if minutes equals 0 then empty the(4,3)
  if(minute==0)
  {
    lcd.setCursor(13,1);
    lcd.print(" ");
  }
  //if hours equals 0 then empty the(1,3)
  if(hour==0)
  {
    lcd.setCursor(10,1);
    lcd.print(" ");
  }
    //--------------------------------------------------------------------------
    
}

//creating a daily attendance file
void dailyAttendanceFile(long regNo)
{
 char getNameExt[15];//get today's file name with extension(.csv)
  long dailyAttendFiles[100];  //an index per day.....3 months period
  int noFileSignal=0;//if noFileSignal=0 means there is no today's signal 

int totalFiles=0;//number of the attendance files reached.....on the sd card
    //should be kept in the memory

//MUST HAVE DATA: regNo and TODAY'S DATE
//regNo---->will be fetched from the fingerprint ID

//today's date
//get today's date as the name of the file

      char getName[8];//date made up of 2digits, month 2 digits and year 4 digits to make 8 characters
      //DateTime now=rtc1.now();//declare the rtc variable type to fetch date,month and year
      if (date<10)
      sprintf(getName,"0%d%d20%d",date,month,year);
      else if (date>9)
      sprintf(getName,"%d%d%d",date,month,year);
      
      Serial.print("Name:");
      Serial.println(getName);//get the name of the file
/*       
SD.remove("attFiles.txt");
SD.remove("662018.csv");
*/
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
          //compare it with the today's file name
          if(temp==getName)
          { 
            noFileSignal=1;
            goto ATTEND;
            break;
          }
          temp="";//empty the temp to read the proceeding file name for comparison
        }
      }
    }
    attendFiles.close();
    
    if(noFileSignal==0)//if noFileSignal still 0 means there is no file opened for today
    {
      //create today's file

      //get first the name
     
      sprintf(getNameExt,"%s.csv",getName); 

     
      //use the name to create the file
      File todayFile;//create a file stream
      todayFile=SD.open(getNameExt);//create a file with the name of today's date
      todayFile.close();
      
      //save the name into the attFiles.txt
      File attendFiles=SD.open("attFiles.txt",FILE_WRITE);
      attendFiles.print(getName);
      attendFiles.print(" ");
      attendFiles.close();
      
    }
  //}

  ATTEND: //open the file to save in the user's regNo and time the attendance is due
  DS3231_read();
   DS3231_display();//to get instant time
   File todayFile=SD.open(getNameExt,FILE_WRITE);//open the file to write to
  todayFile.print(regNo);//print in the regNo of the user
   todayFile.print("(");// open parenthesis
   //print the time:08:40

   
   
   char getTimeNow[5]={};//23:00..must hold 5 characters
   //DateTime now2=rtc1.now();
   sprintf(getTimeNow,"%d:%d",hour,minute);
    todayFile.print(getTimeNow);//print the time the attendance is due
    todayFile.print("),");//  close parenthesis and a comma
    todayFile.close();// close file  


 todayFile=SD.open(getNameExt,FILE_READ);
    Serial.println(getNameExt);
    Serial.println("opening the getNameExt");
    if(todayFile)
    {
      while(todayFile.available())
      {
        char x=todayFile.read();
        Serial.print(x);
      }
      todayFile.close();
      Serial.println("");
      Serial.println("closing getNameExt");
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
void fileAddon(int dayCounter)
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

              //DateTime now = rtc1.now();
              addStr+=String(date)+"/"+String(month)+"/"+String(year);
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

void downloadFile()
{
//  //DOWNLOADABLE FILE=STR3=STR1+STR2
//  //STR1 IS GOTTEN FROM THE newList.csv
//  //STR2 IS GOTTEN FROM THE today's attendance file.csv...(time in + time out)
//  //STR3 =STR1+STR2 makes a line of the new desirable file downFile.csv
//
//String str1={};
//String str2={};
//String str3={};
//
//int commaCounter=0;
//int dayCounter=1;
//int line=1;
//int tempLine;
//String regStr;
//
//
//String regFile={};//will be holding the characters read from the attendance file
//char getNameExt[15];//get today's file name with extension(.csv)
//char download[15];//keep downloadable file name
//   String t1={},t2={};//to hold the TIME-IN AND THE TIME OUT recorded
//   int count=0;
//
//  //STEP1: get str1...line by line FROM THE newList.csv
//  //----------------------------------------------------------------------------------------
//  char fileName[15];
//  sprintf(fileName,"newFile%d.csv",dayCounter);
//  
//  File fileStr1=SD.open(fileName,FILE_READ);
//  Serial.print("file with addon:");
//  Serial.println(fileName);
//  if(fileStr1)
//  {
//    while(fileStr1.available())
//    {
//      char x=fileStr1.read();
//      str1+=x;
//      //Serial.print(x);
//      if(x==',')
//      commaCounter++;
//      
//      if(commaCounter==(4*dayCounter +4))//on the first two lines
//      { 
//        //append to a downloadable file
//  //--------------------------------------------------------------------------------------
//  dayCounter=EEPROM.read(911);
//  
//  sprintf(download,"download%d.csv",dayCounter);
//  SD.remove(download);//if any similar file first delete it...to save storage..no multiplication
//        File fileEdit=SD.open(download,FILE_WRITE);
//    if(fileEdit)
//    {
//      fileEdit.print(str1);
//      str1="";//clear the str1
//      fileEdit.close();
//      
//    }
// //---------------------------------------------------------------------------------------------   
//        commaCounter=0;
//        line++;
//      }
//       if((commaCounter==(4*dayCounter))&&(line>2))//on the remained lines
//      { 
//      
//
//      if((line>2)&&(line<=9))
//      {
//        tempLine=line-2;
//        regStr="11"+(String)tempLine;
//      }
//      if((line>9)&&(line<=12))
//      {
//        tempLine=line-9;
//        regStr="21"+(String)tempLine;
//      }
//      if((line>12)&&(line<=16))
//      {
//        tempLine=line-12;
//        regStr="31"+(String)tempLine;
//      }
//      
//      if((line>16)&&(line<=20))
//      {
//        tempLine=line-16;
//        regStr="32"+(String)tempLine;
//      }
//      if((line>20)&&(line<=22))
//      {
//        tempLine=line-20;
//        regStr="41"+(String)tempLine;
//      }
//      if((line>22)&&(line<=29))
//      {
//        tempLine=line-22;
//        regStr="42"+(String)tempLine;
//      }
//      if((line>29)&&(line<=42))
//      {
//        tempLine=line-29;
//        regStr="43"+(String)tempLine;
//      }
//      if((line>42)&&(line<=48))
//      {
//        tempLine=line-42;
//        regStr="44"+(String)tempLine;
//      }
//      if((line>48)&&(line<=50))
//      {
//        tempLine=line-48;
//        regStr="51"+(String)tempLine;
//      }
//      if((line>50)&&(line<=52))
//      {
//        tempLine=line-50;
//        regStr="52"+(String)tempLine;
//      }
//      if((line>52)&&(line<=58))
//      {
//        tempLine=line-52;
//        regStr="53"+(String)tempLine;
//      }
//      if((line>58)&&(line<=61))
//      {
//        tempLine=line-58;
//        regStr="54"+(String)tempLine;
//      }
//      if((line>61)&&(line<=66))
//      {
//        tempLine=line-61;
//        regStr="55"+(String)tempLine;
//      }
//      if(line==67)
//      {
//        tempLine=line-66;
//        regStr="61"+(String)tempLine;
//      }
//      if(line==68)
//      {
//        tempLine=line-67;
//        regStr="62"+(String)tempLine;
//      }
//      if((line>68)&&(line<=71))
//      {
//        tempLine=line-68;
//        regStr="63"+(String)tempLine;
//      }
//      if((line>71)&&(line<=74))
//      {
//        tempLine=line-71;
//        regStr="64"+(String)tempLine;
//      }
//      if((line>74)&&(line<=81))
//      {
//        tempLine=line-74;
//        regStr="65"+(String)tempLine;
//      }
//      if((line>81)&&(line<=87))
//      {
//        tempLine=line-81;
//        regStr="71"+(String)tempLine;
//      }
//      if((line>87)&&(line<=93))
//      {
//        tempLine=line-87;
//        regStr="81"+(String)tempLine;
//      }
//
//      Serial.print("regStr");
//      Serial.println(regStr);
//     
//
//    //STEP2: get str2...from the today's attendance
//  //----------------------------------------------------------------------------------------
//  
//
//  //open the today's attendance file
//  
//
//  //DateTime now=rtc1.now();//declare the rtc variable type to fetch date,month and year
//  char getName[15];
//      sprintf(getName,"%d%d%d",date,month,year);
//      
//  sprintf(getNameExt,"%s.csv",getName); 
//  File attendFiles=SD.open(getNameExt);//both writing and reading mode
//  Serial.print("today's file");
//  Serial.println(getNameExt);
// 
//  if(attendFiles)
//    {
//      while(attendFiles.available())
//      {
//        char x=attendFiles.read();
//        
//       if(regFile==regStr)//if the regNo in the attendance file == to the regNo in the newList.csv..chance to find out the both time in and the time out
//       {
//        
//        regFile="";//empty the string to store the time attended
//        count++;
//       }
//       
//       if((x=')')&&(count==1))
//       {
//        t1=regFile;
//        regFile="";
//       }
//       
//       if((x=')')&&(count>1))//get to know when he/she was lastly attended
//        {
//        
//          t2=regFile;
//          regFile="";
//          count++;
//        }
//         
//             
//
//        regFile+=x;
//      }
//      attendFiles.close();
//
//      //Now getting the STR2
//      str2=","+
//      t1+","+ t2+",,";
//      str3=str1+str2;
//      
//      Serial.println(str3);
//      
//            //append to a downloadable file
//  //--------------------------------------------------------------------------------------
//        File fileEdit=SD.open(download,FILE_WRITE);
//    if(fileEdit)
//    {
//      fileEdit.print(str3);
//      fileEdit.close();
//     }
//
//     str2="";
//     str1="";
//     str3="";
//
//      
// //---------------------------------------------------------------------------------------------   
//    
//    }
//  
// //-----------------------------------------------------------------------------------------
//      line++;
//      commaCounter=0;
//      }
//    }
//    fileStr1.close();
//  }
  //----------------------------------------------------------------------------------------
}



//----------------WRITE TO EXCEL--------------------------------------
void toXl()
{
  //open the file newFile3.csv
  //know the number of commas in a line for rows and colums

  int commaCounter=0;
  int dayCounter= EEPROM.read(911);
  String str={};
  char ch='A';//incrememtn the columns
  int line=1;//counts number of rows
  char strC[15];

  int numCommas=0;
  //set initial destination to write data to first row,first col
  Serial.println("CLEARSHEET"); // clears starting at row 1

  //.println("CELL,SET,A1,No");
  char download[15]={};//keep downloadable file name
  dayCounter=EEPROM.read(911);
  
  //Serial.print("dayCounter:");
  //Serial.println(dayCounter);
  
  //sprintf(download,"down%d.csv",dayCounter);
  //Serial.print("downloadable file:");
  //Serial.println(download);
//  
  //File downloadFile=SD.open(download,FILE_READ);
File downloadFile=SD.open("newFile0.csv",FILE_READ);

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

void daily_Attendance_To_PySerial()
{
    char x ;
     char getName[8];//date made up of 2digits, month 2 digits and year 4 digits to make 8 characters
      //DateTime now=rtc1.now();//declare the rtc variable type to fetch date,month and year
      if(date > 9)
      sprintf(getName,"%d%d20%d",date,month,year);
      else if(date<10)
      sprintf(getName,"0%d%d20%d",date,month,year);
//      Serial.print("Name:");
//      Serial.println(getName);//get the name of the file

sprintf(getNameExt,"%s.csv",getName); 
//  f  Serial.print("getnameExt:");
//    Serial.println(getNameExt);
File file = SD.open(getNameExt,FILE_READ);

if (file)
{
  while (file.available())
  {
     x = file.read();
     Serial.print(x);
  }
  file.close();
  //Serial.print("\nEnd of file reached");
  Serial.print("]");
}
}


void delete_File()
{
  SD.remove("attFiles.txt");
  SD.remove("0372018.csv");

  for (int i=505;i<605;i++)
  {
    EEPROM.write(i,0);
  }
 EEPROM.write(RTC_setting_loc,0);
}



//---------------------------EEPROM FUNCTIONS------------------------------------
void reading_studCounter_value_in_EEPROM()
{
  studCounter = EEPROM.read(0);
//  Serial.print("studCounter:");
//    Serial.println(studCounter);
}


void reading_fingerprint_id_locations_in_EEPROM()
{
  id = EEPROM.read(id_loc);
//  Serial.print("id:");
//    Serial.println(id);
}

void reading_student_regNo_saved_in_EEPROM()
{
  for (int i=0;i<studCounter;i++)
  {
    stud.regNo[i] = EEPROMRead16bits((regNo_starting_loc+(i*2)));//2 for 2 bytes spaces
//    Serial.print("stud.regNo[");
//    Serial.print((regNo_starting_loc+(i*2)));
//    Serial.print(":");
//    Serial.println(stud.regNo[i]);
    }
}

void reading_student_fingerprints_saved_in_EEPROM()
{
  for (int i=0;i<studCounter;i++)
  {
    stud.prints[i] = EEPROM.read(fprint_starting_loc+i);
//    Serial.print("stud.prints[");
//    Serial.print(fprint_starting_loc+i);
//    Serial.print(":");
//    Serial.println(stud.prints[i]);
  }
}

void reading_pressed_times_saved_in_EEPROM()
{
  for (int i=0;i<studCounter;i++)
  {
    stud.pressedTimes[i] = EEPROM.read(pressed_time_startIng_loc+i);
//    Serial.print("stud.pressedTimes[");
//    Serial.print(pressed_time_startIng_loc+i);
//    Serial.print(":");
//    Serial.println(stud.pressedTimes[i]);
  }
}
//--------------------------------------------------------------------------------

void reset_pressed_times()
{
  for(int z=505;z<604;z++)
   {
    EEPROM.write(z,0);//clear all the EEPROM locations by writing zero to all positions
   }
}

//--------------------------SETTING RTC CLOCK----------------------------------
//-----------------------------------RECORDING THE rtc1 TIME IN A FILE-------
void saving_clock_time_to_file()
{
//  //DateTime now=rtc1.now();
//  File fileStr=SD.open("clock.txt",FILE_WRITE);
//  Serial.print(fileStr);
//    if(fileStr)
//    {
//      Serial.println("inside the savving clock");
//      Serial.print("here");
//      fileStr.print(hour);
//      Serial.println(hour);
//      fileStr.print(":");
//      fileStr.print(minute);
//      Serial.println(minute);
//      fileStr.print(":");
//      fileStr.print(second);
//      Serial.println(second);
//      fileStr.print(":");
//
//      fileStr.print(date);
//      fileStr.print(":");
//      fileStr.print(month);
//      fileStr.print(":");
//      fileStr.print(year);
//      fileStr.print(":");
//      
//     
//    }
//    fileStr.close(); 
//    Serial.println("leaving the saving clock");
// //-------------------READING THE FILE-----------------------------------
// File attendFiles=SD.open("clock.txt",FILE_READ);
//    Serial.println("reading the clock content");
//    if(attendFiles)
//    {
//      while(attendFiles.available())
//      {
//        char x=attendFiles.read();
//        Serial.print(x);
//      }
//      attendFiles.close();
//      //Serial.println("");
//    }
////--------------------------------------------------------------------------------
//else if (!fileStr)
//{
//  rtc1.begin(DateTime(F(__DATE__), F(__TIME__)));
//}
}

void setup_clock()
{
//  int delimiter =0;//":"
//  String string;
//  int rec_hour,rec_min,rec_second,rec_day,rec_month,rec_year;
//File   fileStr=SD.open("clock.txt",FILE_READ);
//    
//    if(fileStr)
//    {
//      Serial.println("inside the setup_clock");
//      while(fileStr.available())
//      {
//        char x=fileStr.read();
//        Serial.print(x);
//
//        if (x == ':')
//        {
//          delimiter ++;
//          if(delimiter == 1)
//          {
////            Serial.print("str:");
////            Serial.println(string);
//            rec_hour = string.toInt();
//            
////            Serial.print("rec_hour:");
////            Serial.println(rec_hour);
//            string = "";
//            goto END;
//          }
//          else if(delimiter == 2)
//          {
////            Serial.print("str:");
////            Serial.println(string);
//            
//            rec_min = string.toInt();
//
////            Serial.print("rec_min:");
////            Serial.println(rec_min);
//            string = "";
//            goto END;
//          }
//          else if(delimiter == 3)
//          {
////            Serial.print("str:");
////            Serial.println(string);
//            rec_second = string.toInt();
////
////            Serial.print("rec_second:");
////            Serial.println(rec_second);
//            string = "";
//            goto END;
//          }
//          else if(delimiter == 4)
//          {
//            rec_day = string.toInt();
//
//            string = "";
//            
//          }
//          else if(delimiter == 5)
//          {
//            rec_month = string.toInt();
//            string = "";
//            goto END;
//          }
//          else if(delimiter == 6)
//          {
//            rec_year =string.toInt();
//            string = "";
//            delimiter = 0;
//            goto END;
//          }
//        }
//        string +=x;
//      END:
//          continue;
//      }
//      fileStr.close();
//      Serial.println("closing the setup clock");
//      //set the clock
//      // January 21, 2014 at 3am you would call:
//    // rtc1.adjust(DateTime(2014, 1, 21, 3, 0, 0));
//
//    Serial.print("rec_year:");
//    Serial.println(rec_year);
//    Serial.print("rec_month:");
//    Serial.println(rec_month);
//    Serial.print("rec_day:");
//    Serial.println(rec_day);
//    Serial.print("rec_hour:");
//    Serial.println(rec_hour);
//    Serial.print("rec_min:");
//    Serial.println(rec_min);
//    
//    rtc1.adjust(DateTime(rec_year,rec_month,rec_day,rec_hour,rec_min, rec_second));
//    
//    }
//    
}

//---------------------RTC CLOCK DEFINITIONS-----------------------------------------------
void DS3231_read(){                             // Function to read time & calendar data
  Wire.beginTransmission(0x68);                 // Start I2C protocol with DS3231 address
  Wire.write(0);                                // Send register address
  Wire.endTransmission(false);                  // I2C restart
  Wire.requestFrom(0x68, 7);                    // Request 7 bytes from DS3231 and release I2C bus at end of reading
  second = Wire.read();                         // Read seconds from register 0
//  Serial.println(second);
  minute = Wire.read();                         // Read minuts from register 1
//  Serial.println(minute);
  hour   = Wire.read();                         // Read hour from register 2
//  Serial.println(hour);
  day    = Wire.read();                         // Read day from register 3
//  Serial.print(day);
  date   = Wire.read();                         // Read date from register 4
//  Serial.println(date);
  month  = Wire.read();                         // Read month from register 5
//  Serial.println(month);
  year   = Wire.read();                         // Read year from register 6
//  Serial.println(year);
}

void calendar_display(){                        // Function to display calendar
  switch(day){
    case 1:  strcpy(calendar, "Sun   /  /20  "); break;
    case 2:  strcpy(calendar, "Mon   /  /20  "); break;
    case 3:  strcpy(calendar, "Tue   /  /20  "); break;
    case 4:  strcpy(calendar, "Wed   /  /20  "); break;
    case 5:  strcpy(calendar, "Thu   /  /20  "); break;
    case 6:  strcpy(calendar, "Fri   /  /20  "); break;
    case 7:  strcpy(calendar, "Sat   /  /20  "); break;
    default: strcpy(calendar, "Sat   /  /20  ");
  }
  calendar[13] = year  % 10 + 48;
  calendar[12] = year  / 10 + 48;
  calendar[8]  = month % 10 + 48;
  calendar[7]  = month / 10 + 48;
  calendar[5]  = date  % 10 + 48;
  calendar[4]  = date  / 10 + 48;
//  Serial.println(calendar);
  //lcd.print(calendar);                          // Display calendar
}

void DS3231_display(){
  // Convert BCD to decimal
  second = (second >> 4) * 10 + (second & 0x0F);
  minute = (minute >> 4) * 10 + (minute & 0x0F);
  hour = (hour >> 4) * 10 + (hour & 0x0F);
  date = (date >> 4) * 10 + (date & 0x0F);
  month = (month >> 4) * 10 + (month & 0x0F);
  year = (year >> 4) * 10 + (year & 0x0F);
  // End conversion
  Time[7]     = second % 10  + 48;
  Time[6]     = second / 10  + 48;
  Time[4]     = minute % 10  + 48;
  Time[3]     = minute / 10  + 48;
  Time[1]     = hour   % 10  + 48;
  Time[0]     = hour   / 10  + 48;
  
  calendar_display();                           // Call calendar display function
  lcd.setCursor(0,2);
  //lcd.print(Time);                              // Display time
//  Serial.println("time:");
//  Serial.print(Time);
}



//---------------------------------------------------------------------------

