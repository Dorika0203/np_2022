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
    uint32_t GetLost(void) const;
    uint64_t GetReceived(void) const;
    uint16_t GetPacketWindowSize() const;
    void SetPacketWindowSize(uint16_t size);

  protected:
    virtual void DoDispose(void);

  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void HandleRead(Ptr<Socket> socket);
    void SendPacket(Ptr<Packet> packet);

    uint16_t m_port;                 //!< Port on which we listen for incoming packets.
    Ptr<Socket> m_socket;            //!< IPv4 Socket
    PacketLossCounter m_lossCounter; //!< Lost packet counter
  };

} // namespace ns3

#endif
