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
    void RequestServer(void);
    void ConnectionSucceededCallback (Ptr<Socket> socket);
    void ConnectionFailedCallback (Ptr<Socket> socket);
    void NormalCloseCallback (Ptr<Socket> socket);
    void ErrorCloseCallback (Ptr<Socket> socket);
    void ReceivedDataCallback (Ptr<Socket> socket);

    uint32_t m_size;  //!< Size of the sent packet (including the SeqTsHeader)

    Ptr<Socket> m_socket;  //!< Socket
    Address m_peerAddress; //!< Remote peer address
    uint16_t m_peerPort;   //!< Remote peer port
    EventId m_sendEvent;   //!< Event to send the next packet
  };

} // namespace ns3

#endif
