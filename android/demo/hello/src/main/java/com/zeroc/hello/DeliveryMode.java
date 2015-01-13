// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package com.zeroc.hello;

enum DeliveryMode
{
    TWOWAY,
    TWOWAY_SECURE,
    ONEWAY,
    ONEWAY_BATCH,
    ONEWAY_SECURE,
    ONEWAY_SECURE_BATCH,
    DATAGRAM,
    DATAGRAM_BATCH;

    Ice.ObjectPrx apply(Ice.ObjectPrx prx)
    {
        switch (this)
        {
        case TWOWAY:
            prx = prx.ice_twoway();
            break;
        case TWOWAY_SECURE:
            prx = prx.ice_twoway().ice_secure(true);
            break;
        case ONEWAY:
            prx = prx.ice_oneway();
            break;
        case ONEWAY_BATCH:
            prx = prx.ice_batchOneway();
            break;
        case ONEWAY_SECURE:
            prx = prx.ice_oneway().ice_secure(true);
            break;
        case ONEWAY_SECURE_BATCH:
            prx = prx.ice_batchOneway().ice_secure(true);
            break;
        case DATAGRAM:
            prx = prx.ice_datagram();
            break;
        case DATAGRAM_BATCH:
            prx = prx.ice_batchDatagram();
            break;
        }
        return prx;
    }

    public boolean isBatch()
    {
        return this == ONEWAY_BATCH || this == DATAGRAM_BATCH || this == ONEWAY_SECURE_BATCH;
    }
}
