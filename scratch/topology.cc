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
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

int main(int argc, char *argv[])
{
  // LogComponentEnableAll(LOG_LEVEL_INFO);
  LogComponentEnable("NewAppServer", LOG_LEVEL_INFO);
  LogComponentEnable("NewAppClient", LOG_LEVEL_INFO);
  std::string topo = "p2p";
  CommandLine cmd;
  uint16_t Scenario;
  cmd.AddValue("Topology","Testing topology",topo);
  cmd.AddValue("Scenario","Scenario Type(0,1,2)",Scenario);
  cmd.Parse(argc,argv);

  // LogComponentEnable("NewAppServer", LOG_LEVEL_FUNCTION);
  // LogComponentEnable("NewAppClient", LOG_LEVEL_FUNCTION);
  // LogComponentEnableAll(LOG_LEVEL_FUNCTION);
  if (topo == "csma"){
    uint16_t clientNum = 3;
    uint16_t serverPort = 9;

    NodeContainer terminal;
    terminal.Create(clientNum+1);

    Ptr<Node> serverPtr = terminal.Get(clientNum);

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
    clientHelper.SetAttribute("ScenarioType",UintegerValue(Scenario));

    ApplicationContainer serverContainer;
    ApplicationContainer clientContainer;

    serverContainer.Add(serverHelper.Install(serverPtr));

    for(int i=0; i<clientNum; i++) {
      // Scenario 0, 1, 2 circular
      // clientHelper.SetAttribute("ScenarioType", UintegerValue(i%3));
      clientContainer.Add(clientHelper.Install(terminal.Get(i)));
    }

    serverContainer.Start(Seconds(1.0));
    clientContainer.Start(Seconds(1.1));

  } 
  else if (topo == "p2p")
  {    
    Ptr<Node> p1 = CreateObject<Node> ();
    Ptr<Node> p2 = CreateObject<Node> ();
    Ptr<Node> p3 = CreateObject<Node> ();
    Ptr<Node> chat_server = CreateObject<Node> ();

    NodeContainer nodes = NodeContainer (p1, p2, p3, chat_server);

    NodeContainer p1_server = NodeContainer(p1, chat_server);
    NodeContainer p2_server = NodeContainer(p2, chat_server);
    NodeContainer p3_server = NodeContainer(p3, chat_server);

    InternetStackHelper stack;
    stack.Install (nodes);
    //install p2p
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer nd_p1_server = p2p.Install (p1_server);
    NetDeviceContainer nd_p2_server = p2p.Install (p2_server);
    NetDeviceContainer nd_p3_server  = p2p.Install (p3_server);

    //add ip address
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ii_p1_server = ipv4.Assign (nd_p1_server);
    ipv4.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer ii_p2_server = ipv4.Assign (nd_p2_server);
    ipv4.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer ii_p3_server = ipv4.Assign (nd_p3_server);
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    uint16_t port = 8080;
    //Address address (InetSocketAddress (ii_p1_server.GetAddress (1), port));

    NewAppServerHelper serverHelper(port);
    NewAppClientHelper clientHelper(ii_p1_server.GetAddress(1), port);
    clientHelper.SetAttribute("ScenarioType",UintegerValue(Scenario));

    ApplicationContainer serverContainer;
    ApplicationContainer clientContainer;

    serverContainer.Add(serverHelper.Install(chat_server));

    clientContainer.Add(clientHelper.Install(p1));
    clientContainer.Add(clientHelper.Install(p2));
    clientContainer.Add(clientHelper.Install(p3));

    serverContainer.Start(Seconds(1.0));
    clientContainer.Start(Seconds(1.1));
    
  } 
  
  

  Simulator::Run();
  Simulator::Stop(Seconds(10));
  Simulator::Destroy();
}
