#ifndef NEW_APP_SERVER_H
#define NEW_APP_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"
#include "ns3/socket.h"

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
    void HandleAccept(Ptr<Socket> socket, const Address &from);
    void HandlePeerClose(Ptr<Socket> socket);
    void HandlePeerError(Ptr<Socket> socket);

    // Sending Packet
    void SendMessage(uint8_t *buffer, Ptr<Socket> socket);

    uint16_t m_port;
    Ptr<Socket> m_socket;
    std::list<Ptr<Socket>> m_socketList;
    std::list<Address> userList;
  };

} // namespace ns3

#endif
