#include <EEPROM.h>
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(29,31,33,35,37,39,41,43,45,47);//lcd in 8 bit mode
//rs,e,d0 to d7

//#include <Wire.h>
#include "RTClib.h"//for rtc clock
RTC_DS3231 rtc;
RTC_Millis rtc1;//to take time from software based on millis()and timer

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
int userCount;

static int lectCounter=0;//has not to exceed 29 (0-30)
static int lessonCounter=0;
static int retakerLessons=0;

static int studCounter=0;//has not to exceed 29 (0-30)

static int retakerCounter=0;

int lectFinger;

int arrCode[10];//array to keep every input data in its array
char key;

//for fingerprint to eeprom

  int i;//for iteration

 //for the rtc to print the exact day in words
 char daysOfTheWeek[7][12] = {"Sund", "Mond", "Tues", "Wedn", "Thur", "Frid", "Satu"};

    
//creating  file pointer
File myfiles1;
File myfiles2;

//for fingerprint to eeprom
static int id=0;//takes all the users'fingerprint




static int studLoc=30;//students'eeprom position counter
static int studLoc1=500;//location starting for students registration numbers
static int studLoc2=60;//location starting for students fingerprints ID's
static int studLoc3=90;

static int pressedTimesCounter=800;

int fileCode[10]={};//fileCode will hold the entered MINIJUST CODE
int fileNames[20]={11,21,31,32,41,42,43,44,51,52,53,54,55,61,62,63,64,65,71,81};//20 files 
String fName;
//adding EEPROM to hold the fingerprints

//student's records

struct students{
        
     
        PROGMEM int studID[30];//take the number of students registered consecutively
        PROGMEM long regNo[30];
        PROGMEM int prints[30];//will take the value of the fingerprint scanned
        PROGMEM int pressedTimes[30]={};//record how many times does a user attempted to attend(either 
        }stud;


void setup() {

  lcd.begin(20,4);
  //lcd.clear();
  Serial.begin(9600);
  delay(10);

 //clears all data on activeSheet of excel if available
  Serial.println("CLEARSHEET");

  
    
  //SD card settings
  pinMode(chipSelect,OUTPUT); //declare the CS as an OUTPUT
  //SD.begin(chipSelect);
  
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
  // following line sets the RTC to the date & time this sketch was compiled
    rtc1.begin(DateTime(F(__DATE__), F(__TIME__)));
   //or set  January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    // January 21, 2014 at 3am you would call:
    // rtc1.adjust(DateTime(2017, 5, 10, 11, 59, 0));
  }
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  // set the data rate for the sensor serial port
  finger.begin(57600);
  //-----------------------FINGERPRINT SCANNER PRESENCE TEST------------------------
  if (finger.verifyPassword()) //
  {
    ////Serial.println("Found fingerprint sensor!");
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print(F("MINISTRY OF"));
    lcd.setCursor(4,1);
    lcd.print(F(" JUSTICE"));
    Serial.println(" fingerprint working");

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

 //--------------------------------------------------------------------------------------

 //---------------------------------------------------------------------------------
 //----------------------RESTORE STUDENTS REG.No FROM EEPROM-------------------------------
 if(EEPROM.read(233)>0);
 id=EEPROM.read(233);//takes the last fingerprint template id registered
 
 if(EEPROMRead16bits(200)>500)//TO PREVENT THE LOSS OF THE INITIAL VALUES
 studLoc1=EEPROMRead16bits(200);

if(EEPROM.read(251)>0)
  studCounter=EEPROM.read(251);

  int r=500;
    
   
   for(int z=0;z<studCounter;z++)//index counter for the students registration numbers
      { 
        while(r<=studLoc1)
        {
          stud.regNo[z]=EEPROMReadlong(r);
          Serial.print("studCounter value:");
          Serial.println(studCounter);
          Serial.print("studLoc1");
          Serial.println(studLoc1);
          Serial.print("r values:");
          Serial.println(r);
          Serial.println(stud.regNo[z]);
         
          break;
        }
        r=r+4;
    }
//------------------------------------------------------------------------------------------
//----------------------RESTORE STUDENTS PRINTS FROM EEPROM-------------------------------
 
 if(EEPROM.read(226)>60)//TO PREVENT THE LOSS OF THE INITIAL VALUES
  studLoc2=EEPROM.read(226);
  
 Serial.print("value in studloc2:");
          Serial.println(studLoc2);
 Serial.print("value in studCounter");
 Serial.println(studCounter);

          int r2=60;
 for(int z=0;z<studCounter;z++)//index counter for the students 
      {
       
        while(r2<=studLoc2)
        {
          
          stud.prints[z]=EEPROM.read(r2);
          Serial.print("value in stud.prints[z]:");
          Serial.println(stud.prints[z]);
        
          //Serial.println(r2);
          break;
          
        }
      
        r2++;
       
    }
//----------------------------------------------------------------------------------------

  

homeScreen();
}

void loop()
{

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
      case 'D'://format device
      delay(1);
          deleteAllPrints();
          
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

   void studentReg()
  {
    lcd.clear();
    lcd.setCursor(0,0);
    
    lcd.print(F("CODE:"));
    lcd.blink();
    stud.regNo[studCounter]=codeInput(1,3);//input codes not more than 10 digits on the second row
    
    //-----------------------writing entered registration number to EEPROM----------------
    EEPROMWritelong(studLoc1,stud.regNo[studCounter]);
    Serial.println("Reg.No written to EEPROM");
    Serial.println(EEPROMReadlong(studLoc1));
    
    //jump other 3 spaces for the next registration number
    studLoc1=studLoc1+4;
    Serial.print("value of incremented studLoc1:");
    Serial.println(studLoc1);
    //---------store the incremented value
     EEPROMWrite16bits(200,studLoc1);//as studLoc1 is a 16bits value
    //EEPROM.write(225,studLoc1);
    Serial.print("studLoc1 EEPROM VALUE:");
    Serial.println(EEPROMRead16bits(200));
    
    ////-----------------------------------------------------------------------------------
    //-----------------READ the corresponding name for the entered REG.No-----------
     int returnedVal=getName(stud.regNo[studCounter]);//call the function to pull the name 
     if(returnedVal)//if the returned value is 1 means the input code is wrong else continue the process
     goto END;
     
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("CODE:  ");
    lcd.print(stud.regNo[studCounter]);
    lcd.setCursor(0,1);//second line
    lcd.print("NAME:");
    lcd.print(fName);//print out the first name corresponding to the entered code
    Serial.print("FFFFFFFFFFFFFFFFFFFFNAMEMMMMMMMMMMMMMMMMMM");
    Serial.println(fName);
    
    
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
  Serial.print("studCounter");
 Serial.println(studCounter);
 Serial.print("stud.prints");
 Serial.println(stud.prints[i]);
 Serial.print("finger.fingerID");
 Serial.println(finger.fingerID);
  if(stud.prints[i]==finger.fingerID)
  {
    
    Serial.println("REACHED");
    lcd.clear();
    lcd.setCursor(0,0);
    //-------------------------GET THE PERSON'S NAME FROM THE FILE-------------------
    lcd.print("NAME:");
    lcd.setCursor(0,1);

    //recall the pressed times by the user from the EEPROM
    stud.pressedTimes[i]=EEPROM.read(800+i);//800 is the 1st EEPROM address for pressedTimes
    
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
   EEPROM.write(800+i,stud.pressedTimes[i]);//pressedTimes EEPROM memory starts from(800-910)
    
  //PRINT THE TIME THE ATTENDANCE IS DUE ON THE SCREEN
  timeNow();
  
   break;//if the target PERSON's data processed...... then break 
  }
  
 }
 

    
    
    //know how many times has a one fingerprint been recorded
    //who is the student?
    //his location
    //save it to EEPROM to prevent data loss
    
   
    Serial.println("the finger id detected:");
    Serial.println(finger.fingerID);
    
    
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

  //store the model of the id
  p = finger.storeModel(id);//store calculated model for later matching
  if (p == FINGERPRINT_OK) {
    //-------------------------FINGER IS STORED-----------------------------------------
    ////Serial.println("REGISTERED");
   // lcd.clear();
   //printId(userCount);
    lcd.setCursor(2,4);
    lcd.print(F("REGISTERED!"));
    

    
    
    
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
 
//---------------------------ENROLLING A STUDENT---------------------------------------------------
 
    stud.studID[studCounter]=studCounter;
    stud.prints[studCounter]=id;//save the finger//printId of a student

//---------------------STORING THE FINGERPRINT ID IN THE EEPROM------------------------
    Serial.println("fingerprint value kept in  stud.prints[j] ");
        Serial.println(stud.prints[studCounter]);
        EEPROM.write(studLoc2,stud.prints[studCounter]);
        
        Serial.println("value in EEPROM:");
        Serial.println(EEPROM.read(studLoc2));
        studLoc2++;
        Serial.println("---------->value in studeLoc2");
        Serial.println(studLoc2);
        EEPROM.write(226,studLoc2);//store the incremented value in EEPROM
        
//--------------------------------------------------------------------------------------



  
     studCounter++;

    //UPDATE studCounter VARIABLE IN THE EEPROM ADDRESS
    EEPROM.update(251,studCounter);
    userCount=studCounter;
    
    id++;// increase the number of overall registered users(lecturer,student,retaker)
    EEPROM.update(233,id);//save to EEPROM

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
  delay(70);
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

lcd.setCursor(2,3);
  lcd.print(":");
  lcd.setCursor(3,3);
  lcd.print(now.minute(), DEC);
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
int getName(long regNo)//return the value telling whether the code given is wrong or good to proceed the process
{
  int wrong=0;//the variable to keep the value indicating whether the code given is wrong or good to proceed the process
   int temp=0;//read the 2 values of the regNo (CODE) entered (name the file)
   int temp2=0;//read the remained digits of the entered CODE(name the number of line in a file)
  
  
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
    arr[3]=0;
  }
  //get the two values.....call it temp
  //-----------------------------------------read the first two values in temp---------
  temp=arr[0]*10+arr[1];
  //---------------------------------------------------------------------------------
  //--------------------------------------read the last values in temp2----------------
  if(arr[3]==0)//if have only 3 values as a CODE
    temp2=arr[2];
  else if(arr[3]!=0)
    temp2=arr[2]*10+arr[3];
  //--------------------------------------------------------------------------------
  
  //-----check if  the two values(got from the regNo) matches one of the 20 file names
  int z;
  for(z=0;z<20;z++)
  {
    //if the two values matches one of the fileName
    if(temp==fileNames[z])//if the first 2 numbers of the regNo matches the fileName
    {
      //open the matched file name
      char file[9];//to hold the name of the file
      sprintf(file,"%d.csv",temp);
      Serial.println("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFILE SSSCV");
      Serial.println(file);
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
    
    if((x==' ') &&(presLines>prevLines))//if the space is encountered and the number of present lines greater than the number of previous lines
    {
      //record the myString value as the First name of the name entered
      presLines=prevLines;//for the fName string to include the first name

      fName="";//empty the previous person's first name for the next person
      fName=myString;//keep the First name of the person
      //Serial.println(fName);
    }
    //if the Fstream.read encounters the comma....reading next person's names
     if(x==',')
     {
      
       
      prevLines=presLines; 
      presLines++;//increment the number of lines variable
      //break if the target person's name is passed
      //the last values of the CODE determine the target name (kept in temp2)
      if(temp2==presLines)//johnstone busingye,
      break;
      
     }  
    }
    fileStream.close();//close the file after reading
  }
 //--------------------------------------------------------------------------------------
else //if not available
  Serial.println("error opening the file");
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

//read key typed from the keypad
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
       
void timeNow()
{
  
 //-------------------------------------PRINT THE TIME THE ATTENDANCE IS DUE---------
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


