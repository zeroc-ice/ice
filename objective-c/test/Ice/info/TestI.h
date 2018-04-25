// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <InfoTest.h>

@interface TestInfoTestIntfI : TestInfoTestIntf<TestInfoTestIntf>
{
}

-(void) shutdown:(ICECurrent*)current;
-(ICEContext*) getEndpointInfoAsContext:(ICECurrent*)current;
-(ICEContext*) getConnectionInfoAsContext:(ICECurrent*)current;
@end
