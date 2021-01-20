#include "MySensorsGate.h"

// Initialize general message
MyMessage msg(CHILD_ID, V_TEXT);

//void setup() {
    // Setup locally attached sensors
//}

//void presentation() {
    // Present locally attached sensors
//}

//void loop() {
    // Send locally attached sensor data here
//}

void receive(const MyMessage &message) {
    if ((message.getSensor() == 0) && (message.getSender() == 100)) {
        Serial.println(message.getFloat());
    }

    //if (message.type == V_STATUS) {
    //}

    // получаем сразу все данные из строки ДЕБАГА и разбираем её )))

    //Serial.println("message");
    //Serial.print("sensor: ");
    //Serial.println(message.sensor);
    //Serial.print("type: ");
    //Serial.println(message.type);
    //Serial.print("getBool(): ");
    //Serial.println(message.getBool());
    //Serial.print("getByte(): ");
    //Serial.println(message.getByte());
    //Serial.print("getString(): ");
    //Serial.println(message.getString());
    //
    //// Serial.println(message.getCustomString());
    //// Serial.println(message.getStream());
    //// Serial.println(message.getCustom());
    //Serial.println(message.getFloat());
    //Serial.println(message.getInt());
    //Serial.println(message.getLong());
    //Serial.print("getCommand(): ");
    //Serial.println(message.getCommand());
    //Serial.print("getSensor(): ");
    //Serial.println(message.getSensor());
    //Serial.print("getSender(): ");
    //Serial.println(message.getSender());
    //Serial.print("getDestination(): ");
    //Serial.println(message.getDestination());
    //Serial.print("getHeaderSize(): ");
    //Serial.println(message.getHeaderSize());
    //Serial.print("getPayloadType(): ");
    //Serial.println(message.getPayloadType());
    //Serial.print("getLength(): ");
    //Serial.println(message.getLength());
    //Serial.print("getExpectedMessageSize(): ");
    //Serial.println(message.getExpectedMessageSize());
    //Serial.print("isProtocolVersionValid(): ");
    //Serial.println(message.isProtocolVersionValid());
    //Serial.print("getLast(): ");
    //Serial.println(message.getLast());
    //Serial.print("isEcho(): ");
    //Serial.println(message.isEcho());
    //Serial.print("getRequestEcho(): ");
    //Serial.println(message.getRequestEcho());
    //Serial.print("getVersion(): ");
    //Serial.println(message.getVersion());
    //Serial.print("getSigned(): ");
    //Serial.println(message.getSigned());

    // Тут получаем и присваиваем значения переменных
    // 0 - это сенсор температуры с ноды 24 ( задано руками в скетче ноды ), 1 - это сенсор влажности

    //if ((message.getSensor() == 0) && (message.getSender() == 100)) {
    //    node24temp = message.getFloat();
    //}

    /*
    if ((message.getSensor() == 1) && (message.getSender() == 24)) {
        node24batt = message.getFloat();
    }
    
      if ((message.getSensor() == 0) && (message.getSender() == 61)) {
    node61temp = message.getFloat();
 
  }

      if ((message.getSensor() == 1) && (message.getSender() == 61)) {
    node61hum = message.getFloat();
  }

        if ((message.getSensor() == 2) && (message.getSender() == 61)) {
    node61pres = message.getFloat();
  }

          if ((message.getSensor() == 3) && (message.getSender() == 61)) {
    node61batt = message.getFloat();
  }  
  */
    /*
    if ((message.getSensor() == 1) && (message.getSender() == 133)) {
    noda133temp = message.getFloat();
 
  }

      if ((message.getSensor() == 2) && (message.getSender() == 133)) {
    noda133hum = message.getFloat();
  }

      if ((message.getSensor() == 110) && (message.getSender() == 133)) {
    noda133batt = message.getFloat();
  } 

      if ((message.getSensor() == 1) && (message.getSender() == 134)) {
    noda134temp = message.getFloat();
 
  }

      if ((message.getSensor() == 2) && (message.getSender() == 134)) {
    noda134hum = message.getFloat();
  }

        if ((message.getSensor() == 1) && (message.getSender() == 135)) {
    noda135temp = message.getFloat();
 
  }

      if ((message.getSensor() == 2) && (message.getSender() == 135)) {
    noda135hum = message.getFloat();
  }

          if ((message.getSensor() == 1) && (message.getSender() == 136)) {
    noda136temp = message.getFloat();
 
  }

      if ((message.getSensor() == 2) && (message.getSender() == 136)) {
    noda136hum = message.getFloat();
  }

      if ((message.getSensor() == 0) && (message.getSender() == 60)) {
    noda60temp = message.getFloat();

  }

      if ((message.getSensor() == 1) && (message.getSender() == 60)) {
    noda60hum = message.getFloat();
  }

        if ((message.getSensor() == 2) && (message.getSender() == 60)) {
    noda60pres = message.getFloat();
  }

          if ((message.getSensor() == 3) && (message.getSender() == 60)) {
    noda60batt = message.getFloat();
  }
  
  if ((message.getCommand() == 1) && (message.getSender() == 30)) {
    if (message.getSensor() == 1) { // данные с сенсора 1 ноды 30 к прмеру может быть температура (вы сами определяете номер сенсора для температуры)
      releSt = message.getBool();

    }
  }
// отправляем на ноду подтверждение, что получили данные и шлем её ответ ))) сразу )))

//  if ((message.getCommand() == 1) && (message.getSender() == 0) && (message.getSensor() == 1) && (message.getDestination() == 22)) {
//    //Confirmation_Answer_Accepted = 1;
//    //sleep_CHANGE = 0;
//    Serial.println(message.getString()); 
//    Serial.println("data for Sensor 22"); 
//    String str = message.getString();
//    str.trim();
//    Serial.println(message.getString()); 
//    if (str == "HIGH") {
//      //s = send(msg.setDestination(0).set("LOW")); 
//      send(msg.setDestination(22).set("Close")); 
//      Serial.println(message.getString()); 
//    }
//    if (str == "LOW") {
//      send(msg.setDestination(22).set("Open")); 
//      Serial.println(message.getString()); 
//    }
//  } 

 */
}