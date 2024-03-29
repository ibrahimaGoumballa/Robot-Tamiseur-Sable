//BIBLIO
#include <MKRWAN.h>
#include <HX711.h> //Poids
#include <Arduino_MKRGPS.h>
#include <Servo.h>

// Pins pour capteur de poids
#define LOADCELL_DOUT_PIN  1
#define LOADCELL_SCK_PIN  0

LoRaModem modem;

//Liaison avec The Things Network
String appEui = "0000000000000000";
String appKey = "EBC127F8D66B753D4E6112CF5F26FE1D";
String devAddr;
String nwkSKey;
String appSKey;

// POIDS
HX711 scale;
float calibration_factor = 21900; // for 16,6 -> 21067 and for ~93,5 -> 21387
float meas, p;

float real_meas;
float pourcentage = 15.38;        //la charge de la batterie en pourcentage
float latitude =   55.8450559;
float longitude  = 7.3556285;

// Capteur de niveau
int liquidLevel = 1;

// variables pour stocker les valeurs mesurées en 16bits
short var_battery;
short var_poids;
short var_liquide;
int var_latitude;
int var_longitude;

//CONSTANTES
const byte TRIGGER_PIN1 = A2;
const byte ECHO_PIN1 = A3;
const byte TRIGGER_PIN_2 = A1;
const byte ECHO_PIN_2 = A4;
const unsigned long MEASURE_TIMEOUT = 25000UL;
const float SOUND_SPEED = 340.0 / 1000;

//VARIABLES DRIVER ET SERVO
Servo myservo1;
Servo myservo2;
int ena = 6;
int in1 = 2;
int in2 = 3;
int in3 = 4;
int in4 = 5;
int enb = 7;
int vitesse;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(TRIGGER_PIN1, OUTPUT);
  digitalWrite(TRIGGER_PIN1, LOW);
  pinMode(ECHO_PIN1, INPUT);
  
  pinMode(TRIGGER_PIN_2, OUTPUT);
  digitalWrite(TRIGGER_PIN_2, LOW);
  pinMode(ECHO_PIN_2, INPUT);
    
  myservo1.attach(A5);
  myservo2.attach(A6);
  pinMode(ena, OUTPUT);  
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enb, OUTPUT);
  vitesse = 800;
  delay(1000);
/*
  if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    while (1) {}
  };
  if (modem.version() != ARDUINO_FW_VERSION) {
    Serial.println("Please make sure that the latest modem firmware is installed.");
    Serial.println("To update the firmware upload the 'MKRWANFWUpdate_standalone.ino' sketch.");
  }

  int mode = 1;
  int connected;
  if (mode == 1) {
    appKey.trim();
    appEui.trim();
    connected = modem.joinOTAA(appEui, appKey);
  }

  if (!connected) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }
*/
  delay(1000);

  // Batterie 
  Serial.print("Pourcentage batterie : ");  // debug value
  Serial.println(pourcentage);
  var_battery = (short)(pourcentage*100);
  Serial.print("Valeur batterie envoyer  : ");
  Serial.println(var_battery);
/*
   Serial.println("HX711 calibration sketch");
   Serial.println("Remove all weight from scale");
   Serial.println("After readings begin, place known weight on scale");
   Serial.println("Press + or a to increase calibration factor");
   Serial.println("Press - or z to decrease calibration factor");
  
   scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
   scale.set_scale();
   scale.tare(); //Reset the scale to 0
  
   long zero_factor = scale.read_average(); //Get a baseline reading
   Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
   Serial.println(zero_factor);  
  // // Pin capteur de niveau
   pinMode(A0, INPUT);
*/
  Serial.print("Poids mesurer : ");  // debug value
  Serial.println(real_meas);
  var_poids = (short)(real_meas*100);
  Serial.print("Valeur poids  envoyer : ");
  Serial.println(var_poids);

  Serial.print("Location: ");
  Serial.print(latitude, 7);
  Serial.print(", ");
  Serial.println(longitude, 7);
  
  var_latitude = latitude * 10000000.0;
  Serial.print("latitude envoyer : ");
  Serial.println(var_latitude);

  var_longitude = longitude * 10000000.0;
  Serial.print("longitude envoyer : ");
  Serial.println(var_longitude);  

  var_liquide = (short) liquidLevel;
  Serial.print("niveau liquide envoyer : ");
  Serial.println(var_liquide);
}

void loop() {
  float distance1 = mesurerDistance1();
  float distance2 = mesurerDistance2();
  if(distance1 !=0 || distance2 !=0){  
    if (distance1 < 500 || distance2 < 500) {
      Serial.println("OBSTACLE");
      trouverMonChemin();
    } else {
      avancer();
    }
  }
}


// Permet de mesurer la distance de l obstacle 
float mesurerDistance1() {
  digitalWrite(TRIGGER_PIN1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN1, LOW);

  long measure = pulseIn(ECHO_PIN1, HIGH, MEASURE_TIMEOUT);

  float distance_mm = measure / 2.0 * SOUND_SPEED;
  Serial.println(distance_mm);
  return distance_mm;
}
float mesurerDistance2() {
  digitalWrite(TRIGGER_PIN_2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN_2, LOW);

  long measure = pulseIn(ECHO_PIN_2, HIGH, MEASURE_TIMEOUT);

  float distance_mm = measure / 2.0 * SOUND_SPEED;
  Serial.println(distance_mm);
  return distance_mm;
}

void arret(){
  digitalWrite(in1, 0);
  digitalWrite(in2, 0);
  digitalWrite(in3, 0);
  digitalWrite(in4, 0);  
  analogWrite(ena, 0);
  analogWrite(enb, 0);  
}

void reculer(){
  digitalWrite(in1, 1);
  digitalWrite(in2, 0);
  digitalWrite(in3, 0);
  digitalWrite(in4, 1);  
  analogWrite(ena, vitesse);
  analogWrite(enb, vitesse);
}

void tourneradroite(){
  digitalWrite(in1, 0);
  digitalWrite(in2, 0);
  digitalWrite(in3, 1);
  digitalWrite(in4, 0);  
  analogWrite(ena, 0);
  analogWrite(enb, vitesse);
}

void tourneragauche(){
  digitalWrite(in1, 0);
  digitalWrite(in2, 1);
  digitalWrite(in3, 0);
  digitalWrite(in4, 0);
  analogWrite(ena, vitesse);
  analogWrite(enb, 0);    
}

void avancer(){
  digitalWrite(in1, 0);
  digitalWrite(in2, 1);
  digitalWrite(in3, 1);
  digitalWrite(in4, 0); 
  analogWrite(ena, vitesse);
  analogWrite(enb, vitesse); 
}

//Tourner l ultrason a gauche  
void regarderagauche() {
  myservo2.write(100); // Angle de rotation pour regarder à gauche (à ajuster)
  delay(500);
}
//Tourner l ultrason a doite  
void regarderadroite() {
  myservo1.write(0); // Angle de rotation pour regarder à droite (à ajuster)
  delay(500);
}
//Tourner l ultrason devant 
void regarderdevant() {
  myservo1.write(120); // Angle de rotation pour regarder droit devant (à ajuster)
  myservo2.write(0); // Angle de rotation pour regarder droit devant (à ajuster)
}
//Fonction permmettant de voir si la voie est libre a Gauche
int VoieLibreAGauche() {
  int distanceAGauche;
  regarderagauche(); // Regarde à gauche
  distanceAGauche = mesurerDistance1(); // Mesure la distance à gauche
  return distanceAGauche;
}
//Fonction permmettant de voir si la voie est libre a Droite
int VoieLibreADroite() {
  int distanceADroite;
  regarderadroite(); // Regarde à droite
  distanceADroite = mesurerDistance2(); // Mesure la distance à droite
  return distanceADroite;
}

void trouverMonChemin() {
  reculer();
  delay(50);
  arret();
  delay(500);
  int distanceAGauche = VoieLibreAGauche();
  Serial.print("DISTANCE A GAUCHE ");
  Serial.println(distanceAGauche);
  int distanceADroite = VoieLibreADroite();
  Serial.print("DISTANCE A DROITE ");
  Serial.println(distanceADroite);
  regarderdevant();
  delay(500);
  avancer();
  if (distanceAGauche < distanceADroite) {
    tourneragauche();
    delay(1000);
  } 
  else {
    tourneradroite();
    delay(1000);
  }

}



