#include <Arduino.h>
#include <NativeEthernet.h>
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

  char buf[256];
  PacketComm packet;

  // // Ethernet
  // byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  // IPAddress remoteIp(192, 168, 150, 174); // Edit this to the IP of your computer runing COSMOS Web
  // unsigned short remotePort = 10095;
  // unsigned short localPort = 10095;
  // EthernetUDP udp;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    continue;

  // Serial.println("Initializing Ethernet...");
  // if (Ethernet.begin(mac) == 0)
  // {
  //   Serial.println("Ethernet not connected");
  // }
  // udp.begin(localPort);

  threads.setSliceMillis(10);

  // Threads
  thread_list.push_back({threads.addThread(Channels::rfm23_channel, 9000), Channels::Channel_ID::RFM23_CHANNEL});
}

void loop()
{
  handle_cmd();

  if (PullQueue(packet, main_queue, main_queue_mtx))
  {
    //   // StaticJsonDocument<1100> doc;
    //   // doc["node_name"] = "artemis_teensy";

    //   switch (packet.header.type)
    //   {
    //   case PacketComm::TypeId::DataObcBeacon:
    //   {
    //     switch ((TypeId)packet.data[0])
    //     {
    //     case TypeId::current1:
    //     {
    //       Serial.println("Recieved Current Beacon 1");
    //       currentbeacon1 beacon1;
    //       memcpy(&beacon1, packet.data.data(), sizeof(beacon1));
    //       // doc["time"] = beacon1.deci;
    //       for (int i = 0; i < ARTEMIS_CURRENT_BEACON_1_COUNT; i++)
    //       {
    //         // JsonObject current1 = doc.createNestedObject(current_sen_names[i]);
    //         // current1["vol"] = beacon1.busvoltage[i];
    //         // current1["cur"] = beacon1.current[i];
    //       }
    //       break;
    //     }
    //     case TypeId::current2:
    //     {
    //       Serial.println("Recieved Current Beacon 2");
    //       currentbeacon2 beacon2;
    //       memcpy(&beacon2, packet.data.data(), sizeof(beacon2));
    //       for (int i = 2; i < ARTEMIS_CURRENT_SENSOR_COUNT; i++)
    //       {
    //         // JsonObject current2 = doc.createNestedObject(current_sen_names[i]);
    //         // current2["vol"] = beacon2.busvoltage[i];
    //         // current2["cur"] = beacon2.current[i];
    //       }
    //       break;
    //     }
    //     case TypeId::temperature:
    //     {
    //       Serial.println("Recieved Temperature Beacon");
    //       temperaturebeacon beacon;
    //       memcpy(&beacon, packet.data.data(), sizeof(beacon));
    //       // doc["time"] = beacon.deci;
    //       for (int i = 0; i < ARTEMIS_TEMP_SENSOR_COUNT; i++)
    //       {
    //         // JsonObject temperature = doc.createNestedObject(temp_sen_names[i]);
    //         // temperature["celsius"] = beacon.temperatureC[i];
    //       }
    //       break;
    //     }
    //     case TypeId::imu:
    //     {
    //       Serial.println("Recieved IMU Beacon 1");
    //       imubeacon beacon;
    //       memcpy(&beacon, packet.data.data(), sizeof(beacon));
    //       // doc["time"] = beacon.deci;

    //       // JsonObject imu_temp = doc.createNestedObject("temp");
    //       // imu_temp["temp"] = beacon.imutemp;

    //       // // JsonObject imu = doc.createNestedObject("imu");
    //       // imu["accelx"] = beacon.accelx;
    //       // imu["accely"] = (beacon.accely);
    //       // imu["accelz"] = (beacon.accelz);

    //       // imu["gyrox"] = (beacon.gyrox);
    //       // imu["gyrox"] = (beacon.gyroy);
    //       // imu["gyrox"] = (beacon.gyroz);

    //       break;
    //     }
    //     case TypeId::mag:
    //     {
    //       Serial.println("Recieved IMU Beacon 2");
    //       magbeacon beacon;
    //       memcpy(&beacon, packet.data.data(), sizeof(beacon));
    //       // doc["time"] = beacon.deci;

    //       // JsonObject mag = doc.createNestedObject("mag");
    //       mag["magx"] = (beacon.magx);
    //       mag["magy"] = (beacon.magy);
    //       mag["magz"] = (beacon.magz);

    //       break;
    //     }
    //     default:
    //       break;
    //     }
    //     // // Send UDP packet
    //     // udp.beginPacket(remoteIp, remotePort);
    //     // serializeJson(doc, udp);
    //     // udp.println();
    //     // udp.endPacket();
    //     break;
    //   }
    //   default:
    //     break;
    //   }
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
        Serial.println("Incorrect command, usage: EpsSwitchName <switch name> 1|0");
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
    Serial.println("Command sent to radio. Waiting to send...");
  }
}
