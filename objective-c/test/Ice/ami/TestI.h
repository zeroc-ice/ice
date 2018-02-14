// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    NSCondition* _cond;
}
@end

@interface TestAMITestIntfControllerI : TestAMITestIntfController<TestAMITestIntfController>
{
    id<ICEObjectAdapter> _adapter;
}
-(id) initWithAdapter:(id<ICEObjectAdapter>)adapter;
@end

