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
oneways(const Ice::CommunicatorPtr&, const Test::MyClassPrxPtr& proxy)
{
    Test::MyClassPrxPtr p = ICE_UNCHECKED_CAST(Test::MyClassPrx, proxy->ice_oneway());
    
    {
        p->ice_ping();
    }

    {
        try
        {
            p->ice_isA("dummy");
            test(false);
        }
#ifdef ICE_CPP11_MAPPING
        catch(const IceUtil::IllegalArgumentException&)
#else
        catch(const Ice::TwowayOnlyException&)
#endif
        {
        }
    }

    {
        try
        {
            p->ice_id();
            test(false);
        }
#ifdef ICE_CPP11_MAPPING
        catch(const IceUtil::IllegalArgumentException&)
#else
        catch(const Ice::TwowayOnlyException&)
#endif
        {
        }
    }

    {
        try
        {
            p->ice_ids();
            test(false);
        }
#ifdef ICE_CPP11_MAPPING
        catch(const IceUtil::IllegalArgumentException&)
#else
        catch(const Ice::TwowayOnlyException&)
#endif
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
#ifdef ICE_CPP11_MAPPING
        catch(const IceUtil::IllegalArgumentException&)
#else
        catch(const Ice::TwowayOnlyException&)
#endif
        {
        }
    }

}
