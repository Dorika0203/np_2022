#ifndef NEW_APP_CLIENT_H
#define NEW_APP_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"

namespace ns3
{

  class Socket;
  class Packet;

  class NewAppClient : public Application
  {
  public:
    static TypeId GetTypeId(void);

    NewAppClient();

    virtual ~NewAppClient();

    void SetRemote(Address ip, uint16_t port);
    void SetRemote(Address addr);

  protected:
    virtual void DoDispose(void);

  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void Send(void);

    uint32_t m_count; //!< Maximum number of packets the application will send
    Time m_interval;  //!< Packet inter-send time
    uint32_t m_size;  //!< Size of the sent packet (including the SeqTsHeader)

    uint32_t m_sent;       //!< Counter for sent packets
    Ptr<Socket> m_socket;  //!< Socket
    Address m_peerAddress; //!< Remote peer address
    uint16_t m_peerPort;   //!< Remote peer port
    EventId m_sendEvent;   //!< Event to send the next packet
  };

} // namespace ns3

#endif
