#include <Arduino.h>
#include "Wire.h"
#include <SPI.h>
#include <Adafruit_MCP23X17.h>
#include "defines.h"

#include <pb_encode.h>
#include <pb_decode.h>
#include <pb_common.h>

#include "pb_arduino.h"

#include "epicethernetinput.pb.h"
#include "epicethernetoutput.pb.h"

#include <ArduinoJson.h>


//#include "pio_without_options.pb.h"

#define DEVICENAME "EPIC"
#define SIZEOFNAME 4

#define SIZE_TIMESTAMP sizeof(unsigned long)

#define NB_INPUT 8
#define NB_CHIP 24
#define nbDigInput NB_CHIP *NB_INPUT
#define nbAnaInput 16

#define MAC_INDEX 0
#define  vitesseTrame 15 //envoie la trame toutes les x millisecondes

//#define DEBUG   //permet d'afficher les traces
//#define DEBUG_EXECUTION_TIME //permet d'afficher les temps d'execution

Adafruit_MCP23X17 digOutput1; // carte sorties digital
Adafruit_MCP23X17 digOutput2;

uint8_t addressMCP1 = 0x20;
uint8_t addressMCP2 = 0x21;

const int anaPin[nbAnaInput] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15};

// port L pour les DATA
const int dataPin[NB_INPUT] = {49, 48, 47, 46, 45, 44, 43, 42};

// liste des chip select    attention 18/19 serial      20/21 I2C  50/51/52/53 SPI
const int chipsSelect[NB_CHIP] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 22, 23, 24, 25, 26, 27, 28,29};

uint8_t operation=0;

struct STRUCT
{
  byte digInput[NB_CHIP];
  byte anaInput[nbAnaInput*2]; // int
} message;

const int NB_LEC_DEBOUNCE=3;
byte tempLec[NB_LEC_DEBOUNCE][NB_CHIP];
unsigned long startTime;
bool reading=false;
unsigned long lastTime=0;



EthernetServer serveur(4200);



/**
 *
 *          resetchipselect
 *
 *
 **/

void resetchipselect()
{
  for (int i = 0; i < NB_CHIP; i++)
  {
    digitalWrite(chipsSelect[i], HIGH);
    //Serial.println(chipsSelect[i]);
  }
}

/*
*
*
*   Lecture du port L en entier pour lire 8 bit de données en un seul coup
*
*/
byte readPort()
{
  return PINL;
}




/**
 *
 *          initchipselect
 *
 *
 **/
void initchipselect()
{
  for (int i = 0; i < NB_CHIP; i++)
  {
    pinMode(chipsSelect[i], OUTPUT);
  }
  resetchipselect();
}



/**
 * 
 * 
 * 
 * 
 * 
*/
/*

bool encode_digInput(pb_ostream_t* stream, const pb_field_t* field, void* const* arg)
{
    DigitalInput digIn=DigitalInput_init_zero;
     resetchipselect();
    //delayMicroseconds(2);
    for (int i = 0; i < NB_CHIP; i++)
    {
      if (i > 0)
      {
        digitalWrite(chipsSelect[i - 1], HIGH);
      }
      digitalWrite(chipsSelect[i], LOW);
      
      delayMicroseconds(5);
      
      uint8_t val=45;
      val=readPort(); //pour inverser ajouter ~ devant
      for(int j=0;j<8;j++){
            digIn=AnalogInput_init_zero;
            int val=bitRead(val,j);
            //digIn.id=i*8+j;
            digIn.value=val;
            #ifdef DEBUG
              Serial.print( " ");
              Serial.print(digIn.id);
              Serial.print (" ");
              Serial.println(val);
            #endif
            if (!pb_encode_tag_for_field(stream, field))
           {
            Serial.println("encode tag  failed dig in");
              return false;
           }

            if( pb_encode_submessage(stream,AnalogInput_fields,&digIn) == false)
            {
              Serial.println("encode failed dig in");
              return false;
            }    
      }

     
    } 
    return true;
}
*/

/**
 * 
 * 
 * 
 * 
 * 
*/

/*
bool encode_anaInput(pb_ostream_t* stream, const pb_field_t* field, void* const* arg)
{
    AnalogInput anaIn=AnalogInput_init_zero;
     for(int i=0;i<nbAnaInput;i++){
          anaIn=AnalogInput_init_zero;
          
          int val=analogRead(anaPin[i]);
          //anaIn.id=i;
          anaIn.value=(float)val;
          #ifdef DEBUG
          Serial.print( " ");
          Serial.print(i);
          Serial.print (" ");
          Serial.println(val);
          #endif
           if (!pb_encode_tag_for_field(stream, field))
           {
              return false;
           }

           

          if(pb_encode_submessage(stream,AnalogInput_fields,&anaIn) == false)
          {
            Serial.println("encode sub anainput failed");
            return false;
          }    
    }

    return true;
}
*/

bool encode_string(pb_ostream_t* stream, const pb_field_t* field, void* const* arg)
{
    const char* str = (const char*)(*arg);

    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, (uint8_t*)str, strlen(str));
}



bool decode_string(pb_istream_t *stream, const pb_field_t *field,  void **arg)
{
    byte buffer[1024] = {0};
    
    /* We could read block-by-block to avoid the large buffer... */
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;
    
    /* Print the string, in format comparable with protoc --decode.
     * Format comes from the arg defined in main().
     */
    String ch=String((char*)buffer);
    //Serial.println( ch);
    return true;
  
}

void p(byte X) {

   if (X < 10) {Serial.print("0");}

   Serial.println(X, HEX);

}

bool decode_anainputs(pb_istream_t *stream, const pb_field_t *field,  void **arg)
{
    //Serial.println("call decodelist anainput");
   AnalogInput anain = AnalogInput_init_zero;
   if (!pb_decode(stream,AnalogInput_fields,&anain)){
      Serial.print("error decoding anainput: ");
      Serial.println(stream->errmsg);
   }
  
  #ifdef DEBUG
    Serial.print("id ");
    Serial.print(anain.id);
    Serial.print(" value ");
    Serial.println(anain.value);
   #endif
   
  
  
}



/**
 * 
 * 
 *    lecture des entrees et envoie du message en protobuf
 * 
 * 
 * 
*/

void readAndSendInputPB(EthernetClient client){

 

  uint8_t buffer[2048];
  size_t message_length;
 bool status;
  //send
  simInput epicIn = simInput_init_zero;
  DigitalInput digIn=DigitalInput_init_zero;

  Serial.print("start millis");
  Serial.println(millis());
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
        
        
        epicIn.numberAnalogInput=16;
        epicIn.numberDigitalInput=192;
        strcpy(epicIn.DeviceName,"EPIC");
        //epicIn.DeviceName.arg = "EPIC";
        //epicIn.DeviceName.funcs.encode = &encode_string;

  Serial.print("devicename millis");
  Serial.println(millis());
         String macStr="";
         String hexstring="";
        for(int i=0;i<6;i++){
            hexstring="";
            if(mac[MAC_INDEX][i] < 0x10) {
              hexstring += '0';
            }

            hexstring +=String(mac[MAC_INDEX][i], HEX);
            
          macStr+= hexstring +":";
        }
        macStr=macStr.substring(0,macStr.length()-1);
        //Serial.println(macStr);
        //epicIn.MacAddress.arg = macStr.c_str();
        //epicIn.MacAddress.funcs.encode = &encode_string;
          strcpy(epicIn.MacAddress,macStr.c_str());
     
    
     resetchipselect();
    //delayMicroseconds(2);
    for (int i = 0; i < NB_CHIP; i++)
    {
      if (i > 0)
      {
        digitalWrite(chipsSelect[i - 1], HIGH);
      }
      digitalWrite(chipsSelect[i], LOW);
      
      delayMicroseconds(5);
      
      digIn=DigitalInput_init_zero;
      uint8_t val=45;
      val=readPort(); //pour inverser ajouter ~ devant
      for(int j=0;j<8;j++){
            digIn=AnalogInput_init_zero;
            int val=bitRead(val,j);
            digIn.id=i*8+j;
            digIn.value=val;
            #ifdef DEBUG
              Serial.print( " ");
              Serial.print(digIn.id);
              Serial.print (" ");
              Serial.println(val);
            #endif
             epicIn.diginputs[i*8+j]=digIn;
      }
     

     
    } 
       

       
        //epicIn.anainputs.funcs.encode= &encode_anaInput;

        //epicIn.diginputs.funcs.encode= &encode_digInput;


        epicIn.timeStamp=millis();

        /* Now we are ready to encode the message! */
        status = pb_encode(&stream, simInput_fields, &epicIn);
        message_length = stream.bytes_written;
        
        /* Then just check for any errors.. */
        if (!status)
        {
            Serial.print("Encoding failed:");
            Serial.println(PB_GET_ERROR(&stream));
            
        }
      Serial.print("encoding millis");
  Serial.println(millis());
  Serial.println();
        /*
        for(int k=0;k<message_length;k++){

          p(buffer[k]);

        }
        */

        if(client){
          client.write((char *)buffer,message_length);
        }

         #ifdef DEBUG
        Serial.print("message length ");
        Serial.println(message_length);
  
        Serial.println("call decode epicinput");
        Serial.print("message length ");
        Serial.println(message_length);
      
        EpicEthernetInput epicIntmp = EpicEthernetInput_init_zero;
        pb_istream_t istreamtmp = pb_istream_from_buffer(buffer, message_length);

        epicIntmp.DeviceName.funcs.decode=decode_string;
        epicIntmp.MacAddress.funcs.decode=decode_string;
        //epicIntmp.anainputs.funcs.decode=decode_anainputs;

        if (!pb_decode(&istreamtmp,EpicEthernetInput_fields,&epicIntmp)){
            Serial.print("error decoding: ");
            Serial.println(istreamtmp.errmsg);
        }
        else{
            Serial.println("decode epic input ok");
            
        }
  
      Serial.println();
      #endif


}


void readAndSendInputJson(EthernetClient client)
{
   Serial.print("start millis");
  Serial.println(millis());
  DynamicJsonDocument doc(2048);
  doc["platine"]="EPIC";
  String macStr="";
         String hexstring="";
        for(int i=0;i<6;i++){
            hexstring="";
            if(mac[MAC_INDEX][i] < 0x10) {
              hexstring += '0';
            }

            hexstring +=String(mac[MAC_INDEX][i], HEX);
            
          macStr+= hexstring +":";
        }
        macStr=macStr.substring(0,macStr.length()-1);
  doc["Mac"]=macStr;

  for (int i = 0; i < NB_CHIP; i++)
    {
      if (i > 0)
      {
        digitalWrite(chipsSelect[i - 1], HIGH);
      }
      digitalWrite(chipsSelect[i], LOW);
      
      delayMicroseconds(5);
      
      uint8_t val=45;
      val=readPort(); //pour inverser ajouter ~ devant
      doc["DI"][i]=val;
      Serial.print(val,BIN);
     
    } 
  for(int i=0;i<nbAnaInput;i++)
  {
    int val=analogRead(anaPin[i]);
     doc["AI"][i]=val;
    //Serial.println(val);
        
  }
  
  
  doc["timestamp"]=millis();
  byte dataToWrite[2048];
  size_t realSize=serializeJson(doc, dataToWrite);

  Serial.print("realsize ");
  Serial.println(realSize);

  client.write(dataToWrite,realSize);
   Serial.print("end millis");
  Serial.println(millis());

}
/**
 * 
 * 
 *    lecture des entrees et envoie du message optimizé
 * 
 * 
 * 
*/



void readAndSendInputOptimize(EthernetClient client){

    String DeviceName = DEVICENAME;
    Serial.print("start millis ");
    Serial.println(millis());
    byte byteName[SIZEOFNAME+1];
    DeviceName.getBytes(byteName, SIZEOFNAME+1);



    byte bufferData[NB_CHIP+nbAnaInput*2+ SIZE_MAC + SIZE_TIMESTAMP + SIZEOFNAME]; //80= 24 digital + 32 ana + 6 mac + 4 timestamp + 4 nom
     resetchipselect();
    //delayMicroseconds(2);
    for (int i = 0; i < NB_CHIP; i++)
    {
      if (i > 0)
      {
        digitalWrite(chipsSelect[i - 1], HIGH);
      }
      digitalWrite(chipsSelect[i], LOW);
      
      delayMicroseconds(5);
      
      uint8_t val=45;
      val=readPort(); //pour inverser ajouter ~ devant
      bufferData[i]=val;
     // Serial.print(val,BIN);
     
    } 
    //Serial.println();
   
   int indexByte=NB_CHIP;
    for(int i=0;i<nbAnaInput;i++){
          int val=analogRead(anaPin[i]);
          #ifdef DEBUG
          Serial.print( " ");
          Serial.print(i);
          Serial.print (" ");
          Serial.println(val);
          /*
          Serial.print("val first byte 0x");
          Serial.print ((val & 0xFF00) ,HEX);
          Serial.print(" val second byte 0x");
          Serial.println ((val & 0x00FF) ,HEX);
       
       
        Serial.print(" high ");
        Serial.print("0x");
        Serial.print(highByte(val) < 16 ? "0" : "");
        Serial.print(highByte(val), HEX);
        
        Serial.print(" low ");
        Serial.print("0x");
        Serial.print(lowByte(val) < 16 ? "0" : "");
        Serial.print(lowByte(val), HEX);
        Serial.print(" ");

        Serial.println(val);
        */
       Serial.print(i);
        Serial.print(" ");
        Serial.println(val);
          #endif
          
        
        
        bufferData[indexByte]=highByte(val);
        indexByte++;
        bufferData[indexByte]=lowByte(val);
        indexByte++;
    }

    
    for(int i=0;i<SIZE_MAC;i++){
      bufferData[NB_CHIP+nbAnaInput*2+i]=mac[MAC_INDEX][i];
    }

    unsigned long timestamp=millis();
    for(int i=0;i<SIZE_TIMESTAMP;i++){
        #ifdef DEBUG
        Serial.print("time stamp ");
        Serial.print(timestamp);
        Serial.print("  ");
        Serial.print((8*(3-i)));
        Serial.print("  ");
        Serial.println ((timestamp >> (8*(3-i))) & 0x00FF,HEX);
        #endif
        bufferData[NB_CHIP+nbAnaInput*2+ SIZE_MAC+ i]=(timestamp >> (8*(3-i)) & 0x00FF  );
    }
    
    
    for(int i=0;i<SIZEOFNAME;i++){
      bufferData[NB_CHIP+nbAnaInput*2+ SIZE_MAC + SIZE_TIMESTAMP + i]=byteName[i];
    }
  
     #ifdef DEBUG
    for(int i=0; i<sizeof(bufferData); i++){
      
        Serial.print("0x");
        Serial.print(bufferData[i] < 16 ? "0" : "");
        Serial.print(bufferData[i], HEX);
        Serial.print(" ");
    }
    #endif

    client.write(bufferData,sizeof(bufferData));
    Serial.print("end millis ");
    Serial.println(millis());
 
}




/**
 *
 *
 *    setOutput
 *
 *
 *  changer l'etat d'un port des MCP23017
 *
 *
 **/
void setOutput(int channel, bool state)
{
  if (channel >= 0 && channel <= 15)
  {
    digOutput1.digitalWrite(channel, state);
    /*
    Serial.print("dig1 ");
    Serial.print(channel);
    Serial.print(" state ");
    Serial.println(state);
    */
  }
  if (channel >= 16 && channel <= 31)
  {
    int tempChannel = channel - 16;
    digOutput2.digitalWrite(tempChannel, state);
    /*
    Serial.print("dig2 ");
   Serial.print(channel);
    Serial.print(" state ");
    Serial.println(state);
    */
  }
 
}



/**
 * 
 * 
 *    fonction callback pour decoder le message protobuf
 * 
 * 
 * 
 * 
*/
void decodeListOutput(pb_istream_t *stream, const pb_field_t *field, void **arg){

  //Serial.println("call decodelistoutput");
   DigitalOutput digout = DigitalOutput_init_zero;
   if (!pb_decode(stream,DigitalOutput_fields,&digout)){
      Serial.print("error decoding: ");
      Serial.println(stream->errmsg);
   }
   else{
       #ifdef DEBUG
      Serial.println("decodelist ok");
      #endif
   }

   #ifdef DEBUG
   Serial.print("numChannel ");
   Serial.print(digout.numChannel);
   Serial.print(" value ");
   Serial.println(digout.value);
   #endif
  setOutput(digout.numChannel,digout.value);

}





/**
 *
 *
 *    initDigOutput
 *
 *
 *  initialiser les ports des composants MCP23017 en sortie
 *
 *
 **/
void initDigOutput()
{
  for (int i = 0; i < 16; i++)
  {
    digOutput1.pinMode(i, OUTPUT);
    digOutput2.pinMode(i, OUTPUT);
  }

  /*
  Serial.println("set HIGH");

  for (int i = 0; i < 32; i++)
  {
      setOutput(i,HIGH);
      
  }

  delay(20000);
  for (int i = 0; i < 32; i++)
  {
      setOutput(i,LOW);
  }
  Serial.println("set LOW");
   delay(4000);
   */

}



/**
 *
 *          analogReadInput
 *    lecture des analogiques
 *
 **/

void analogReadInput()
{
#ifdef DEBUG

  Serial.println("analogRead");

#endif

#ifdef DEBUG_EXECUTION_TIME

  Serial.print("\tRead analog Input start at ");
  Serial.println(millis());

#endif
int index=0;
  for (int i = 0; i < nbAnaInput; i++)
  {
    int val=analogRead(anaPin[i]);
    message.anaInput[index] = val; //decompose la lecture des ana en 2 byte
    index++;
     message.anaInput[index] = (val>>8);
     index++;
  
#ifdef DEBUG

    Serial.print(val);
    Serial.print(" ");

#endif
  }
#ifdef DEBUG_EXECUTION_TIME

  Serial.print("\tRead analog Input end at ");
  Serial.println(millis());
  Serial.println();
#endif
#ifdef DEBUG

  Serial.println();

#endif
}



/**
 *
 *          initInput
 *
 *
 **/
void initInput()
{
  // DDRL = 0;
  for (int i = 0; i < NB_INPUT; i++)
  {
    pinMode(dataPin[i], INPUT_PULLUP);
  }
}

/**
 *
 *          digitalReadInput pour debug
 *
 *
 **/
void digitalReadInput(byte *table)
{
#ifdef DEBUG_EXECUTION_TIME

  Serial.print("\tRead digital Input start at ");
  Serial.println(millis());
#endif
  resetchipselect();
  //delayMicroseconds(2);
  for (int i = 0; i < NB_CHIP; i++)
  {
    if (i > 0)
    {
      digitalWrite(chipsSelect[i - 1], HIGH);
    }
    digitalWrite(chipsSelect[i], LOW);
    
   delayMicroseconds(5);
    
    uint8_t val=45;
    /*
    bool localstate=false;

    Serial.print("\n"+String(i)+"   ");

    for(int j=0;j<8;j++){
      localstate=digitalRead(dataPin[j]);
      Serial.print(localstate);
      bitWrite(val,j,localstate);
      
    }
     Serial.println();
     */

    val=readPort(); //pour inverser ajouter ~ devant
    table[i]=val;
    //delayMicroseconds(5);
#ifdef DEBUG

   Serial.print("val ");
   Serial.print(val);
   Serial.print(" ");
    Serial.println(val, BIN);
    
#endif
  }
#ifdef DEBUG
  Serial.println();
  // delay(1000);
#endif

 // digitalWrite(chipsSelect[NB_CHIP - 1], HIGH);

#ifdef DEBUG_EXECUTION_TIME

  Serial.print("\tRead digital Input end at ");
  Serial.println(millis());
  Serial.println();
#endif
}



/**
 *
 *          initMsg
 *
 *
 **/
void initMsg()
{
  int i = 0;

  for (i = 0; i < NB_CHIP; i++)
  {
    message.digInput[i] = 0;
  }

  for (i = 0; i < nbAnaInput*2; i++)
  {
    message.anaInput[i] = -1;
  }
}


/*
*
*
*   Lire les entrees num avec debounce
*   
*   3 lectures consecutives et vote
*
*/
void readDebounceInput(){
  
        #ifdef DEBUG_EXECUTION_TIME

          Serial.print("\tRead digital Input start at ");
          Serial.println(millis());
        #endif

        
        
        for(int indexLecture=0;indexLecture<NB_LEC_DEBOUNCE;indexLecture++){

              digitalReadInput(tempLec[indexLecture]);
              for(int i=0;i<NB_CHIP;i++){
                #ifdef DEBUG
                    Serial.print(" ");
                    Serial.print(tempLec[indexLecture][i], BIN);
                #endif
                
              }
              #ifdef DEBUG
                    Serial.println();
                    // delay(1000);
                #endif
             
        }
          

        #ifdef DEBUG_EXECUTION_TIME

          Serial.print("\tRead digital Input end at ");
          Serial.println(millis());
          Serial.println();
        #endif


    //reading=true;
  for(int i=0;i<NB_CHIP;i++){

        //tempLec[0][i]; //1010
        //tempLec[1][i]; //1100
        //tempLec[2][i]; //1001
        
        int nb=0;
        
        byte res=0b00000000;

        for(int j=0;j<8;j++){
            nb=0;
           
            bool st1=bitRead(tempLec[0][i],j);
            if(st1){nb++;}else{nb--;}

            bool st2=bitRead(tempLec[1][i],j);
            if(st2){nb++;}else{nb--;}

            bool st3=bitRead(tempLec[2][i],j);
            if(st3){nb++;}else{nb--;}


            #ifdef DEBUG
            /*
            Serial.print("nb:");
            Serial.print(nb);
            
            Serial.println();
            */
            #endif
            
            if(nb>0){
              bitWrite(res,j,true);
            }else{
              bitWrite(res,j,false);
            }
           
        }
        

        #ifdef DEBUG
          Serial.print(res);
          Serial.print(" ");
        #endif
      
        message.digInput[i]=res; //sauvegarde la valeur apres vote



  }

    #ifdef DEBUG
          Serial.println();
        #endif

 
}



/**
 *
 *          setup
 *
 **/
void setup()
{

  Serial.begin(115200);
  //Serial1.begin(115200);
    Wire.begin();

  pinMode(18,OUTPUT);
  digitalWrite(18, LOW);
  delay(1);
  digitalWrite(18, HIGH);

  pinMode(USE_THIS_SS_PIN, OUTPUT);
  digitalWrite(USE_THIS_SS_PIN, HIGH);

  Serial.println("Demarrage");

  initDigOutput();

  SerialDebug.print("\nStarting WebServer on ");
  SerialDebug.print(BOARD_NAME);
  SerialDebug.print(F(" with "));
  SerialDebug.println(SHIELD_TYPE);
  SerialDebug.println(ETHERNET_GENERIC_VERSION);

  Ethernet.init (USE_THIS_SS_PIN);

  delay(1);
  

  IPAddress dnServer(192, 168, 0, 1);
// the router's gateway address:
IPAddress gateway(192, 168, 0, 1);
// the subnet:
IPAddress subnet(255, 255, 255, 0);

//the IP address is dependent on your network
IPAddress ip(192, 168, 0, 2);


  Ethernet.begin(mac[MAC_INDEX],ip,dns,gateway,subnet);
  
  
  
  SerialDebug.print(F("Connected! IP address: "));
  SerialDebug.println(Ethernet.localIP());

  if ( (Ethernet.getChip() == w5500) || (Ethernet.getChip() == w6100) || (Ethernet.getAltChip() == w5100s) )
  {
    if (Ethernet.getChip() == w6100)
      SerialDebug.print(F("W6100 => "));
    else if (Ethernet.getChip() == w5500)
      SerialDebug.print(F("W6100 => "));
    else
      SerialDebug.print(F("W5100S => "));
    
    SerialDebug.print(F("Speed: "));
    SerialDebug.print(Ethernet.speedReport());
    SerialDebug.print(F(", Duplex: "));
    SerialDebug.print(Ethernet.duplexReport());
    SerialDebug.print(F(", Link status: "));
    SerialDebug.println(Ethernet.linkReport());
  }

  // start the web server on port 80
  serveur.begin();
  Serial.print("Pret !");

    //initialisation des I/O
  initchipselect();
  initInput();
  initMsg();
  
}





/**
 *
 *          loop
 *
 * 
 *      lecture en continue des I/O
 * 
 **/

void loop()
{
  uint8_t buffer[256]; // Ajustez la taille du tampon en fonction de la taille maximale du message protobuf
  int bytesRead;
  

  EthernetClient client = serveur.available();
  //client.setTimeout(10);
  if (client) {
    // Quelqu'un est connecté !
    //Serial.print("On envoi !");
    bytesRead=0;
    
    /*
    
        Quand on recoit un message

    */
    while(client.connected()){
        #ifdef DEBUG_EXECUTION_TIME
          startTime=millis();
        #endif

        /*
        int sizeR=client.available();
        if(sizeR)
        { 
  
            bytesRead = client.read(buffer, sizeof(buffer));
            if(bytesRead>0)
            {
              
              // Désérialiser le message protobuf à partir du tampon
              pb_istream_t istream = pb_istream_from_buffer(buffer, bytesRead);
              EpicEthernetOutput epicEthernetOutput = EpicEthernetOutput_init_default; // Structure du message EpicEthernetOutput
              epicEthernetOutput.digoutputs.funcs.decode=decodeListOutput;
              // Utilisez pb_decode pour désérialiser un message 
              bool success = pb_decode(&istream, EpicEthernetOutput_fields, &epicEthernetOutput);  //on decode le message recu

              if (success) {
                
                Serial.print("success ");
                Serial.println(epicEthernetOutput.nbChannel);

              }else{
                Serial.print("error ");
                Serial.println(istream.errmsg);
              }
              
            }
            
        }else{
          
          if( millis() > lastTime+16){
            readAndSendInput(client);
            lastTime=millis();
            //Serial.println(millis());

          }
          

         //Serial.println("do something else");
          //delay(1000);



        }
        */

       if( millis() > lastTime+vitesseTrame){
            //readAndSendInputPB(client);
            //readAndSendInputOptimize(client);
            readAndSendInputJson(client);
            lastTime=millis();
            //Serial.println(millis());

          }


         #ifdef DEBUG_EXECUTION_TIME
            unsigned long deltaTime=millis()-startTime;
          Serial.print("execution total time : ");
          Serial.println(deltaTime);
            //delay(1000);
          #endif
    }

    
  
   
  }

  
}
