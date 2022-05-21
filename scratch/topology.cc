#include <iostream>
#include <fstream>
#include <vector>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/csma-module.h"

using namespace ns3;

int main(int argc, char *argv[])
{
  // LogComponentEnableAll(LOG_LEVEL_INFO);
  LogComponentEnable("NewAppServer", LOG_LEVEL_INFO);
  LogComponentEnable("NewAppClient", LOG_LEVEL_INFO);

  uint16_t clientNum = 5;
  uint16_t serverPort = 9;

  NodeContainer terminal;
  terminal.Create(clientNum+1);

  Ptr<Node> serverPtr = terminal.Get(clientNum);

  // PointToPointHelper p2pHelper;
  // p2pHelper.SetDeviceAttribute("DataRate", DataRateValue(5000000));
  // p2pHelper.SetChannelAttribute("Delay", StringValue("10us"));

  CsmaHelper csmaHelper;
  csmaHelper.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csmaHelper.SetChannelAttribute("Delay", StringValue("10us"));

  NetDeviceContainer nds;
  nds = csmaHelper.Install(terminal);

  InternetStackHelper internet;
  internet.Install(terminal);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interface;
  interface = ipv4.Assign(nds);

  NewAppServerHelper serverHelper(serverPort);
  NewAppClientHelper clientHelper(interface.GetAddress(clientNum), serverPort);

  ApplicationContainer serverContainer;
  ApplicationContainer clientContainer;

  serverContainer.Add(serverHelper.Install(serverPtr));

  for(int i=0; i<clientNum; i++) {
    clientContainer.Add(clientHelper.Install(terminal.Get(i)));
  }

  serverContainer.Start(Seconds(1.0));
  clientContainer.Start(Seconds(1.1));

  Simulator::Run();
  Simulator::Stop(Seconds(1.5));
  Simulator::Destroy();
}
