// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <AdapterDeactivationTest.h>

@interface TestAdapterDeactivationI : TestAdapterDeactivationTestIntf<TestAdapterDeactivationTestIntf>
-(void) transient:(ICECurrent*)current;
-(void) deactivate:(ICECurrent*)current;
@end
