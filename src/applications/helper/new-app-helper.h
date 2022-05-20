#ifndef NEW_APP_HELPER_H
#define NEW_APP_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/new-app-server.h"
#include "ns3/new-app-client.h"
namespace ns3 {

class NewAppServerHelper
{
public:

  NewAppServerHelper ();
  NewAppServerHelper (uint16_t port);
  void SetAttribute (std::string name, const AttributeValue &value);
  ApplicationContainer Install (NodeContainer c);
  Ptr<NewAppServer> GetServer (void);
private:
  ObjectFactory m_factory; //!< Object factory.
  Ptr<NewAppServer> m_server; //!< The last created server application
};

class NewAppClientHelper
{
public:

  NewAppClientHelper ();
  NewAppClientHelper (Address ip, uint16_t port);
  NewAppClientHelper (Address addr);
  void SetAttribute (std::string name, const AttributeValue &value);
  ApplicationContainer Install (NodeContainer c);

private:
  ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif
