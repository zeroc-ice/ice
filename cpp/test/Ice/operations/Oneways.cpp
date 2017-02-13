// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

void
oneways(const Ice::CommunicatorPtr&, const Test::MyClassPrx& proxy)
{
    Test::MyClassPrx p = Test::MyClassPrx::uncheckedCast(proxy->ice_oneway());
    
    {
        p->ice_ping();
    }

    {
        try
        {
            p->ice_isA("dummy");
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_id();
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_ids();
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }    

    {
        p->opVoid();
    }

    {
        p->opIdempotent();
    }

    {
        p->opNonmutating();
    }

    {
        Ice::Byte b;

        try
        {
            p->opByte(Ice::Byte(0xff), Ice::Byte(0x0f), b);
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

}
