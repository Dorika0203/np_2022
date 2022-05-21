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
#include "packet-loss-counter.h"

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

  // std::list<Ptr<Socket>>
  // NewAppServer::GetAcceptedSockets(void) const
  // {
  //   NS_LOG_FUNCTION(this);
  //   return m_socketList;
  // }


  void
  NewAppServer::StartApplication(void)
  {
    NS_LOG_FUNCTION(this);

    if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
      m_socket = Socket::CreateSocket(GetNode(), tid);
      InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
      if (m_socket->Bind(local) == -1)
      {
        NS_FATAL_ERROR("Failed to bind socket");
      }
      m_socket->Listen();
      m_socket->ShutdownSend();
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
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from)))
    {
      if (packet->GetSize() == 0)
        break;
      socket->GetSockName(localAddress);
      SeqTsHeader seqTs;
      packet->RemoveHeader(seqTs);
      uint32_t currentSequenceNumber = seqTs.GetSeq();

      if (InetSocketAddress::IsMatchingType(from))
      {
        NS_LOG_INFO("At time " << Simulator::Now().GetSeconds()
                               << "s SERVER received "
                               << packet->GetSize() << " bytes from "
                               << InetSocketAddress::ConvertFrom(from).GetIpv4()
                               << " port " << InetSocketAddress::ConvertFrom(from).GetPort()
                               << " seqNum " << currentSequenceNumber
                               );
      }
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

} // Namespace ns3
