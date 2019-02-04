//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
