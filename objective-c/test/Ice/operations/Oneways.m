//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <TestCommon.h>
#import <OperationsTest.h>

void
oneways(id<ICECommunicator>__unused communicator, id<TestOperationsMyClassPrx> proxy)
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
