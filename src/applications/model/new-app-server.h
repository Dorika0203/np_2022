#ifndef NEW_APP_SERVER_H
#define NEW_APP_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"
#include "packet-loss-counter.h"

namespace ns3
{
  class NewAppServer : public Application
  {
  public:
    static TypeId GetTypeId(void);
    NewAppServer();
    virtual ~NewAppServer();

  protected:
    virtual void DoDispose(void);

  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    // TCP Read Handler
    void HandleRead(Ptr<Socket> socket);

    // TCP Listen, Accept, Close Handler
    void HandleAccept(Ptr<Socket> socket, const Address& from);
    void HandlePeerClose(Ptr<Socket> socket);
    void HandlePeerError(Ptr<Socket> socket);

    // Sending Packet
    void SendPacket(Ptr<Packet> packet);

    uint16_t m_port;                 //!< Port on which we listen for incoming packets.
    Ptr<Socket> m_socket;            //!< IPv4 Socket
    std::list<Ptr<Socket>> m_socketList; //!< the accepted sockets
    // std::list<Ptr<Socket>> NewAppServer::GetAcceptedSockets(void) const;
  };

} // namespace ns3

#endif
