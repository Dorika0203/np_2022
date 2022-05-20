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
#include "new-app-client.h"
#include "seq-ts-header.h"
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
                            .AddAttribute("MaxPackets",
                                          "The maximum number of packets the application will send",
                                          UintegerValue(100),
                                          MakeUintegerAccessor(&NewAppClient::m_count),
                                          MakeUintegerChecker<uint32_t>())
                            .AddAttribute("Interval",
                                          "The time to wait between packets", TimeValue(Seconds(1.0)),
                                          MakeTimeAccessor(&NewAppClient::m_interval),
                                          MakeTimeChecker())
                            .AddAttribute("RemoteAddress",
                                          "The destination Address of the outbound packets",
                                          AddressValue(),
                                          MakeAddressAccessor(&NewAppClient::m_peerAddress),
                                          MakeAddressChecker())
                            .AddAttribute("RemotePort", "The destination port of the outbound packets",
                                          UintegerValue(100),
                                          MakeUintegerAccessor(&NewAppClient::m_peerPort),
                                          MakeUintegerChecker<uint16_t>())
                            .AddAttribute("PacketSize",
                                          "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                                          UintegerValue(1024),
                                          MakeUintegerAccessor(&NewAppClient::m_size),
                                          MakeUintegerChecker<uint32_t>(12, 65507));
    return tid;
  }

  NewAppClient::NewAppClient()
  {
    NS_LOG_FUNCTION(this);
    m_sent = 0;
    m_socket = 0;
    m_sendEvent = EventId();
  }

  NewAppClient::~NewAppClient()
  {
    NS_LOG_FUNCTION(this);
  }

  void
  NewAppClient::SetRemote(Address ip, uint16_t port)
  {
    NS_LOG_FUNCTION(this << ip << port);
    m_peerAddress = ip;
    m_peerPort = port;
  }

  void
  NewAppClient::SetRemote(Address addr)
  {
    NS_LOG_FUNCTION(this << addr);
    m_peerAddress = addr;
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

    if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
      m_socket = Socket::CreateSocket(GetNode(), tid);
      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
      {
        if (m_socket->Bind() == -1)
        {
          NS_FATAL_ERROR("Failed to bind socket");
        }
        m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
      }
      else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
      {
        if (m_socket->Bind6() == -1)
        {
          NS_FATAL_ERROR("Failed to bind socket");
        }
        m_socket->Connect(Inet6SocketAddress(Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
      }
      else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true)
      {
        if (m_socket->Bind() == -1)
        {
          NS_FATAL_ERROR("Failed to bind socket");
        }
        m_socket->Connect(m_peerAddress);
      }
      else if (Inet6SocketAddress::IsMatchingType(m_peerAddress) == true)
      {
        if (m_socket->Bind6() == -1)
        {
          NS_FATAL_ERROR("Failed to bind socket");
        }
        m_socket->Connect(m_peerAddress);
      }
      else
      {
        NS_ASSERT_MSG(false, "Incompatible address type: " << m_peerAddress);
      }
    }

    m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    m_socket->SetAllowBroadcast(true);
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &NewAppClient::Send, this);
  }

  void
  NewAppClient::StopApplication(void)
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_sendEvent);
  }

  void
  NewAppClient::Send(void)
  {
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_sendEvent.IsExpired());
    SeqTsHeader seqTs;
    seqTs.SetSeq(m_sent);
    Ptr<Packet> p = Create<Packet>(m_size - (8 + 4)); // 8+4 : the size of the seqTs header
    p->AddHeader(seqTs);

    std::stringstream peerAddressStringStream;
    if (Ipv4Address::IsMatchingType(m_peerAddress))
    {
      peerAddressStringStream << Ipv4Address::ConvertFrom(m_peerAddress);
    }
    else if (Ipv6Address::IsMatchingType(m_peerAddress))
    {
      peerAddressStringStream << Ipv6Address::ConvertFrom(m_peerAddress);
    }

    if ((m_socket->Send(p)) >= 0)
    {
      ++m_sent;
      NS_LOG_INFO("TraceDelay TX " << m_size << " bytes to "
                                   << peerAddressStringStream.str() << " Uid: "
                                   << p->GetUid() << " Time: "
                                   << (Simulator::Now()).GetSeconds());
    }
    else
    {
      NS_LOG_INFO("Error while sending " << m_size << " bytes to "
                                         << peerAddressStringStream.str());
    }

    if (m_sent < m_count)
    {
      m_sendEvent = Simulator::Schedule(m_interval, &NewAppClient::Send, this);
    }
  }

} // Namespace ns3
