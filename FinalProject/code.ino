//Import temperature sensor library. Connect pin to 10.
#include <dht.h>
dht DHT;
#define DHT11_PIN 10

//Import servo motor library. Connect pin to 11.
#include <Servo.h>
int servoPin = 11;
Servo servo;
int servoAngle = 0;   // servo position in degrees

//Connect ultrasonic Sensor trigger pin to 13 & echo pin to 12.
#define trigPin 13
#define echoPin 12
int distance; //to measure the distance

//Define led pin.
int red_light_pin = 4;
int green_light_pin = 3;
int blue_light_pin = 2;

//Import mp3 module library
#include <SoftwareSerial.h>
#define ARDUINO_RX 6//should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 5//connect to RX of the module
SoftwareSerial mySerial(ARDUINO_RX, ARDUINO_TX);//init the serial protocol, tell to myserial wich pins are TX and RX

////////////////////////////////////////////////////////////////////////////////////
//all the commands needed in the datasheet(http://geekmatic.in.ua/pdf/Catalex_MP3_board.pdf)
static int8_t Send_buf[8] = {0} ;//The MP3 player undestands orders in a 8 int string
#define NEXT_SONG 0X01
#define PREV_SONG 0X02
#define CMD_PLAY_W_INDEX 0X03 //DATA IS REQUIRED (number of song)
#define VOLUME_UP_ONE 0X04
#define VOLUME_DOWN_ONE 0X05
#define CMD_SET_VOLUME 0X06//DATA IS REQUIRED (number of volume from 0 up to 30(0x1E))
#define SET_DAC 0X17
#define CMD_PLAY_WITHVOLUME 0X22 //data is needed  0x7E 06 22 00 xx yy EF;(xx volume)(yy number of song)
#define CMD_SEL_DEV 0X09 //SELECT STORAGE DEVICE, DATA IS REQUIRED
#define DEV_TF 0X02 //HELLO,IM THE DATA REQUIRED
#define SLEEP_MODE_START 0X0A
#define SLEEP_MODE_WAKEUP 0X0B
#define CMD_RESET 0X0C//CHIP RESET
#define CMD_PLAY 0X0D //RESUME PLAYBACK
#define CMD_PAUSE 0X0E //PLAYBACK IS PAUSED
#define CMD_PLAY_WITHFOLDER 0X0F//DATA IS NEEDED, 0x7E 06 0F 00 01 02 EF;(play the song with the directory \01\002xxxxxx.mp3
#define STOP_PLAY 0X16
#define PLAY_FOLDER 0X17// data is needed 0x7E 06 17 00 01 XX EF;(play the 01 folder)(value xx we dont care)
#define SET_CYCLEPLAY 0X19//data is needed 00 start; 01 close
#define SET_DAC 0X17//data is needed 00 start DAC OUTPUT;01 DAC no output
////////////////////////////////////////////////////////////////////////////////////


void setup() {
  Serial.begin (9600);

  //Define distance sensor output and input (devices)
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //Define rgb
  pinMode(red_light_pin, OUTPUT);
  pinMode(green_light_pin, OUTPUT);
  pinMode(blue_light_pin, OUTPUT);

  //Servo motor
  servo.attach(servoPin);

  //define mp3 player
  mySerial.begin(9600);//Start our Serial coms for THE MP3
  sendCommand(CMD_SEL_DEV, DEV_TF);//select the TF card

}

void loop() {
  //Initial ultrasonic sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  distance = pulseIn(echoPin, HIGH);

  // Read temperature as Celsius
  float temperature = DHT.temperature;
  int chk = DHT.read11(DHT11_PIN);

  // when no human around
  if ( distance > 2000 && temperature <= 35) {
    RGB_color(0, 255, 0); //LED Green
    servo.write(0); //Motor open
  }

  //when people without fever around
  if ( distance >= 0 && distance <= 2000 && temperature >= 35 && temperature <= 37.5) {
    RGB_color(255, 255, 0); //LED Yellow
    servo.write(90);  // Motor Close
    sendCommand(CMD_PLAY_W_INDEX, 0x01);//play message no.1
  }

  //when people who has fever around us
  if ( distance >= 0 && distance <= 2000 && temperature >= 37.5 && temperature <= 45) {
    RGB_color(255, 0, 0); //LED Red
    servo.write(90); //Motor Close
    sendCommand(CMD_PLAY_W_INDEX, 0x02);//play message no.2
  }

  Serial.print(" Temperature: ");
  Serial.print(temperature);
  Serial.print(F("°C "));          //prints the temperature
  Serial.print("distance= ");
  Serial.println(distance);        //prints the distance
}


//RGB control
void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
{
  analogWrite(red_light_pin, red_light_value);
  analogWrite(green_light_pin, green_light_value);
  analogWrite(blue_light_pin, blue_light_value);
}


//MP3 control
void sendCommand(int8_t command, int16_t dat)
{
  delay(20);
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0xff; //version
  Send_buf[2] = 0x06; //the number of bytes of the command without starting byte and ending byte
  Send_buf[3] = command; //
  Send_buf[4] = 0x00;//0x00 = no feedback, 0x01 = feedback
  Send_buf[5] = (int8_t)(dat >> 8);//datah
  Send_buf[6] = (int8_t)(dat); //datal
  Send_buf[7] = 0xef; //ending byte
  for (uint8_t i = 0; i < 8; i++) //
  {
    mySerial.write(Send_buf[i]) ;//send bit to serial mp3
  }
}
