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

  protected:
    virtual void DoDispose(void);

  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void RequestServer(void);
    void SendMessageToFriend(const char* message);
    void ServerReceiveCallback(Ptr<Socket> socket);
    void FriendReceiveCallback(Ptr<Socket> socket);
    void FriendConnectGoodCallback(Ptr<Socket> socket);
    void FriendConnectBadCallback(Ptr<Socket> socket);

    EventId server_request_event;   //!< Event to send the next packet
    uint8_t scenario_type;

    Ptr<Socket> server_socket;  //!< Socket
    Address server_address; //!< Remote peer address
    uint16_t server_port;   //!< Remote peer port

    Ptr<Socket> friend_socket;
    Address friend_address;
    uint16_t friend_port;
    uint32_t message_size;

    Address my_address;
  };

} // namespace ns3

#endif
