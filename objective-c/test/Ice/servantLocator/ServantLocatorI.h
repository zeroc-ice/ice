// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>

#import <Foundation/Foundation.h>

@interface TestServantLocatorI : NSObject<ICEServantLocator>
{
    NSString* _category;
    BOOL _deactivated;
    int _requestId;
}
-(id) init:(NSString*)category;
-(ICEObject*) newServantAndCookie:(id*)cookie;
-(void) checkCookie:(id)cookie;
-(void) throwTestIntfUserException;
-(void) exception:(ICECurrent*)current;
@end
