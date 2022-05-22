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
#include "ns3/uinteger.h"
#include "ns3/double.h"
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
                                          MakeAddressAccessor(&NewAppClient::m_peerAddress),
                                          MakeAddressChecker())
                            .AddAttribute("RemotePort", "The destination port of the outbound packets",
                                          UintegerValue(100),
                                          MakeUintegerAccessor(&NewAppClient::m_peerPort),
                                          MakeUintegerChecker<uint16_t>())
                            .AddAttribute("PacketSize",
                                          "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                                          UintegerValue(128),
                                          MakeUintegerAccessor(&NewAppClient::m_size),
                                          MakeUintegerChecker<uint32_t>(12, 65507));
    return tid;
  }

  NewAppClient::NewAppClient()
  {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
    m_sendEvent = EventId();
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
      else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true)
      {
        if (m_socket->Bind() == -1)
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

    // m_socket->SetConnectCallback(MakeCallback(&NewAppClient::ConnectionSucceededCallback, this),
    //                              MakeCallback(&NewAppClient::ConnectionFailedCallback, this));
    // m_socket->SetCloseCallbacks(MakeCallback(&NewAppClient::NormalCloseCallback, this),
    //                             MakeCallback(&NewAppClient::ErrorCloseCallback, this));
    m_socket->SetRecvCallback(MakeCallback(&NewAppClient::ReceivedDataCallback, this));
    // m_socket->SetAttribute("MaxSegLifetime", DoubleValue(111.111));
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &NewAppClient::RequestServer, this);
  }

  void
  NewAppClient::StopApplication(void)
  {
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_sendEvent);
  }

  void
  NewAppClient::RequestServer(void)
  {
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_sendEvent.IsExpired());

    uint8_t buffer[128];
    buffer[0] = 123;
    Ptr<Packet> p = Create<Packet>(buffer, m_size);

    std::stringstream peerAddressStringStream;
    if (Ipv4Address::IsMatchingType(m_peerAddress))
    {
      peerAddressStringStream << Ipv4Address::ConvertFrom(m_peerAddress);
    }

    if ((m_socket->Send(p)) >= 0)
    {
      NS_LOG_INFO("CLIENT requested matching to " << peerAddressStringStream.str() << " Time: " << (Simulator::Now()).GetSeconds());
    }
    else
    {
      NS_LOG_INFO("Error while sending " << m_size << " bytes to " << peerAddressStringStream.str());
    }
  }



























  void
  NewAppClient::ConnectionSucceededCallback(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
  }

  void
  NewAppClient::ConnectionFailedCallback(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
  }

  void
  NewAppClient::NormalCloseCallback(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
  }

  void
  NewAppClient::ErrorCloseCallback(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
  }

  void
  NewAppClient::ReceivedDataCallback(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet = socket->Recv();
    uint8_t buffer[128];
    packet->CopyData(buffer, 128);
    NS_LOG_INFO((int)buffer[0]);
  }
} // Namespace ns3
