/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef __TOCINO_DIMENSION_ORDER_ROUTER_H__
#define __TOCINO_DIMENSION_ORDER_ROUTER_H__

#include <vector>

#include "tocino-router.h"
#include "tocino-net-device.h"

namespace ns3
{

class TocinoDimensionOrderRouter : public TocinoRouter
{
    public:
    static TypeId GetTypeId( void );

    TocinoDimensionOrderRouter();

    void Initialize( Ptr<TocinoNetDevice>, const TocinoRx* );

    uint32_t Route( Ptr<const Packet> p );

    private:

    bool ShouldRoutePositive( const uint32_t src, const uint32_t dst ) const;

    Ptr<TocinoNetDevice> m_tnd;
    const TocinoRx *m_trx;

    // wrap-around support for rings/tori
    static const int32_t NO_WRAP = -1;
    int32_t m_wrapPoint;

    std::vector<uint32_t> m_currentRoutes;
};

}

#endif //__TOCINO_DIMENSION_ORDER_ROUTER_H__
