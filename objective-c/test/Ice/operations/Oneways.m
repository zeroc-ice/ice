// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <OperationsTest.h>

void
oneways(id<ICECommunicator> communicator, id<TestOperationsMyClassPrx> proxy)
{
    id<TestOperationsMyClassPrx> p = [TestOperationsMyClassPrx uncheckedCast:[proxy ice_oneway]];

    {
        [p opVoid];
    }

    {
        ICEByte b;
        ICEByte r;

        @try
        {
            r = [p opByte:(ICEByte)0xff p2:(ICEByte)0x0f p3:&b];
            test(NO);
        }
        @catch(ICETwowayOnlyException*)
        {
        }
    }

}
