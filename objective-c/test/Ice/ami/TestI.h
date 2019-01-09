// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <AMITest.h>
#import <Foundation/Foundation.h>

//
// Servant implementation
//
@interface TestAMITestIntfI : TestAMITestIntf<TestAMITestIntf>
{
    int _batchCount;
    BOOL _dispatching;
    NSCondition* _cond;
}
@end

//
// Servant implementation
//
@interface TestAMITestOuterInnerTestIntfI : TestAMITestOuterInnerTestIntf<TestAMITestOuterInnerTestIntf>
{
}
@end

@interface TestAMITestIntfControllerI : TestAMITestIntfController<TestAMITestIntfController>
{
    id<ICEObjectAdapter> _adapter;
}
-(id) initWithAdapter:(id<ICEObjectAdapter>)adapter;
@end
