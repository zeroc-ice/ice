//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

void
oneways(const Ice::CommunicatorPtr&, const Test::MyClassPrx& proxy)
{
    Test::MyClassPrx p = proxy->ice_oneway();

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
        uint8_t b;

        try
        {
            p->opByte(uint8_t(0xff), uint8_t(0x0f), b);
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

}
