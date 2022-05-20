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
                                          MakeUintegerChecker<uint16_t>())
                            .AddAttribute("PacketWindowSize",
                                          "The size of the window used to compute the packet loss. This value should be a multiple of 8.",
                                          UintegerValue(32),
                                          MakeUintegerAccessor(&NewAppServer::GetPacketWindowSize,
                                                               &NewAppServer::SetPacketWindowSize),
                                          MakeUintegerChecker<uint16_t>(8, 256));
    return tid;
  }

  NewAppServer::NewAppServer()
      : m_lossCounter(0)
  {
    NS_LOG_FUNCTION(this);
  }

  NewAppServer::~NewAppServer()
  {
    NS_LOG_FUNCTION(this);
  }

  uint16_t
  NewAppServer::GetPacketWindowSize() const
  {
    NS_LOG_FUNCTION(this);
    return m_lossCounter.GetBitMapSize();
  }

  void
  NewAppServer::SetPacketWindowSize(uint16_t size)
  {
    NS_LOG_FUNCTION(this << size);
    m_lossCounter.SetBitMapSize(size);
  }

  uint32_t
  NewAppServer::GetLost(void) const
  {
    NS_LOG_FUNCTION(this);
    return m_lossCounter.GetLost();
  }

  void
  NewAppServer::DoDispose(void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
  }

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
    }
    m_socket->SetRecvCallback(MakeCallback(&NewAppServer::HandleRead, this));
  }

  void
  NewAppServer::StopApplication()
  {
    NS_LOG_FUNCTION(this);
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
      socket->GetSockName(localAddress);
      if (packet->GetSize() > 0)
      {
        SeqTsHeader seqTs;
        packet->RemoveHeader(seqTs);
        uint32_t currentSequenceNumber = seqTs.GetSeq();
        if (InetSocketAddress::IsMatchingType(from))
        {
          NS_LOG_INFO(
            "TraceDelay: RX " << packet->GetSize() 
            << " bytes from " << InetSocketAddress::ConvertFrom(from).GetIpv4() 
            << " Sequence Number: " << currentSequenceNumber 
            << " Uid: " << packet->GetUid() 
            << " TXtime: " << seqTs.GetTs() 
            << " RXtime: " << Simulator::Now() 
            << " Delay: " << Simulator::Now() - seqTs.GetTs()
          );
        }
        m_lossCounter.NotifyReceived(currentSequenceNumber);
      }
    }
  }

} // Namespace ns3
