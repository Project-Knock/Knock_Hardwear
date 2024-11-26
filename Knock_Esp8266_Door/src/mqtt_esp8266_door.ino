#include <ESP8266WiFi.h>
#include <Servo.h>
#include <PubSubClient.h>
#include <string.h>

#define PWPIN 5
const char *ssid = "bssm_free";
const char *password = "bssm_free";
const char* mqttServer = "10.150.151.42"; // 라즈베리 파이의 IP 주소
const int mqttPort = 1883; // MQTT 포트
const char* topic = "room/302/control/door";
char message_buff[100];

Servo door;

WiFiClient wifiClient;
void moveServo(Servo sv){
  sv.write(0);
  delay(500);
  sv.write(60);
  delay(500);
}
void callback(char* topic, byte* payload, unsigned int length) {
  int i = 0;

  Serial.println("Message arrived: topic: " + String(topic));
  Serial.println("Length: "+ String(length,DEC));

  //create character buffer with ending null terminator (string)
  for(i=0; i<length; i++){
    message_buff[i] = payload[i];
  }
  message_buff[i]= '\0';
  String topicString = String(topic);
  String msgString = String(message_buff);
  Serial.println("Payload: "+ msgString);
  if (msgString == "open"){
      moveServo(door);
  }
}
PubSubClient mqttClient(mqttServer, mqttPort, callback, wifiClient);
void setup() {

  Serial.begin(115200);
  door.attach(PWPIN);
  door.write(60);
  WiFi.setOutputPower(19.25);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  //연결되면 접속한 IP가 뭔지 출력한다.
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());
  mqttClient.setServer(mqttServer, mqttPort);
  if(mqttClient.connect("302_door")){
    mqttClient.subscribe(topic);
  }
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  delay(2000); // 2초마다 데이터 수신
}

void reconnect() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect("302_door")) {
      Serial.println("Connected to MQTT Broker");
    } else {
      delay(5000);
    }
  }
}
