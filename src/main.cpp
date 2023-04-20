#include <Arduino.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include <string.h>
#include "channels/artemis_channels.h"
#include <support/configCosmosKernel.h>
#include <artemisbeacons.h>

void handle_cmd();

namespace
{
  using namespace Artemis::Teensy;
  PacketComm packet;

  uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

  const IPAddress remoteIP(192, 168, 157, 228);
  const unsigned int remotePort = 10095;

  EthernetUDP udp;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    continue;

  Ethernet.begin(mac);
  delay(1000);

  unsigned int localPort = 8888;
  udp.begin(localPort);

  threads.setSliceMillis(10);

  // Threads
  thread_list.push_back({threads.addThread(Channels::rfm23_channel, 9000), Channels::Channel_ID::RFM23_CHANNEL});

  Serial.println("Ground Station Test Software Setup Complete");
}

String temperatureBeaconToJson(const temperaturebeacon &beacon)
{
  StaticJsonDocument<256> doc;
  doc["type"] = static_cast<uint8_t>(beacon.type);
  doc["deci"] = beacon.deci;
  JsonArray tempArray = doc.createNestedArray("temperatureC");
  for (const auto &temp : beacon.temperatureC)
  {
    tempArray.add(temp);
  }
  String json;
  serializeJson(doc, json);
  return json;
}

String currentBeacon1ToJson(const currentbeacon1 &beacon)
{
  StaticJsonDocument<1000> doc;
  doc["type"] = static_cast<uint8_t>(beacon.type);
  doc["deci"] = beacon.deci;
  JsonArray voltageArray = doc.createNestedArray("busvoltage");
  JsonArray currentArray = doc.createNestedArray("current");
  for (const auto &voltage : beacon.busvoltage)
  {
    voltageArray.add(voltage);
  }
  for (const auto &current : beacon.current)
  {
    currentArray.add(current);
  }
  String json;
  serializeJson(doc, json);
  return json;
}

String currentBeacon2ToJson(const currentbeacon2 &beacon)
{
  StaticJsonDocument<256> doc;
  doc["type"] = static_cast<uint8_t>(beacon.type);
  doc["deci"] = beacon.deci;
  JsonArray voltageArray = doc.createNestedArray("busvoltage");
  JsonArray currentArray = doc.createNestedArray("current");
  for (const auto &voltage : beacon.busvoltage)
  {
    voltageArray.add(voltage);
  }
  for (const auto &current : beacon.current)
  {
    currentArray.add(current);
  }
  String json;
  serializeJson(doc, json);
  return json;
}

String magBeaconToJson(const magbeacon &beacon)
{
  StaticJsonDocument<128> doc;
  doc["type"] = static_cast<uint8_t>(beacon.type);
  doc["deci"] = beacon.deci;
  doc["magx"] = beacon.magx;
  doc["magy"] = beacon.magy;
  doc["magz"] = beacon.magz;
  String json;
  serializeJson(doc, json);
  return json;
}

String imuBeaconToJson(const imubeacon &beacon)
{
  StaticJsonDocument<256> doc;
  doc["type"] = static_cast<uint8_t>(beacon.type);
  doc["deci"] = beacon.deci;
  doc["accelx"] = beacon.accelx;
  doc["accely"] = beacon.accely;
  doc["accelz"] = beacon.accelz;
  doc["gyrox"] = beacon.gyrox;
  doc["gyroy"] = beacon.gyroy;
  doc["gyroz"] = beacon.gyroz;
  doc["imutemp"] = beacon.imutemp;
  String json;
  serializeJson(doc, json);
  return json;
}

String gpsBeaconToJson(const gpsbeacon &beacon)
{
  StaticJsonDocument<256> doc;
  doc["type"] = static_cast<uint8_t>(beacon.type);
  doc["deci"] = beacon.deci;
  doc["latitude"] = beacon.latitude;
  doc["longitude"] = beacon.longitude;
  doc["speed"] = beacon.speed;
  doc["angle"] = beacon.angle;
  doc["altitude"] = beacon.altitude;
  doc["satellites"] = beacon.satellites;
  String json;
  serializeJson(doc, json);
  return json;
}

String switchBeaconToJson(const switchbeacon &beacon)
{
  StaticJsonDocument<256> doc;
  doc["type"] = static_cast<uint8_t>(beacon.type);
  doc["deci"] = beacon.deci;
  JsonArray swArray = doc.createNestedArray("sw");
  for (const auto &sw : beacon.sw)
  {
    swArray.add(sw);
  }
  String json;
  serializeJson(doc, json);
  return json;
}

void sendJsonOverUdp(const String &json)
{
  udp.beginPacket(remoteIP, remotePort);
  udp.write(json.c_str());
  udp.endPacket();
}

void loop()
{
  handle_cmd();

  if (PullQueue(packet, main_queue, main_queue_mtx))
  {
    switch (packet.header.type)
    {
    case PacketComm::TypeId::DataObcBeacon:
    {

      switch (static_cast<TypeId>(packet.data[0]))
      {
      case TypeId::current1:
      {
        Serial.println("Received Current Beacon 1");
        currentbeacon1 beacon1;
        packet.data.resize(sizeof(beacon1));
        memcpy(&beacon1, packet.data.data(), sizeof(beacon1));
        String json = currentBeacon1ToJson(beacon1);
        sendJsonOverUdp(json);
        break;
      }
      case TypeId::current2:
      {
        Serial.println("Received Current Beacon 2");
        currentbeacon2 beacon2;
        packet.data.resize(sizeof(beacon2));
        memcpy(&beacon2, packet.data.data(), sizeof(beacon2));
        String json = currentBeacon2ToJson(beacon2);
        sendJsonOverUdp(json);
        break;
      }
      case TypeId::temperature:
      {
        Serial.println("Received Temperature Beacon");
        temperaturebeacon beacon;
        memcpy(&beacon, packet.data.data(), sizeof(beacon));
        String json = temperatureBeaconToJson(beacon);
        sendJsonOverUdp(json);
        break;
      }
      case TypeId::mag:
      {
        Serial.println("Received Magnetometer Beacon");
        magbeacon beacon;
        memcpy(&beacon, packet.data.data(), sizeof(beacon));
        String json = magBeaconToJson(beacon);
        sendJsonOverUdp(json);
        break;
      }
      case TypeId::imu:
      {
        Serial.println("Received IMU Beacon");
        imubeacon beacon;
        memcpy(&beacon, packet.data.data(), sizeof(beacon));
        String json = imuBeaconToJson(beacon);
        sendJsonOverUdp(json);
        break;
      }
      case TypeId::gps:
      {
        Serial.println("Received GPS Beacon");
        gpsbeacon beacon;
        memcpy(&beacon, packet.data.data(), sizeof(beacon));
        String json = gpsBeaconToJson(beacon);
        sendJsonOverUdp(json);
        break;
      }
      case TypeId::sw:
      {
        Serial.println("Received Switch Status Beacon");
        switchbeacon beacon;
        memcpy(&beacon, packet.data.data(), sizeof(beacon));
        String json = switchBeaconToJson(beacon);
        sendJsonOverUdp(json);
        break;
      }
      default:
        Serial.println("Received Unknown Packet");
        break;
      }
      break;
    }
    default:
      break;
    }
  }
}

void handle_cmd()
{
  if (Serial.available() > 0)
  {
    String input = Serial.readStringUntil('\n');
    char *cmdline = input.begin();

    vector<string> args;

    for (char *token = strtok(cmdline, " "); token != NULL; token = strtok(NULL, " "))
    {
      args.push_back(string(token));
    }

    if (args.size() < 2)
    {
      Serial.println("COMMAND SYNTAX: <node_dest> <command> [data]");
      return;
    }

    // Clear packet
    packet.data.resize(0);
    packet.wrapped.resize(0);

    // Set origin
    packet.header.nodeorig = NODES::GROUND_NODE_ID;

    // Get destination
    if (NodeType.find(args[0]) != NodeType.end())
    {
      packet.header.nodedest = NodeType.find(args[0])->second;
    }
    else
    {
      Serial.println("Invalid Node Destination. Options=[artemis_teensy, artemis_rpi]");
      return;
    }

    packet.header.chanin = Artemis::Teensy::Channels::Channel_ID::RFM23_CHANNEL;
    packet.header.chanout = Artemis::Teensy::Channels::Channel_ID::RFM23_CHANNEL;

    // Parse command
    if (packet.StringType.find(args[1]) != packet.StringType.end())
    {
      packet.header.type = packet.StringType.find(args[1])->second;
    }
    else
    {
      Serial.println("Invalid Command Type");
      return;
    }

    switch (packet.header.type)
    {
    case PacketComm::TypeId::CommandEpsSwitchName:
    {
      if (args.size() < 4)
      {
        Serial.println("Incorrect command, usage: EpsSwitchName <switch name> 1|0");
        return;
      }
      String switch_name(args[2].c_str());
      switch_name = switch_name.toLowerCase();
      if (PDUType.find(switch_name.c_str()) != PDUType.end())
      {
        packet.data.push_back((uint8_t)PDUType.find(switch_name.c_str())->second);
      }
      else
      {
        Serial.println("Switch name not found");
        return;
      }
      if (args[3] == "1")
      {
        packet.data.push_back((uint8_t)1);
      }
      else if (args[3] == "0")
      {
        packet.data.push_back((uint8_t)0);
      }
      else
      {
        Serial.println("Incorrect command, usage: EpsSwitchName <switch name> 1|0");
        return;
      }
      break;
    }
    default:
      break;
    }
    // Store data
    // if (args.size() > 3)
    // {
    //   for (size_t i = 3; i < args.size(); i++)
    //   {
    //     for (size_t j = 0; j < args[i].size(); j++)
    //     {
    //       packet.data.push_back(args[i][j]);
    //     }
    //     if (i != args.size() - 1)
    //       packet.data.push_back(' ');
    //   }
    // }

    PushQueue(packet, rfm23_queue, rfm23_queue_mtx);
    Serial.println("Command sent to radio");
  }
}