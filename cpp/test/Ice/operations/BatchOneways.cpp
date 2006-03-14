// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
batchOneways(const Test::MyClassPrx& p)
{
    const Test::ByteS bs1(10  * 1024, 0);
    const Test::ByteS bs2(99  * 1024, 0);
    const Test::ByteS bs3(100  * 1024, 0);
    
    try
    {
	p->opByteSOneway(bs1);
	test(true);
    }
    catch(const Ice::MemoryLimitException&)
    {
	test(false);
    }

    try
    {
	p->opByteSOneway(bs2);
	test(true);
    }
    catch(const Ice::MemoryLimitException&)
    {
	test(false);
    }
    
    try
    {
	p->opByteSOneway(bs3);
	test(false);
    }
    catch(const Ice::MemoryLimitException&)
    {
	test(true);
    }
    
    Test::MyClassPrx batch = Test::MyClassPrx::uncheckedCast(p->ice_batchOneway());
    
    int i;

    for(i = 0 ; i < 9 ; ++i)
    {
	try
	{
	    batch->opByteSOneway(bs1);
	    test(true);
	}
	catch(const Ice::MemoryLimitException&)
	{
	    test(false);
	}
	
	batch->ice_connection()->flushBatchRequests();
    }
    
    for(i = 0 ; i < 10 ; ++i)
    {
	try
	{
	    batch->opByteSOneway(bs1);
	    test(i < 9);
	}
	catch(const Ice::MemoryLimitException&)
	{
	    test(i == 9);
	}
    }
    
    for(i = 0 ; i < 9 ; ++i)
    {
	try
	{
	    batch->opByteSOneway(bs1);
	    test(true);
	}
	catch(const Ice::MemoryLimitException&)
	{
	    test(false);
	}
	
	batch->ice_connection()->flushBatchRequests();
    }
}
