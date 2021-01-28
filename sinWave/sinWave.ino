#define PI 3.1415926535897932384626433832795


const int thisPin = 2;
const int highestPin = 2;

float analogOut = 0;


void setup() {
    pinMode(thisPin, OUTPUT);
    pinMode(47, OUTPUT);
    Serial.begin(9600);
}


float rad = 0;
int schritte = 10;
void loop() {
    // if(rad >= 2*PI){
    //     rad = 0;
    // }
    // analogOut = (sin(rad) + 1)/2*3.3;
    
    // analogWrite(thisPin, analogOut);

    // rad = rad + PI / schritte;
    // Serial.println(((float)analogRead(7)));
    // // Serial.print(" ");
    // //delay(1000);

    digitalWrite(47,HIGH);
    delay(1000);
    digitalWrite(47,LOW);
    delay(1000);
}