//
// Created by Aleksey Timin on 11/16/19.
//

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
#include <winsock2.h>
#define OS_Windows (1)
#endif

#include <sstream>
#include <cip/connectionManager/NetworkConnectionParams.h>
#include "SessionInfo.h"
#include "ConnectionManager.h"
#include "utils/Logger.h"
#include "utils/Buffer.h"

using namespace eipScanner::cip;
using eipScanner::SessionInfo;
using eipScanner::MessageRouter;
using eipScanner::ConnectionManager;
using eipScanner::cip::connectionManager::ConnectionParameters;
using eipScanner::cip::connectionManager::NetworkConnectionParams;
using eipScanner::utils::Buffer;
using eipScanner::utils::Logger;
using eipScanner::utils::LogLevel;

int main(int argc, char**argv) {
  Logger::setLogLevel(LogLevel::DEBUG);

  int implicit_port=2222;
  char *address;

  if(argc < 2) {
      Logger(LogLevel::ERROR) << "Must specify IFM IP address (and optionally port)";
      return EXIT_FAILURE;
  }

  address = argv[1];

  if(argc > 2) {
      implicit_port = atoi(argv[2]);
      Logger(LogLevel::INFO) << "Using implicit port " << implicit_port;
  }


#if OS_Windows
  WSADATA wsaData;
  int winsockStart = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (winsockStart != 0) {
    Logger(LogLevel::ERROR) << "Failed to start WinSock - error code: " << winsockStart;
    return EXIT_FAILURE;
  }
#endif

  auto si = std::make_shared<SessionInfo>(address, 0xAF12);

  // Implicit messaging
  ConnectionManager connectionManager(std::make_shared<MessageRouter>((CipUint)implicit_port));

  std::vector<uint8_t> path = {0x20, 0x04, 0x24, 0xC7, 0x2C, 0xC1, 0x2C, 0x64,
      0x80, 25,
      2,  // 8,Param28, Communication Profile Etherent/IP only
      0,  // 8,Param29, Port process data size, 2 bytes
      3,  // 8,Param30, Port 1 Mode, IO Link
      0,  // 8,Param31, Port 1 cycle time, as fast a possible
      1,  // 8,Param32, Port 1 swap, enabled
      0,  // 8,Param33, Port 1 validation, none
      0x1A, 0x00,    // 16,Param34, vendor id
      0xf1, 0x9c, 0x09, 0x00,    // 32,Param35, device ID
      0,  // 8,Param36, // No Fail Safe
      0,  // 8,Param37, // DO failsafe reset
      3,      // 8,Param130,
      0,      // 8,Param131,
      1,      // 8,Param132,
      0,      // 8,Param133,
      0x00, 0x00,    // 16,Param134,
      0x00, 0x00, 0x00, 0x00,    // 32,Param135,
      0,    // 8,Param136,
      0,    // 8,Param137,
      3,        // 8,Param230,
      0,        // 8,Param231,
      1,        // 8,Param232,
      0,        // 8,Param233,
      0x00, 0x00,     // 16,Param234,
      0x00, 0x00, 0x00, 0x00,     // 32,Param235,
      0,        // 8,Param236,
      0,        // 8,Param237,
      3,            // 8,Param330,
      0,            // 8,Param331,
      1,            // 8,Param332,
      0,            // 8,Param333,
      0x00, 0x00,    // 16,Param334,
      0x00, 0x00, 0x00, 0x00,    // 32,Param335,
      0,            // 8,Param336,
      0             // 8,Param337;
  };


  ConnectionParameters parameters;
  parameters.connectionPath = path;  // config Assm151, output Assm150, intput Assm100
  parameters.originatorVendorId = 342;
  parameters.originatorSerialNumber = 32423;
  parameters.o2tRealTimeFormat = false;
  parameters.t2oNetworkConnectionParams |= NetworkConnectionParams::FIXED;
  parameters.t2oNetworkConnectionParams |= NetworkConnectionParams::P2P;
  parameters.t2oNetworkConnectionParams |= NetworkConnectionParams::SCHEDULED_PRIORITY;
  parameters.t2oNetworkConnectionParams |= 246; //size of Assm100 =32
  parameters.o2tNetworkConnectionParams |= NetworkConnectionParams::FIXED;
  parameters.o2tNetworkConnectionParams |= NetworkConnectionParams::P2P;
  parameters.o2tNetworkConnectionParams |= NetworkConnectionParams::SCHEDULED_PRIORITY;
  parameters.o2tNetworkConnectionParams |= 0; //size of Assm150 = 32
  parameters.o2tRPI = 1000000;
  parameters.t2oRPI = 1000000;
  parameters.transportTypeTrigger |= NetworkConnectionParams::CLASS1;
  parameters.transportTypeTrigger |= NetworkConnectionParams::TRIG_CYCLIC;

  auto io = connectionManager.forwardOpen(si, parameters);
  if (auto ptr = io.lock()) {
    //ptr->setDataToSend(std::vector<uint8_t>(32));

    ptr->setReceiveDataListener([implicit_port](auto realTimeHeader, auto sequence, auto data) {
      std::ostringstream ss;
      ss << "secNum=" << sequence << " data=";
      for (auto &byte : data) {
        ss << "[" << std::hex << (int) byte << "]";
      }

      Logger(LogLevel::INFO) << "Received from IFM(" << implicit_port << ") : " << ss.str();
    });

    ptr->setCloseListener([]() {
      Logger(LogLevel::INFO) << "IFM A Closed";
    });
  }

  int count = 200;
  while (connectionManager.hasOpenConnections() && count-- > 0) {
    connectionManager.handleConnections(std::chrono::milliseconds(100));
  }

  connectionManager.forwardClose(si, io);

#if OS_Windows
  WSACleanup();
#endif

  return EXIT_SUCCESS;
}
