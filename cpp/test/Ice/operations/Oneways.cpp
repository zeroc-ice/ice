// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

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
            [[maybe_unused]] bool _ = p->ice_isA("dummy");
            test(false);
        }
        catch (const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            [[maybe_unused]] string _ = p->ice_id();
            test(false);
        }
        catch (const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            [[maybe_unused]] vector<string> _ = p->ice_ids();
            test(false);
        }
        catch (const Ice::TwowayOnlyException&)
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
        uint8_t b;

        try
        {
            p->opByte(uint8_t(0xff), uint8_t(0x0f), b);
            test(false);
        }
        catch (const Ice::TwowayOnlyException&)
        {
        }
    }
}
