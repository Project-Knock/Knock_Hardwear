#include <ESP8266WiFi.h>
#include <Servo.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <string.h>

#define DHTPIN D4 // DHT 센서 핀
#define DHTTYPE DHT11 // DHT 타입 (DHT11 또는 DHT22)
#define PWPIN 16
#define TEMPPIN 5
const char *ssid = "bssm_free";
const char *password = "bssm_free";
const char* mqttServer = "10.150.151.42"; // 라즈베리 파이의 IP 주소
const int mqttPort = 1883; // MQTT 포트
const char* topic = "room/302/control/*";
char message_buff[100];

DHT dht(DHTPIN, DHTTYPE);
Servo power;
Servo temp;

WiFiClient wifiClient;
void moveServo(Servo sv,int angle,int back){
  sv.write(angle);
  delay(500);
  sv.write(back);
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
  if (msgString == "power"){
    moveServo(power,45,0);
  }else if(msgString == "up"){
    moveServo(temp,120,60);
  }else if(msgString == "down"){
    moveServo(temp,0,60);
  }
}
PubSubClient mqttClient(mqttServer, mqttPort, callback, wifiClient);
void setup() {

  Serial.begin(115200);
  power.attach(PWPIN);
  temp.attach(TEMPPIN);
  power.write(0);
  temp.write(60);
  dht.begin();
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
  if(mqttClient.connect("mqttclient")){
    mqttClient.subscribe(topic);
  }
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }
  String temphumi = String("Temperature: ") + t + ", Humidity: " + h;
  String airconpower = String("Power: ") + "off";
  mqttClient.publish("room/302/info/temphumi", temphumi.c_str());
  mqttClient.publish("topic/302/info/aircon", airconpower.c_str());
  delay(2000); // 2초마다 데이터 전송
}

void reconnect() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect("mqttclient")) {
      Serial.println("Connected to MQTT Broker");
    } else {
      delay(5000);
    }
  }
}
