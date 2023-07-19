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
//#include "pio_without_options.pb.h"

#define NB_INPUT 8
#define NB_CHIP 24
#define nbDigInput NB_CHIP *NB_INPUT
#define nbAnaInput 16


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


EthernetServer serveur(4200);
//EthernetClient client();


uint8_t *readbuf = (uint8_t *)malloc(100);
int indexreadbuf = 0;

void decodeListOutput(pb_istream_t *stream, const pb_field_t *field, void **arg){

  Serial.println("call decodelistoutput");
   DigitalOutput digout = DigitalOutput_init_zero;
   if (pb_decode(stream,DigitalOutput_fields,&digout)){
      Serial.print("error decoding: ");
      Serial.println(stream->errmsg);
   }
   else{
      Serial.println("decodelist ok");
   }
   Serial.print("id ");
   Serial.print(digout.id);
   Serial.print(" value ");
   Serial.println(digout.value);
  digitalWrite(digout.id,digout.value);

}

void decodeMessage2()
{
   
    pb_istream_s pb_in = pb_istream_from_buffer(*readbuf,indexreadbuf);
    //pb_ostream_s pb_out =as_pb_ostream(client);
    Serial.print("state ");
    Serial.println(pb_in.errmsg);


   
    EpicEthernetOutput frameReceived= EpicEthernetOutput_init_zero;
   

    //reception message pour piloter les sorties
     bool status = pb_decode(&pb_in, EpicEthernetOutput_fields, &frameReceived);
    if(status != true)
    {
      Serial.print("error decoding: ");
      Serial.println(pb_in.errmsg);
      
    }else{
      Serial.println("decode ok ");
    
    }
    frameReceived.digoutputs.funcs.decode=&decodeListOutput;
}

void decodeMessage(EthernetClient client)
{
    int32_t buffer[40];
    size_t message_length;



    pb_istream_s pb_in =as_pb_istream(client);
    //pb_ostream_s pb_out =as_pb_ostream(client);

    EpicEthernetOutput frameReceived= EpicEthernetOutput_init_zero;
   

    //reception message pour piloter les sorties
     bool status = pb_decode(&pb_in, EpicEthernetOutput_fields, &frameReceived);
    if(status != true)
    {
      Serial.print("error decoding: ");
      Serial.println(pb_in.errmsg);
      
    }
    frameReceived.digoutputs.funcs.decode=&decodeListOutput;
    

  
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
 *
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
*   Lecture du port L en entier
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

  SerialDebug.print("\nStarting WebServer on ");
  SerialDebug.print(BOARD_NAME);
  SerialDebug.print(F(" with "));
  SerialDebug.println(SHIELD_TYPE);
  SerialDebug.println(ETHERNET_GENERIC_VERSION);

  Ethernet.init (USE_THIS_SS_PIN);

  delay(1);
  Ethernet.begin(mac[0]);
  
  
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
  
  EthernetClient client = serveur.available();
  if (client) {
    // Quelqu'un est connecté !
    Serial.print("On envoi !");
    
    while(client.connected()){
        if(client.available())
        {
          uint8_t carlu = client.read(); //on lit ce qu'il raconte
          readbuf[indexreadbuf] = carlu;
          indexreadbuf++;
          /*
          if(carlu != '\n') { // On est en fin de chaîne ?
            // non ! alors on stocke le caractère
            readbuf[indexreadbuf] = carlu;
            
          } else {
            // on a fini de lire ce qui nous intéresse
            // on marque la fin de l'url (caractère de fin de chaîne)
            
            //readbuf[indexreadbuf] = '\0';
            //readbuf[indexreadbuf] = '';
            // + TRAITEMENT
            // on quitte le while
            break;
          }*/
        }
    }
  Serial.print("message recu ");
  Serial.println(indexreadbuf);
  //Serial.println(*readbuf);
    decodeMessage2();
    
    //decodeMessage(client);
    /*
    // On fait notre en-tête
    // Tout d'abord le code de réponse 200 = réussite
    client.println("HTTP/1.1 200 OK");
    // Puis le type mime du contenu renvoyé, du json
    client.println("Content-Type: application/json");
    // Et c'est tout !
    // On envoie une ligne vide pour signaler la fin du header
    client.println();

    // Puis on commence notre JSON par une accolade ouvrante
    client.println("{");
    // On envoie la première clé : "uptime"
    client.print("\t\"uptime (ms)\": ");
    // Puis la valeur de l'uptime
    client.print(millis());
    //Une petite virgule pour séparer les deux clés
    client.println(",");
    // Et on envoie la seconde nommée "analog 0"
    client.print("\t\"analog 0\": ");
    client.println(12);
    // Et enfin on termine notre JSON par une accolade fermante
    client.println("}");
    // Donne le temps au client de prendre les données
    delay(10);
    // Ferme la connexion avec le client
    */
    client.stop();
  }


  #ifdef DEBUG_EXECUTION_TIME
  startTime=millis();
  #endif

   analogReadInput();
  //digitalReadInput(message.digInput);
  readDebounceInput();


   #ifdef DEBUG
    //delay(1000);
  #endif
  #ifdef DEBUG_EXECUTION_TIME
    unsigned long deltaTime=millis()-startTime;
  Serial.print("execution total time : ");
  Serial.println(deltaTime);
    //delay(1000);
  #endif
  
}
