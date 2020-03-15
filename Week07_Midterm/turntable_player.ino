//I found this code from(https://create.arduino.cc/projecthub/javier-munoz-saez/arduino-mp3-player-catalex-2effef)

#include <SoftwareSerial.h>
#define ARDUINO_RX 5//should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 6//connect to RX of the module
SoftwareSerial mySerial(ARDUINO_RX, ARDUINO_TX);//init the serial protocol, tell to myserial wich pins are TX and RX

////////////////////////////////////////////////////////////////////////////////////
//all the commands needed in the datasheet(http://geekmatic.in.ua/pdf/Catalex_MP3_board.pdf)
static int8_t Send_buf[8] = {0} ;//The MP3 player undestands orders in a 8 int string
//0X7E FF 06 command 00 00 00 EF;(if command =01 next song order)
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





int track1threshold = 256;
int track2threshold = 512;
int track3threshold = 768;
int track4threshold = 1024;
boolean playing1 = false;
boolean playing2 = false;
boolean playing3 = false;
int volume = 0;

void setup() {
  Serial.begin(9600);//Start our Serial coms for serial monitor in our pc
  pinMode(9, OUTPUT);
  mySerial.begin(9600);//Start our Serial coms for THE MP3
  delay(500);//Wait chip initialization is complete
  sendCommand(CMD_SEL_DEV, DEV_TF);//select the TF card
  delay(200);//wait for 200ms

}

void loop()
{
  // read the mp3 control sensor input on analog pin 0:
  int controlsensorValue = analogRead(A0);
  int volumesenesorValue = analogRead(A1);

  Serial.print("threshold is ");
  Serial.println(controlsensorValue);
  delay(1);

  // delay in between reads for stability

  volume = map(volumesenesorValue, 0, 1024, 1, 30);
  sendCommand(CMD_SET_VOLUME, volume);

  Serial.print("Volume is ");
  Serial.print(volume);
  Serial.print(" Sensor is ");
  Serial.print(volumesenesorValue);
  Serial.println();
  delay(1);

///////////////////////////////////////////////////////////////////////////////////////////////////
  //if control sensor value is smaller than play threshhold
  if (controlsensorValue <= track1threshold) {
    digitalWrite(9, LOW);//turn off motor
    sendCommand(CMD_PAUSE, 0X0F02);//stop playing the music
    playing1 = false;
    playing2 = false;
    playing3 = false;
  }

  if (controlsensorValue >= track1threshold && controlsensorValue <= track2threshold && !playing1) {
    digitalWrite(9, HIGH);//turn on motor
    sendCommand(CMD_PLAY_W_INDEX, 0x01);//play song no.1
    playing1 = true;
    playing2 = false;
    playing3 = false;
  }

  if (controlsensorValue >= track2threshold && controlsensorValue <= track3threshold && !playing2) {
    sendCommand(CMD_PLAY_W_INDEX, 0x02);//play song no.2
    playing2 = true;
    playing1 = false;
    playing3 = false;
  }

  if (controlsensorValue >= track3threshold && controlsensorValue <= track4threshold && !playing3) {
    sendCommand(CMD_PLAY_W_INDEX, 0x03);//play song no.3
    playing3 = true;
    playing1 = false;
    playing2 = false;
  }

///////////////////////////////////////////////////////////////////////////////////////////////////
  //keyboard testing
  char c = Serial.read();
  if (c == 'p') {
    sendCommand(CMD_PLAY_WITHVOLUME, 0X0F01);//play the first song with volume 15 class
  }

  // if we get an 's' on the serial console, stop!
  if (c == 's') {
    sendCommand(CMD_PAUSE, 0X0F02);
  }
  if (c == 'r') {
    sendCommand(CMD_PLAY, 0X0F02);
  }
  if (c == 'u') {
    sendCommand(CMD_SET_VOLUME, 30);
  }
  if (c == 'd') {
    sendCommand(CMD_SET_VOLUME, 2);
  }
  if (c == 'n') {
    sendCommand(CMD_PLAY_W_INDEX, 0x02);//play song no.2
  }

  delay(1000); //the programm will send the play option each 100 seconds to the catalex chip
}
///////////////////////////////////////////////////////////////////////////////////////////////////

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
    //Serial.print(Send_buf[3], HEX); //send bit to serial monitor in pc
  }
  //Serial.println();
  //  Serial.print("CMD is ");
  //  Serial.print(Send_buf[3]);
  //  Serial.println();

  //control
  //  if (Serial.available()) {
  //    char c = Serial.read();
  //
  //    // if we get an 's' on the serial console, stop!
  //    if (c == 's') {
  //      Send_buf[3] = CMD_PAUSE;
  //    }
  //  }
}
