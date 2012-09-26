/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"
#include "ns3/data-rate.h"
#include "ns3/uinteger.h"

#include "tocino-channel.h"
#include "tocino-queue.h"
#include "tocino-net-device-receiver.h"
#include "tocino-net-device-transmitter.h"
#include "tocino-net-device.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TocinoNetDevice);

TypeId TocinoNetDevice::GetTypeId(void)
{
  static TypeId tid = TypeId( "ns3::TocinoNetDevice" )
    .SetParent<NetDevice>()
    .AddConstructor<TocinoNetDevice>()
     .AddAttribute ("Channels", 
                    "Number of channels connected to net device.",
                    UintegerValue (6),
                    MakeUintegerAccessor (&TocinoNetDevice::m_nChannels),
                    MakeUintegerChecker<uint32_t> ());
  return tid;
}

TocinoNetDevice::TocinoNetDevice() :
    m_node( 0 ),
    m_ifIndex( 0 ),
    m_mtu( DEFAULT_MTU )
{
  uint32_t i, j;

  m_nChannels = 6; // placeholder
  m_nPorts = m_nChannels + 1;

  // create queues - right now 1 per s/d pair
  for (i = 0; i < (m_nPorts * m_nPorts); i++)
    {
      m_queues[i] = CreateObject<TocinoQueue>();
    }

  // create receivers and transmitters
  for (i = 0; i < m_nChannels; i++)
    {
      m_receivers[i] = CreateObject<TocinoNetDeviceReceiver> ();
      m_receivers[i]->m_tnd = this;
      m_receivers[i]->m_channelNumber = i;

      m_transmitters[i] = CreateObject<TocinoNetDeviceTransmitter> ();
      m_transmitters[i]->m_tnd = this;
      m_transmitters[i]->m_channelNumber = i;
    }

  // build linkage between tx, rx, and q for channel interfaces
  for (i = 0; i < m_nChannels; i++)
    {
      for (j = 0; j < m_nChannels; j++)
        {
          m_receivers[i]->m_queues[j] = m_queues[(i * m_nPorts) + j];
          m_transmitters[i]->m_queues[j] = m_queues[i + (j * m_nPorts)];
        }
    }

  // build linkage between injection/ejection port
  // injection/ejection port is always port# m_nChannels
  NS_LOG_UNCOND("missing connection to injection port");
  for (i = 0; i < m_nChannels; i++)
    {
      m_receivers[i]->m_queues[m_nChannels] = m_queues[(i * m_nPorts) + m_nChannels];
      m_transmitters[i]->m_queues[m_nChannels] = m_queues[i + (m_nChannels * m_nPorts)];
    }
}

void TocinoNetDevice::SetIfIndex( const uint32_t index )
{
    m_ifIndex = index;
}

uint32_t TocinoNetDevice::GetIfIndex( void ) const
{ 
    return m_ifIndex;
}

Ptr<Channel> TocinoNetDevice::GetChannel( void ) const
{
    return 0;
}

bool TocinoNetDevice::SetMtu( const uint16_t mtu )
{
    m_mtu = mtu;
    return true;
}

uint16_t TocinoNetDevice::GetMtu( void ) const
{
    return m_mtu;
}

void TocinoNetDevice::SetAddress( Address address )
{
    m_address = TocinoAddress::ConvertFrom( address );
}

Address TocinoNetDevice::GetAddress( void ) const
{
    return m_address;
}

bool TocinoNetDevice::IsLinkUp( void ) const
{
    return true;
}

void TocinoNetDevice::AddLinkChangeCallback( Callback<void> callback )
{
    //Do nothing for now 
}

bool TocinoNetDevice::IsBroadcast( void ) const
{
    return true;
}

Address TocinoNetDevice::GetBroadcast( void ) const
{
    return TocinoAddress::ConvertFrom( Mac48Address ("ff:ff:ff:ff:ff:ff") );
}

bool TocinoNetDevice::IsMulticast( void ) const
{
    return true;
}

Address TocinoNetDevice::GetMulticast( Ipv4Address a ) const
{
    return TocinoAddress::ConvertFrom( Mac48Address::GetMulticast( a ) );
}

Address TocinoNetDevice::GetMulticast( Ipv6Address a ) const
{
    return TocinoAddress::ConvertFrom( Mac48Address::GetMulticast( a ) );
}

bool TocinoNetDevice::IsPointToPoint( void ) const
{
    return false;
}

bool TocinoNetDevice::IsBridge( void ) const
{
    return false;
}

bool TocinoNetDevice::Send( Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber )
{
    return SendFrom( packet, m_address, dest, protocolNumber );
}

bool TocinoNetDevice::SendFrom( Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber )
{
  // eventually call InjectPacket to hand packet to fabric
    // TODO
    // TODO
    // TODO
    return false;
    // TODO
    // TODO
    // TODO
}

Ptr<Node>
TocinoNetDevice::GetNode( void ) const {return m_node;}

void
TocinoNetDevice::SetNode( Ptr<Node> node ) {m_node = node;}

bool
TocinoNetDevice::NeedsArp( void ) const {return true;}

void
TocinoNetDevice::SetReceiveCallback( NetDevice::ReceiveCallback cb )
{
    m_rxCallback = cb;
}

void
TocinoNetDevice::SetPromiscReceiveCallback( PromiscReceiveCallback cb )
{
    m_promiscRxCallback = cb;
}

bool TocinoNetDevice::SupportsSendFrom( void ) const
{
    return true;
}

void
TocinoNetDevice::SetTxChannel(Ptr<TocinoChannel> c, uint32_t port)
{
  m_transmitters[port]->SetChannel(c);
}

bool
TocinoNetDevice::InjectPacket(Ptr<Packet> p)
{
  uint32_t iport = m_nPorts-1;
 
  if (m_receivers[iport]->IsBlocked()) return false;
  m_receivers[m_nPorts-1]->Receive(p);
  return true;
}

bool
TocinoNetDevice::EjectPacket(Ptr<Packet>)
{
  // marksan to write this
  return true;
}

void
TocinoNetDevice::SetRxChannel(Ptr<TocinoChannel> c, uint32_t port)
{
  // not sure we care about this - linkage is TocinoChannel invoking Receive
  //m_receivers[port]->SetChannel(c);
}

} // namespace ns3

