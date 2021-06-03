#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <time.h>

const char* root_ca = \
                      "-----BEGIN CERTIFICATE-----\n" \
                      "MIIFXTCCBEWgAwIBAgISA+5BW9lJIFmK67BavxfrdLuvMA0GCSqGSIb3DQEBCwUA\n" \
                      "MEoxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MSMwIQYDVQQD\n" \
                      "ExpMZXQncyBFbmNyeXB0IEF1dGhvcml0eSBYMzAeFw0yMDA0MDEwMDQ3MTVaFw0y\n" \
                      "MDA2MzAwMDQ3MTVaMBUxEzARBgNVBAMTCnZjLXNydnIucnUwggEiMA0GCSqGSIb3\n" \
                      "DQEBAQUAA4IBDwAwggEKAoIBAQDGHrk+uukGIsTPSxpTdS4kx3MIZ1e34ZlhzVkV\n" \
                      "TIR1WB14x4PGbrP2FyG/qXiwXFAsbV2h7gxFOl3eaw9/zMe5x2LhYhMq6Jjx6x8c\n" \
                      "/B2fFfIHzuYE98xNzefOd0bJo5m9SXrMCRcTt1kHEzFk3WELahfJbMXg10rpnxvs\n" \
                      "sCzQNRR2lqx4j4eb/cu3z6/TOA/qeya03zmgKw/zb1W+6lksWhi5XfjOEwirpkZA\n" \
                      "vnsPhm4gozkFj6OKuevehDXcReArOTMtYZNrl8udIOZUodcQNDfchzU4NLPzeGAq\n" \
                      "5jG9U10SVwddXIpwzDCLeNlc5jGpEB0RQQPGuU0qSZfP2DDXAgMBAAGjggJwMIIC\n" \
                      "bDAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMC\n" \
                      "MAwGA1UdEwEB/wQCMAAwHQYDVR0OBBYEFLhimPWLxtOJ2Ab6dUpNeQdeclKPMB8G\n" \
                      "A1UdIwQYMBaAFKhKamMEfd265tE5t6ZFZe/zqOyhMG8GCCsGAQUFBwEBBGMwYTAu\n" \
                      "BggrBgEFBQcwAYYiaHR0cDovL29jc3AuaW50LXgzLmxldHNlbmNyeXB0Lm9yZzAv\n" \
                      "BggrBgEFBQcwAoYjaHR0cDovL2NlcnQuaW50LXgzLmxldHNlbmNyeXB0Lm9yZy8w\n" \
                      "JQYDVR0RBB4wHIIKdmMtc3J2ci5ydYIOd3d3LnZjLXNydnIucnUwTAYDVR0gBEUw\n" \
                      "QzAIBgZngQwBAgEwNwYLKwYBBAGC3xMBAQEwKDAmBggrBgEFBQcCARYaaHR0cDov\n" \
                      "L2Nwcy5sZXRzZW5jcnlwdC5vcmcwggEFBgorBgEEAdZ5AgQCBIH2BIHzAPEAdwBe\n" \
                      "p3P531bA57U2SH3QSeAyepGaDIShEhKEGHWWgXFFWAAAAXEza81UAAAEAwBIMEYC\n" \
                      "IQDGMt2B8ev5P8yE1t03ihbBMUmXOGjXCvXb0v9DvFWjjgIhANwnsD9Dg1rPdniX\n" \
                      "tIqH++G4t44hT+FhOqEmVlUlAtdHAHYAB7dcG+V9aP/xsMYdIxXHuuZXfFeUt2ru\n" \
                      "vGE6GmnTohwAAAFxM2vNnQAABAMARzBFAiAyFifPTlpdWZ3vaHlrsbsaWweJIbEX\n" \
                      "R8Lj0ElGqdTk1AIhAJ0qDsJnkJ7piJg8g8VYhWonr3OemGJikPypH+L+rZHPMA0G\n" \
                      "CSqGSIb3DQEBCwUAA4IBAQAgmNV69leD4Xgsn3y0o8vqy6AD3EQzSCCh2tRf/Cax\n" \
                      "cOcUdSNFVpQum1T4CcH63vYDsLSsnyUF5gjL3b43flMHK9Koqb8vWwPcb+6nZrLU\n" \
                      "o7cFOy73uvZ7F1OcBDhCHXYXNi5N42U5ESxGDBL7BuPYb+vK44eh/UWAhYt1Y8EI\n" \
                      "/lDfIrEH6qgu3m//bqdM+YFfj2pQsR+RZZN2G1Ssgak3TQJWt43NvvFOKSImJVv5\n" \
                      "KQB4mThKbBSBfYL/kUFR63KXUpbN9zDREIP5kDGIJqqdfDThIX+GA8TbYVfX99nH\n" \
                      "SD/9ROCxtFBv3w7hB80WZYKU0LRsDSXSyzRhasWy4ZxS\n" \
                      "-----END CERTIFICATE-----\n";

#define RXD2 16
#define TXD2 17

#define PARSE_AMOUNT 20
String intData[PARSE_AMOUNT];     // массив после парсинга
boolean recievedFlag;
boolean getStarted;
byte index1;
String string_convert = "";

const char* hostNAME = "back.vc-app.ru";
uint16_t  hostPORT = 443;
const char* hostGET = "https://back.vc-app.ru/dev/script?did=10155&sc_id=71";//5371
const char* aHttpMethod = "GET";
const char* hostPOST = "https://back.vc-app.ru/dev/datchik";
const char* hostGETF = "https://back.vc-app.ru/dev/status?did=10155";
HTTPClient http;
int httpCode = 0;

WiFiUDP ntpUDP;
int GTMOffset = 3;
bool flagNTP = 0;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", GTMOffset * 60 * 60, 60 * 60 * 1000);
uint16_t timeNow = 0;
static tm getDateTimeByParams(long time) {
  struct tm* newtime;
  const time_t tim = time;
  newtime = localtime(&tim);
  return *newtime;
}

static String getDateTimeStringByParams(tm* newtime, char* pattern = (char*)"%d/%m/%Y %H:%M:%S") {
  char buffer[30];
  strftime(buffer, 30, pattern, newtime);
  return buffer;
}

static String getEpochStringByParams(long time, char* pattern = (char*)"%d/%m/%Y %H:%M:%S") {
  //    struct tm *newtime;
  tm newtime;
  newtime = getDateTimeByParams(time);
  return getDateTimeStringByParams(&newtime, pattern);
}

struct RoomStruct {
  uint8_t id;
  uint16_t dayRoom[10];
  uint16_t timeStartRoom;
  float tempRoom;
  uint16_t co2Room;
  float humRoom;
  uint16_t mute;
  uint16_t at_home;
};

struct nowParametrRoomStruct {
  float tempRoom;
  float tempValveRoom;
  uint16_t co2Room;
  float humRoom;
  uint16_t peopleRoom;
} actualStruct, actualStruct2;

bool flow1 = 0;
bool flow2 = 0;

RoomStruct* OurRoomStruct = 0;
int RoomStructAmount = 0;
String arrayOfScriptForTheWeek;
uint16_t numberOfSettingsInWeek = 0;

RoomStruct* OurRoomStruct2 = 0;
int RoomStructAmount2 = 0;
uint16_t arrayOfScriptForTheWeek2[100];
uint16_t numberOfSettingsInWeek2 = 0;

bool connectionReadJSONflag = 0;
RoomStruct nowRoomStruct;
bool flagFoundScript = 0;
uint16_t nextTime;
bool flagNextDay = 0;
uint16_t numberNextDay = 0;


bool flagFoundScript2 = 0;
uint16_t nextTime2;
RoomStruct nowRoomStruct2;
bool flagNextDay2 = 0;
uint16_t numberNextDay2 = 0;

void clearBuf() {
  delete[] OurRoomStruct;
  RoomStructAmount = 0;

  delete[] OurRoomStruct2;
  RoomStructAmount2 = 0;

}

void getFlowStatus() {
  Serial.println("getFlowStatus");
  if ((WiFi.status() == WL_CONNECTED)) {
    http.begin(hostGETF, root_ca); //Specify the URL and certificate
    http.addHeader(headerName, headerValue);
    httpCode = http.GET();                                                  //Make the request
    String payload;
    //Serial.println(httpCode);
    if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
      //Serial.println(httpCode);
      //Serial.println(payload);//2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4)
    }
    else {
      Serial.println("Error on HTTP request");
    }
    http.end();
    if (httpCode == 200)
    {
      const size_t capacity = 2 * JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4);
      DynamicJsonBuffer jsonBuffer(capacity);
      JsonObject& root = jsonBuffer.parseObject(payload);
      if (!root.success())
      {
        Serial.println("Parsing root error");
      }
      JsonObject& rid47 = root["rid47"];
      if (rid47.success())
      {
        flow1 = rid47["flow"];
      }
      JsonObject& rid48 = root["rid48"];
      if (rid48.success())
      {
        flow2 = rid48["flow"];
      }
      jsonBuffer.clear();
      root.prettyPrintTo(Serial);
    }
    memset(&payload, 0, sizeof(payload));
    
  }
  else {
    Serial.println("WL_CONNECTED ERROR");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print("Connecting to Wifi ");  // "Пытаемся подключиться к WEP-сети, SSID: "
      Serial.println(WiFi.status());
      delay(1000);
    }
  }
}

//connect and pasre script
void connectionReadJSONRoom() {

  if ((WiFi.status() == WL_CONNECTED)) {
    http.begin(hostGET, root_ca); //Specify the URL and certificate
    http.addHeader(headerName, headerValue);
    httpCode = http.GET();                                                  //Make the request
    String payload;
    Serial.println(httpCode);
    if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
      //Serial.println(httpCode);
      Serial.println(payload);
    }
    else {
      Serial.println("Error on HTTP request");
    }
    http.end();
    if (httpCode == 200)
    {
      const size_t capacity = 24 * JSON_ARRAY_SIZE(0) + 2 * JSON_ARRAY_SIZE(1) + 4 * JSON_ARRAY_SIZE(2) + 2 * JSON_ARRAY_SIZE(3) + 6 * JSON_OBJECT_SIZE(3) + 3 * JSON_OBJECT_SIZE(4) + 12 * JSON_OBJECT_SIZE(8);
      DynamicJsonBuffer jsonBuffer(capacity);
      JsonObject& root = jsonBuffer.parseObject(payload);
      if (!root.success())
      {
        Serial.println("Parsing root error");
      }
      JsonObject& roomGroup0 = root[String("roomGroup") + String("0")];
      if (root.success())
      {
        for (int j = 0; j < (roomGroup0.size() - 1); j++)
        {
          JsonObject& dayGroup0 = roomGroup0[String("dayGroup") + String(j)];
          if (dayGroup0.success())
          {
            for (int k = 0; k < (dayGroup0.size() - 1); k++)
            {
              JsonArray& days = dayGroup0["days"];
              if (days.success())
              {
                for (int q = 0; q < (days.size()); q++)
                {
                  JsonObject& setting0 = dayGroup0[String("setting") + String(k)];
                  if (setting0.success())
                  {
                    //for ()

                    RoomStruct* Obj;//выделение памяти
                    if (RoomStructAmount == 0)
                    {
                      Obj = new RoomStruct[RoomStructAmount + 1];
                    }
                    else
                    {
                      RoomStruct* tempObj = new RoomStruct[RoomStructAmount + 1];
                      for (int r = 0; r < RoomStructAmount; r++)
                      {
                        tempObj[r] = OurRoomStruct[r]; // копируем во временный объект
                      }
                      delete [] Obj;
                      Obj = tempObj;
                    }
                    OurRoomStruct = Obj;
                    const char* time = setting0["time"];
                    char subHours[4] = {0,};
                    char subMin[4] = {0,};
                    subHours[0] = time[0];
                    subHours[1] = time[1];
                    subMin[0] = time[3];
                    subMin[1] = time[4];
                    OurRoomStruct[RoomStructAmount].timeStartRoom = dayGroup0["days"][q];// * 24 * 60 +  atoi(subHours) * 60 + atoi(subMin));
                    OurRoomStruct[RoomStructAmount].timeStartRoom = (OurRoomStruct[RoomStructAmount].timeStartRoom - 1) * 24 * 60 + atoi(subHours) * 60 + atoi(subMin);
                    OurRoomStruct[RoomStructAmount].id = RoomStructAmount;
                    OurRoomStruct[RoomStructAmount].tempRoom = setting0["temp"]; // 21
                    OurRoomStruct[RoomStructAmount].humRoom = setting0["hum"]; // 55
                    OurRoomStruct[RoomStructAmount].co2Room = setting0["co2"]; // 700
                    OurRoomStruct[RoomStructAmount].mute = setting0["mute"]; // 0
                    OurRoomStruct[RoomStructAmount].at_home = setting0["at_home"]; // 1
                    RoomStructAmount++;
                  } else Serial.println("Parsing setting0!");
                }
              } else Serial.println("Parsing days!");
            }
          } else Serial.println("Parsing dayGroup0!");
        }
      } else Serial.println("Parsing roomGroup0!");
      RoomStructAmount--;

      //sorting an array of structures in ascending order
      for (int i = 0; i < RoomStructAmount; i++)
      {
        for (int k = 0;  k < RoomStructAmount; k++) // внутренний цикл прохода
        {
          if (OurRoomStruct[k].timeStartRoom > OurRoomStruct[k + 1].timeStartRoom)
          {
            RoomStruct Obj;//выделение памяти
            Obj = OurRoomStruct[k + 1];
            OurRoomStruct[k + 1] = OurRoomStruct[k];
            OurRoomStruct[k] = Obj;
          }
        }
      }
      //pars second room
      JsonObject& roomGroup2 = root[String("roomGroup") + String("1")];
      if (root.success())
      {
        for (int j = 0; j < (roomGroup2.size() - 1); j++)
        {
          JsonObject& dayGroup2 = roomGroup2[String("dayGroup") + String(j)];
          if (dayGroup2.success())
          {
            for (int k = 0; k < (dayGroup2.size() - 1); k++)
            {
              JsonArray& days2 = dayGroup2["days"];
              if (days2.success())
              {
                for (int q = 0; q < (days2.size()); q++)
                {
                  JsonObject& setting2 = dayGroup2[String("setting") + String(k)];
                  if (setting2.success())
                  {
                    //for ()

                    RoomStruct* Obj;//выделение памяти
                    if (RoomStructAmount2 == 0)
                    {
                      Obj = new RoomStruct[RoomStructAmount2 + 1];
                    }
                    else
                    {
                      RoomStruct* tempObj = new RoomStruct[RoomStructAmount2 + 1];
                      for (int r = 0; r < RoomStructAmount2; r++)
                      {
                        tempObj[r] = OurRoomStruct2[r]; // копируем во временный объект
                      }
                      delete [] Obj;
                      Obj = tempObj;
                    }
                    OurRoomStruct2 = Obj;
                    const char* time = setting2["time"];
                    char subHours[4] = {0,};
                    char subMin[4] = {0,};
                    subHours[0] = time[0];
                    subHours[1] = time[1];
                    subMin[0] = time[3];
                    subMin[1] = time[4];
                    OurRoomStruct2[RoomStructAmount2].timeStartRoom = dayGroup2["days"][q];// * 24 * 60 +  atoi(subHours) * 60 + atoi(subMin));
                    OurRoomStruct2[RoomStructAmount2].timeStartRoom = (OurRoomStruct2[RoomStructAmount2].timeStartRoom - 1) * 24 * 60 + atoi(subHours) * 60 + atoi(subMin);
                    OurRoomStruct2[RoomStructAmount2].id = RoomStructAmount2;
                    OurRoomStruct2[RoomStructAmount2].tempRoom = setting2["temp"]; // 21
                    OurRoomStruct2[RoomStructAmount2].humRoom = setting2["hum"]; // 55
                    OurRoomStruct2[RoomStructAmount2].co2Room = setting2["co2"]; // 700
                    OurRoomStruct2[RoomStructAmount2].mute = setting2["mute"]; // 0
                    OurRoomStruct2[RoomStructAmount2].at_home = setting2["at_home"]; // 1
                    RoomStructAmount2++;
                  } else Serial.println("Parsing setting0!");
                }
              } else Serial.println("Parsing days!");
            }
          } else Serial.println("Parsing dayGroup0!");
        }
      } else Serial.println("Parsing roomGroup0!");
      RoomStructAmount2--;

      //sorting an array of structures in ascending order
      for (int i = 0; i < RoomStructAmount2; i++)
      {
        for (int k = 0;  k < RoomStructAmount2; k++) // внутренний цикл прохода
        {
          if (OurRoomStruct2[k].timeStartRoom > OurRoomStruct2[k + 1].timeStartRoom)
          {
            RoomStruct Obj;//выделение памяти
            Obj = OurRoomStruct2[k + 1];
            OurRoomStruct2[k + 1] = OurRoomStruct2[k];
            OurRoomStruct2[k] = Obj;
          }
        }
      }
      jsonBuffer.clear();
    }
    else Serial.println("Error in response");
    memset(&payload, 0, sizeof(payload));
    connectionReadJSONflag = 1;
    /*for (int i = 0; i <= RoomStructAmount; i++)
    {
      Serial.println(OurRoomStruct[i].id);
      Serial.println(OurRoomStruct[i].timeStartRoom);
      Serial.println(OurRoomStruct[i].tempRoom);
      Serial.println(OurRoomStruct[i].humRoom);
      Serial.println(OurRoomStruct[i].co2Room);
      Serial.println(OurRoomStruct[i].mute);
      Serial.println(OurRoomStruct[i].at_home);
      Serial.println("________");
    }*/
    /*for (int i = 0; i < numberOfSettingsInWeek; i++) {//вывод массива сценариев
      Serial.print(arrayOfScriptForTheWeek[i]);
    }
    Serial.println(arrayOfScriptForTheWeek);*/
    /*Serial.println();
    for (int i = 0; i <= RoomStructAmount2; i++)
    {
      Serial.println(OurRoomStruct2[i].id);
      for (int j = 1; j < 8; j++) {
        Serial.print(OurRoomStruct2[i].dayRoom[j]);
      }
      Serial.println();
      Serial.println(OurRoomStruct2[i].timeStartRoom);
      Serial.println(OurRoomStruct2[i].tempRoom);
      Serial.println(OurRoomStruct2[i].humRoom);
      Serial.println(OurRoomStruct2[i].co2Room);
      Serial.println(OurRoomStruct2[i].mute);
      Serial.println(OurRoomStruct2[i].at_home);
      Serial.println("________");
    }*/
    /*for (int i = 0; i < numberOfSettingsInWeek2; i++)
    { //вывод массива сценариев
      Serial.print(arrayOfScriptForTheWeek2[i]);
      Serial.println();
    }*/
    Serial.println();

  }
  else {
    Serial.println("WL_CONNECTED ERROR");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print("Connecting to Wifi ");  // "Пытаемся подключиться к WEP-сети, SSID: "
      Serial.println(WiFi.status());
      delay(1000);
    }
  }
}

void checkNowScript() {
  if (flagNTP == 1) {
    for (int i = 0; i < RoomStructAmount; i++) {
      if (flagFoundScript == 0)
      {
        if ((OurRoomStruct[i].timeStartRoom <= timeNow) && (OurRoomStruct[i + 1].timeStartRoom > timeNow)) {
          {
            nowRoomStruct = OurRoomStruct[i];
            nextTime = OurRoomStruct[i + 1].timeStartRoom;
            flagFoundScript = 1;
            Serial.println("flag1");
          }
        }
      }
    }
    for (int i = 0; i < RoomStructAmount2; i++) {
      if (flagFoundScript2 == 0)
      {
        if ((OurRoomStruct2[i].timeStartRoom <= timeNow) && (OurRoomStruct2[i + 1].timeStartRoom > timeNow)) {
          {
            nowRoomStruct2 = OurRoomStruct2[i];
            nextTime2 = OurRoomStruct2[i + 1].timeStartRoom;
            flagFoundScript2 = 1;
            Serial.println("flag2");
          }
        }
      }
    }
  }
  else Serial.println("NTP not update");
  //if (timeClient.getDay() == 1) flagFoundScript = 0;
  if ((nextTime <= timeNow) && (flagNextDay == 0))
  {
    flagFoundScript = 0;
    Serial.println("(flagNextDay == 0) && (nextTime <= timeNow)");
  }
  if ((nextTime2 <= timeNow) && (flagNextDay2 == 0))
  {
    flagFoundScript2 = 0;
    Serial.println("(flagNextDay2 == 0) && (nextTime2 <= timeNow)");
  }

}

void printNowScript() {
  if (flagFoundScript == 1) {
    Serial.println("nowRoomStruct1:");
    Serial.println(String("nextTime") + String(nextTime));
    Serial.println(nowRoomStruct.id);
    Serial.println();
    Serial.println(nowRoomStruct.timeStartRoom);
    Serial.println(nowRoomStruct.tempRoom);
    Serial.println(nowRoomStruct.humRoom);
    Serial.println(nowRoomStruct.co2Room);
    Serial.println(nowRoomStruct.mute);
    Serial.println(nowRoomStruct.at_home);
    Serial.println("________");
  }
  else Serial.println("Сценария 1 нет");
  if (flagFoundScript2 == 1) {
    Serial.println("nowRoomStruct2:");
    Serial.println(String("nextTime") + String(nextTime2));
    Serial.println(nowRoomStruct2.id);
    Serial.println();
    Serial.println(nowRoomStruct2.timeStartRoom);
    Serial.println(nowRoomStruct2.tempRoom);
    Serial.println(nowRoomStruct2.humRoom);
    Serial.println(nowRoomStruct2.co2Room);
    Serial.println(nowRoomStruct2.mute);
    Serial.println(nowRoomStruct2.at_home);
    Serial.println("________");
  }
  else Serial.println("Сценария 2 нет");
}
// 13 14-1 снизу выступ на 14, не меньше 9 экран   , 73 ,33 центр микроюсб. 
void transmitDataToStm() {
  Serial2.write('*');
  Serial2.print(nowRoomStruct.tempRoom);
  Serial2.print(" ");
  Serial2.print(nowRoomStruct.humRoom);
  Serial2.print(" ");
  Serial2.print(nowRoomStruct.co2Room);
  Serial2.print(" ");

  Serial2.print(nowRoomStruct2.tempRoom);
  Serial2.print(" ");
  Serial2.print(nowRoomStruct2.humRoom);
  Serial2.print(" ");
  Serial2.print(nowRoomStruct2.co2Room);
  Serial2.print(" ");
  Serial2.print(flow1);
  Serial2.print(" ");
  Serial2.print(flow2);
  Serial2.print(";\r\n");
}

//send Data from sensors to server

void receiveDataToStm() {
  if (Serial2.available() > 0) {
    char incomingByte = Serial2.read();
    if (getStarted) {                         // если приняли начальный символ (парсинг разрешён)
      if (incomingByte != ' ' && incomingByte != ';') {   // если это не пробел И не конец
        string_convert += incomingByte;       // складываем в строку
      } else {                                // если это пробел или ; конец пакета
        intData[index1] = string_convert;  // преобразуем строку в int и кладём в массив
        string_convert = "";                  // очищаем строку
        index1++;                              // переходим к парсингу следующего элемента массива
      }
    }
    if (incomingByte == '*') {                // если это $
      getStarted = true;                      // поднимаем флаг, что можно парсить
      index1 = 0;                              // сбрасываем индекс
      string_convert = "";                    // очищаем строку
    }
    if (incomingByte == ';') {                // если таки приняли ; - конец парсинга
      getStarted = false;                     // сброс
      recievedFlag = true;
      Serial2.end(); // флаг на принятие
    }
  }
  if (recievedFlag == 1) {
    Serial.println(intData[0]);
    Serial.println(intData[1]);
    Serial.println(intData[2]);
    Serial.println(intData[3]);
    Serial.println(intData[4]);
    Serial.println(intData[5]);
    Serial.println(intData[6]);
    Serial.println(intData[7]);
    actualStruct.tempRoom = intData[0].toFloat();
    actualStruct.humRoom = intData[1].toFloat();
    actualStruct.co2Room = intData[2].toInt();
    actualStruct.peopleRoom = intData[3].toInt();
    actualStruct2.tempRoom = intData[4].toFloat();
    actualStruct2.humRoom = intData[5].toFloat();
    actualStruct2.co2Room = intData[6].toInt();
    actualStruct2.peopleRoom = intData[7].toInt();
    //
  }
}


void setServerParam() {

  if ((WiFi.status() == WL_CONNECTED)) {
    //
    const int capacity = JSON_OBJECT_SIZE(3) + 2 * JSON_OBJECT_SIZE(6);
    StaticJsonBuffer<capacity> jsonBuffer;
    JsonObject& did = jsonBuffer.createObject();
    did["did"] = "10155";
    JsonObject& rid39 = did.createNestedObject("rid39");//"rid39"
    rid39["rid"] = 47;
    rid39["temp"] = actualStruct.tempRoom;
    if (actualStruct.tempRoom < 100000) {
      actualStruct.tempRoom++;
    }
    else actualStruct.tempRoom = 0;
    rid39["temp_valve"] = (char*)0;
    rid39["humidity"] = actualStruct.humRoom;
    if (actualStruct.humRoom < 100000) {
      actualStruct.humRoom++;
    }
    else actualStruct.humRoom = 0;
    rid39["co2"] = actualStruct.co2Room;
    if (actualStruct.co2Room < 100000) {
      actualStruct.co2Room++;
    }
    else actualStruct.co2Room = 0;
    rid39["people"] = actualStruct.peopleRoom;
    if (actualStruct.peopleRoom < 100000) {
      actualStruct.peopleRoom++;
    }
    else actualStruct.peopleRoom = 0;
    JsonObject& rid40 = did.createNestedObject("rid40");//"rid40"
    rid40["rid"] = 48;
    rid40["temp"] = actualStruct2.tempRoom;
    if (actualStruct2.tempRoom < 100000) {
      actualStruct2.tempRoom++;
    }
    else actualStruct2.tempRoom = 0;
    rid40["temp_valve"] = (char*)0;
    rid40["humidity"] = actualStruct2.humRoom;
    if (actualStruct2.humRoom < 100000) {
      actualStruct2.humRoom++;
    }
    else actualStruct2.humRoom = 0;
    rid40["co2"] = actualStruct2.co2Room;
    if (actualStruct2.co2Room < 100000) {
      actualStruct2.co2Room++;
    }
    else actualStruct2.co2Room = 0;
    rid40["people"] = actualStruct2.peopleRoom;
    if (actualStruct2.peopleRoom < 100000) {
      actualStruct2.peopleRoom++;
    }
    else actualStruct2.peopleRoom = 0;

    did.prettyPrintTo(Serial);
    char jsonChar[255];
    did.printTo((char*)jsonChar, did.measureLength() + 1); //отправка данных на сервер
    http.begin(hostPOST, root_ca);
    http.addHeader(headerName, headerValue);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.sendRequest("POST", jsonChar); //получение кода ответа
    Serial.print("HTTP Response code: "); //Вывод кода в терминал
    Serial.println(httpResponseCode); // Вывод кода в терминал

    jsonBuffer.clear();
    memset(&jsonChar[0], 0, sizeof(jsonChar));
    http.end();
  }
  else {
    Serial.println("WL_CONNECTED ERROR");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print("Connecting to Wifi ");  // "Пытаемся подключиться к WEP-сети, SSID: "
      Serial.println(WiFi.status());
      delay(1000);
    }
  }
}

void setup() {
  M5.begin();
  Serial.begin(115200);
  int keyIndex = 0;                                // индекс ключа к вашей сети
  int status = WL_IDLE_STATUS;
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  }
  else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to Wifi ");  // "Пытаемся подключиться к WEP-сети, SSID: "
    Serial.println(WiFi.status());
    // if (WiFi.status() == 4) {
    //WiFi.close();
    //WiFi.begin(ssid, password);
    //}
    delay(1000);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Connected to the WiFi network");
  Serial.println("Setup done");
  timeClient.begin();
  delay(1000);
  if (timeClient.update()) {
    Serial.print("Adjust local clock");
    unsigned long epoch = timeClient.getEpochTime();
    // HERE I'M UPDATE LOCAL CLOCK
    setTime(epoch);
    flagNTP = 1;
  }
  else {
    flagNTP = 0;
    Serial.print("NTP Update not work");
  }

}

void loop() {
  if (connectionReadJSONflag == 0) {
    connectionReadJSONRoom();

  }
  if (flagNTP == 0) {
    if (timeClient.update()) {
      Serial.print("Adjust local clock");
      unsigned long epoch = timeClient.getEpochTime();
      // HERE I'M UPDATE LOCAL CLOCK
      setTime(epoch);
      flagNTP = 1;
    }
    else {
      flagNTP = 0;
      Serial.print("NTP Update not WORK!!");
    }
  }
  timeNow = ((timeClient.getDay() - 1) * 24 * 60) + (timeClient.getHours() * 60) + (timeClient.getMinutes());
  Serial.println(String("timeNow ") + String(timeNow));
  Serial.println(String("timeClient.getDay() = ") + String(timeClient.getDay()));
  if (httpCode == 200){
    checkNowScript();
    printNowScript();
  }
  getFlowStatus();
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  transmitDataToStm();
  while (recievedFlag != true) {
    receiveDataToStm();
  }
  setServerParam();
  //recievedFlag = false;
  //clearBuf();
}
