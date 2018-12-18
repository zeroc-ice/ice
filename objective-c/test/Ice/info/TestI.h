// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
