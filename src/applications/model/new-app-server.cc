#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/tcp-socket.h"
#include "ns3/tcp-socket-factory.h"

#include "seq-ts-header.h"
#include "new-app-server.h"

namespace ns3
{

  NS_LOG_COMPONENT_DEFINE("NewAppServer");
  NS_OBJECT_ENSURE_REGISTERED(NewAppServer);

  TypeId
  NewAppServer::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::NewAppServer")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<NewAppServer>()
                            .AddAttribute("Port",
                                          "Port on which we listen for incoming packets.",
                                          UintegerValue(100),
                                          MakeUintegerAccessor(&NewAppServer::m_port),
                                          MakeUintegerChecker<uint16_t>());
    return tid;
  }

  NewAppServer::NewAppServer()
  {
    NS_LOG_FUNCTION(this);
  }

  NewAppServer::~NewAppServer()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  NewAppServer::DoDispose(void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
    m_socket = 0;
    m_socketList.clear();
  }

  void
  NewAppServer::StartApplication(void)
  {
    NS_LOG_FUNCTION(this);

    if (m_socket == 0)
    {
      m_socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
      InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
      if (m_socket->Bind(local) == -1)
      {
        NS_FATAL_ERROR("Failed to bind socket");
      }
      m_socket->Listen();
    }
    m_socket->SetRecvCallback(MakeCallback(&NewAppServer::HandleRead, this));
    m_socket->SetAcceptCallback(
        MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
        // MakeNullCallback<void, Ptr<Socket>, const Address &>());
        MakeCallback(&NewAppServer::HandleAccept, this));
    m_socket->SetCloseCallbacks(
        // MakeNullCallback<void, Ptr<Socket>>(),
        // MakeNullCallback<void, Ptr<Socket>>());
        MakeCallback(&NewAppServer::HandlePeerClose, this),
        MakeCallback(&NewAppServer::HandlePeerError, this));
  }

  void
  NewAppServer::StopApplication()
  {
    NS_LOG_FUNCTION(this);
    while (!m_socketList.empty())
    {
      Ptr<Socket> acceptedSocket = m_socketList.front();
      m_socketList.pop_front();
      acceptedSocket->Close();
    }
    if (m_socket != 0)
    {
      m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
  }

  void
  NewAppServer::HandleRead(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
    Address from;
    Address localAddress;
    uint8_t buffer[128];
    socket->RecvFrom(buffer, 128, 0, from);
    userList.push_back(from);

    NS_LOG_INFO("SERVER got matching request from " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " queue size: " << userList.size() << " Time: " << (Simulator::Now()).GetSeconds());

    // IF USER WAITING IS MORE THAN 2
    if (userList.size() > 1)
    {
      Ipv4Address user1, user2;
      user1 = InetSocketAddress::ConvertFrom(userList.back()).GetIpv4();
      userList.pop_back();
      user2 = InetSocketAddress::ConvertFrom(userList.back()).GetIpv4();
      userList.pop_back();

      Ptr<Socket> user1_socket;
      Ptr<Socket> user2_socket;
      Ptr<Socket> temp;
      Address addr_checker;
      int counter = 0;
      for (auto it = m_socketList.begin(); it != m_socketList.end(); it++)
      {
        if (counter == 2)
          break;

        temp = *it;
        temp->GetPeerName(addr_checker);

        if (user1.IsEqual(InetSocketAddress::ConvertFrom(addr_checker).GetIpv4()))
        {
          user1_socket = temp;
          counter++;
        }
        else if (user2.IsEqual(InetSocketAddress::ConvertFrom(addr_checker).GetIpv4()))
        {
          user2_socket = temp;
          counter++;
        }
      }
      uint32_t addr_int;

      // SEND USER 1 INFO TO USER 2
      addr_int = user1.Get();
      memcpy(buffer, &addr_int, sizeof(uint32_t));
      buffer[4] = 0; // who wait;
      SendMessage(buffer, user2_socket);

      // SEND USER 2 INFO TO USER 1
      addr_int = user2.Get();
      memcpy(buffer, &addr_int, sizeof(uint32_t));
      buffer[4] = 1; // who try connect;
      SendMessage(buffer, user1_socket);
    }
  }

  void NewAppServer::HandleAccept(Ptr<Socket> s, const Address &from)
  {
    NS_LOG_FUNCTION(this << s << from);
    s->SetRecvCallback(MakeCallback(&NewAppServer::HandleRead, this));
    m_socketList.push_back(s);
  }

  void NewAppServer::HandlePeerError(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
  }

  void NewAppServer::HandlePeerClose(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
  }

  void NewAppServer::SendMessage(uint8_t *buffer, Ptr<Socket> socket)
  {
    Ptr<Packet> p = Create<Packet>(buffer, 128);
    int retval = socket->Send(p);
    if (retval >= 0)
      NS_LOG_INFO("SERVER respond matching info to client at " << (Simulator::Now()).GetSeconds());
    else
      NS_LOG_INFO("SERVER send fail, ERROR-Code: " << socket->GetErrno() << " Time: " << (Simulator::Now()).GetSeconds());

    socket->Close();
  }

} // Namespace ns3
