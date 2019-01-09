// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <AdapterDeactivationTest.h>

@interface TestAdapterDeactivationI : TestAdapterDeactivationTestIntf<TestAdapterDeactivationTestIntf>
-(void) transient:(ICECurrent*)current;
-(void) deactivate:(ICECurrent*)current;
@end
