#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"

#include "ns3/ipv4-address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"

#include "new-app-client.h"
#include "seq-ts-header.h"

#include "ns3/uinteger.h"
#include "ns3/double.h"

#include "ns3/socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/socket.h"
#include "ns3/tcp-socket.h"
#include "ns3/udp-socket.h"

#include <cstdlib>
#include <cstdio>

namespace ns3
{

  NS_LOG_COMPONENT_DEFINE("NewAppClient");

  NS_OBJECT_ENSURE_REGISTERED(NewAppClient);

  TypeId
  NewAppClient::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::NewAppClient")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<NewAppClient>()
                            .AddAttribute("RemoteAddress",
                                          "The destination Address of the outbound packets",
                                          AddressValue(),
                                          MakeAddressAccessor(&NewAppClient::server_address),
                                          MakeAddressChecker())
                            .AddAttribute("RemotePort", "The destination port of the outbound packets",
                                          UintegerValue(100),
                                          MakeUintegerAccessor(&NewAppClient::server_port),
                                          MakeUintegerChecker<uint16_t>())
                            .AddAttribute("PacketSize",
                                          "Packet Size for chatting",
                                          UintegerValue(512),
                                          MakeUintegerAccessor(&NewAppClient::message_size),
                                          MakeUintegerChecker<uint32_t>(12, 65507))
                            .AddAttribute("ScenarioType", "Scenario number", UintegerValue(0),
                                          MakeUintegerAccessor(&NewAppClient::scenario_type),
                                          MakeUintegerChecker<uint8_t>(0, 2));
    return tid;
  }

  NewAppClient::NewAppClient()
  {
    NS_LOG_FUNCTION(this);
    server_socket = 0;
    server_request_event = EventId();
    friend_socket = 0;
  }

  NewAppClient::~NewAppClient()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  NewAppClient::DoDispose(void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
  }

  void
  NewAppClient::StartApplication(void)
  {
    NS_LOG_FUNCTION(this);

    if (server_socket == 0)
    {
      server_socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
      if (Ipv4Address::IsMatchingType(server_address) == true)
      {
        if (server_socket->Bind() == -1)
        {
          NS_FATAL_ERROR("Failed to bind socket");
        }
        server_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(server_address), server_port));
      }
      else
      {
        NS_ASSERT_MSG(false, "Incompatible address type: " << server_address);
      }
    }
    server_socket->SetRecvCallback(MakeCallback(&NewAppClient::ServerReceiveCallback, this));
    server_request_event = Simulator::Schedule(Seconds(0.0), &NewAppClient::RequestServer, this);
    friend_port = 12321;

    server_socket->GetSockName(my_address);
  }

  void
  NewAppClient::StopApplication(void)
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(server_request_event);
  }

  void
  NewAppClient::RequestServer(void)
  {
    NS_LOG_FUNCTION(this);
    NS_ASSERT(server_request_event.IsExpired());

    uint8_t buffer[128];
    buffer[0] = 123;
    Ptr<Packet> p = Create<Packet>(buffer, 128);

    std::stringstream peerAddressStringStream;
    if (Ipv4Address::IsMatchingType(server_address))
    {
      peerAddressStringStream << Ipv4Address::ConvertFrom(server_address);
    }

    if ((server_socket->Send(p)) >= 0)
    {
      NS_LOG_INFO("CLIENT(" << InetSocketAddress::ConvertFrom(my_address).GetIpv4()
                            << ") " << peerAddressStringStream.str()
                            << " Time: " << (Simulator::Now()).GetSeconds());
    }
    else
    {
      NS_LOG_INFO("CLIENT(" << InetSocketAddress::ConvertFrom(my_address).GetIpv4() << ") "
                            << "Error while sending to" << peerAddressStringStream.str());
    }
  }

  // SERVER RECEIVE CALLBACK
  void
  NewAppClient::ServerReceiveCallback(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);

    Ptr<Packet> packet = socket->Recv();
    uint8_t buffer[128];
    packet->CopyData(buffer, 128);

    uint32_t addr_tmp;
    memcpy(&addr_tmp, buffer, sizeof(uint32_t));
    Ipv4Address friend_address_ipv4;
    friend_address_ipv4.Set(addr_tmp);
    friend_address = friend_address_ipv4;

    NS_LOG_INFO("CLIENT(" << InetSocketAddress::ConvertFrom(my_address).GetIpv4() << ") "
                          << "found friend: " << friend_address_ipv4);

    // CREATE SOCKET TO FRIEND

    if (friend_socket != 0)
      friend_socket->Close();
    friend_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    if (!Ipv4Address::IsMatchingType(friend_address))
      NS_ASSERT_MSG(false, "Incompatible address type: " << friend_address);
    InetSocketAddress local = InetSocketAddress(InetSocketAddress::ConvertFrom(my_address).GetIpv4(), friend_port);
    if (friend_socket->Bind(local) == -1)
      NS_FATAL_ERROR("Failed to bind socket");

    friend_socket->SetConnectCallback(
        MakeCallback(&NewAppClient::FriendConnectGoodCallback, this),
        MakeCallback(&NewAppClient::FriendConnectBadCallback, this));
    friend_socket->SetRecvCallback(
        MakeCallback(&NewAppClient::FriendReceiveCallback, this));

    // User who are requested to connect. IF TCP, will be used.
    // if (buffer[4] == 1)
    //   friend_socket->Connect(InetSocketAddress(friend_address_ipv4, friend_port));

    friend_socket->Connect(InetSocketAddress(friend_address_ipv4, friend_port));

    // Schedule Simulation
    switch (scenario_type)
    {
    case 0:
      Simulator::Schedule(Seconds(0.1), &NewAppClient::SendMessageToFriend, this, "0_0");
      Simulator::Schedule(Seconds(0.2), &NewAppClient::SendMessageToFriend, this, "0_1");
      Simulator::Schedule(Seconds(0.3), &NewAppClient::SendMessageToFriend, this, "0_2");
      break;
    case 1:
      Simulator::Schedule(Seconds(1.4), &NewAppClient::SendMessageToFriend, this, "1_0");
      Simulator::Schedule(Seconds(1.5), &NewAppClient::SendMessageToFriend, this, "1_1");
      Simulator::Schedule(Seconds(1.7), &NewAppClient::SendMessageToFriend, this, "1_2");
      break;
    case 2:
      Simulator::Schedule(Seconds(2.6), &NewAppClient::SendMessageToFriend, this, "2_0");
      Simulator::Schedule(Seconds(2.9), &NewAppClient::SendMessageToFriend, this, "2_1");
      Simulator::Schedule(Seconds(3.2), &NewAppClient::SendMessageToFriend, this, "2_2");
      break;
    default:
      break;
    }

  }

  void
  NewAppClient::FriendReceiveCallback(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);

    Address from;
    uint8_t buffer[message_size];
    int retval = socket->RecvFrom(buffer, message_size, 0, from);
    Ptr<Packet> packet = socket->Recv();

    NS_LOG_INFO("CLIENT(" << InetSocketAddress::ConvertFrom(my_address).GetIpv4()
                          << ") recv <------ " << InetSocketAddress::ConvertFrom(from).GetIpv4()
                          << ", Time: " << (Simulator::Now()).GetSeconds()
                          << ", Bytes: " << retval << " || Message: " << buffer);
  }

  void
  NewAppClient::FriendConnectGoodCallback(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_INFO("CLIENT(" << InetSocketAddress::ConvertFrom(my_address).GetIpv4() << ") "
                          << "friend connect SUCCESS");
  }

  void
  NewAppClient::FriendConnectBadCallback(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_INFO("CLIENT(" << InetSocketAddress::ConvertFrom(my_address).GetIpv4() << ") "
                          << "friend connect FAIL");
  }

  void NewAppClient::SendMessageToFriend(const char* message)
  {
    NS_LOG_FUNCTION(this);
    uint8_t buffer[message_size];
    uint32_t length = sizeof(message) / sizeof(char);
    memcpy(buffer, message, length > message_size ? message_size : length);
    Ptr<Packet> p = Create<Packet>(buffer, message_size);
    std::stringstream peerAddressStringStream;
    peerAddressStringStream << Ipv4Address::ConvertFrom(friend_address);

    int retval = friend_socket->Send(p);
    if (retval >= 0)
    {
      // std::cout << message << std::endl;
      NS_LOG_INFO("CLIENT(" << InetSocketAddress::ConvertFrom(my_address).GetIpv4()
                            << ") send ------> " << peerAddressStringStream.str()
                            << ", Time: " << (Simulator::Now()).GetSeconds()
                            << ", Bytes: " << (int)retval << " || Message: " << message
                            );
    }
    else
    {
      // std::cout << "ERROR" << std::endl;
      NS_LOG_INFO("CLIENT(" << InetSocketAddress::ConvertFrom(my_address).GetIpv4()
                            << ") send ------> " << peerAddressStringStream.str()
                            << ", Time: " << (Simulator::Now()).GetSeconds()
                            << " || [ErrorCode] " << (int)friend_socket->GetErrno()
                            );
    }
  }

} // Namespace ns3
