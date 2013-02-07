/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef __TOCINO_ADDRESS_H__
#define __TOCINO_ADDRESS_H__

#include <stdint.h>

#include "ns3/mac48-address.h"

namespace ns3 {

class TocinoAddress
{
    public:

    TocinoAddress()
    {
        m_address.raw = 0;
    }

    explicit TocinoAddress( uint32_t a )
    {
        m_address.raw = a;
    }
    
    typedef uint8_t Coordinate;
    
    TocinoAddress( Coordinate x, Coordinate y, Coordinate z, uint8_t res = 0 )
    {
        m_address.x = x;
        m_address.y = y;
        m_address.z = z;
        m_address.reserved = res;
        m_address.multicast = 0;
    }

    Address ConvertTo() const
    {
        return Address( GetType(),
            reinterpret_cast<const uint8_t*>( &m_address.raw ),
            sizeof( m_address ) );
    }

    operator Address() const
    {
        return ConvertTo();
    }

    static TocinoAddress ConvertFrom( const Address &a )
    {
        TocinoAddress ta;
        NS_ASSERT( a.CheckCompatible( GetType(), sizeof( m_address ) ) );
        a.CopyTo( reinterpret_cast<uint8_t*>( &ta.m_address.raw ) );
        return ta;
    }

    bool IsMulticast()
    {
        return m_address.multicast;
    }

    void SetMulticast()
    {
        m_address.multicast = true; 
    }
    
    Mac48Address AsMac48Address() const
    {
        uint8_t buf[6];
       
        // Microsoft XCG OUI
        buf[5] = 0xDC;
        buf[4] = 0xB4;
        buf[3] = 0xC4;

        // embed XYZ in lower 3B
        buf[2] = m_address.x;
        buf[1] = m_address.y;
        buf[0] = m_address.z;

        Mac48Address a;
        a.CopyFrom( buf );

        return a;
    }
    
    Coordinate GetX() const
    {
        return m_address.x;
    }
    
    Coordinate GetY() const
    {
        return m_address.y;
    }
    
    Coordinate GetZ() const
    {
        return m_address.z;
    }
  
    // FIXME: TocinoAddress should probably
    // become a class template with this as
    // a non-type template parameter
    static const int DIM = 3;

    Coordinate GetCoordinate( int d ) const
    {
        NS_ASSERT( d >= 0 );
        NS_ASSERT( d < DIM );

        return m_address.coord[d];
    }

    private:

    static uint8_t GetType()
    {
        static uint8_t type = Address::Register();
        return type;
    }

    union
    {
        struct
        {
            union
            {
                struct
                {
                    Coordinate x, y, z;
                };
                Coordinate coord[DIM];
            };

            uint8_t reserved  : 7;
            uint8_t multicast : 1;
        };
        uint32_t raw;
    }
    m_address;
};

}

#endif /* __TOCINO_ADDRESS_H__ */

