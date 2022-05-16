/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

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

NS_LOG_COMPONENT_DEFINE("proj");


int main (int argc, char *argv[])
{
  LogComponentEnable("proj", LOG_LEVEL_INFO);

  int clientNum = 1;
  NodeContainer clients;
  clients.Create(clientNum);
  
  NodeContainer server;
  server.Create(1);
  Ptr<Node> serverPtr = server.Get(0);

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", StringValue("10us"));

  NetDeviceContainer clientDevices;
  NetDeviceContainer serverDevice;

  for (int i = 0; i < clientNum; i++)
  {
    NetDeviceContainer link = csma.Install (NodeContainer (clients.Get(i), serverPtr));
    clientDevices.Add (link.Get(0));
    serverDevice.Add (link.Get(1));
  }

  InternetStackHelper internet;
  internet.Install (clients);
  internet.Install (server);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4.Assign (clientDevices);

  Ipv4AddressHelper ipv4_server;
  ipv4_server.SetBase("10.1.2.0", "255.255.255.0");
  ipv4_server.Assign(serverDevice);

  Simulator::Run ();
  Simulator::Stop (Seconds(20));
  Simulator::Destroy ();

}

