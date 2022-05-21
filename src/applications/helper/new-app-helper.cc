#include "new-app-helper.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

namespace ns3
{

  NewAppServerHelper::NewAppServerHelper()
  {
    m_factory.SetTypeId(NewAppServer::GetTypeId());
  }

  NewAppServerHelper::NewAppServerHelper(uint16_t port)
  {
    m_factory.SetTypeId(NewAppServer::GetTypeId());
    SetAttribute("Port", UintegerValue(port));
  }

  void
  NewAppServerHelper::SetAttribute(std::string name, const AttributeValue &value)
  {
    m_factory.Set(name, value);
  }

  ApplicationContainer
  NewAppServerHelper::Install(NodeContainer c)
  {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
      Ptr<Node> node = *i;

      m_server = m_factory.Create<NewAppServer>();
      node->AddApplication(m_server);
      apps.Add(m_server);
    }
    return apps;
  }

  Ptr<NewAppServer>
  NewAppServerHelper::GetServer(void)
  {
    return m_server;
  }

  NewAppClientHelper::NewAppClientHelper()
  {
    m_factory.SetTypeId(NewAppClient::GetTypeId());
  }

  NewAppClientHelper::NewAppClientHelper(Address address, uint16_t port)
  {
    m_factory.SetTypeId(NewAppClient::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
    SetAttribute("RemotePort", UintegerValue(port));
  }

  NewAppClientHelper::NewAppClientHelper(Address address)
  {
    m_factory.SetTypeId(NewAppClient::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
  }

  void
  NewAppClientHelper::SetAttribute(std::string name, const AttributeValue &value)
  {
    m_factory.Set(name, value);
  }

  ApplicationContainer
  NewAppClientHelper::Install(NodeContainer c)
  {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NewAppClient> client = m_factory.Create<NewAppClient>();
      node->AddApplication(client);
      apps.Add(client);
    }
    return apps;
  }

} // namespace ns3
