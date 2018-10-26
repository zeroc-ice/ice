// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <ServantLocatorTest.h>
#import <servantLocator/ServantLocatorI.h>
#import <TestCommon.h>

@implementation TestServantLocatorI
-(id) init:(NSString*)category
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    _category = ICE_RETAIN(category);
    _deactivated = NO;
    _requestId = -1;
    return self;
}
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_category release];
    [super dealloc];
}
#endif
-(ICEObject*) locate:(ICECurrent*)current cookie:(id*)cookie
{
    test(!_deactivated);
    test([current.id_.category isEqual:_category] || [_category length] == 0);

    if([current.id_.name isEqual:@"unknown"])
    {
        return 0;
    }

    test([current.id_.name isEqual:@"locate"] || [current.id_.name isEqual:@"finished"]);
    if([current.id_.name isEqual:@"locate"])
    {
        [self exception:current];
    }

    //
    // Ensure locate() is only called once per request.
    //
    test(_requestId == -1);
    _requestId = current.requestId;

    return [self newServantAndCookie:cookie];
}
-(void) finished:(ICECurrent*)current servant:(ICEObject*)__unused servant cookie:(id)cookie
{
    test(!_deactivated);

    //
    // Ensure finished() is only called once per request.
    //
    test(_requestId == current.requestId);
    _requestId = -1;

    test([current.id_.category isEqual:_category]  || [_category length] == 0);
    test([current.id_.name isEqual:@"locate"] || [current.id_.name isEqual:@"finished"]);

    if([current.id_.name isEqual:@"finished"])
    {
        [self exception:current];
    }

    [self checkCookie:cookie];
}
-(void) deactivate:(NSString*)__unused category
{
    test(!_deactivated);

    _deactivated = YES;
}
-(ICEObject*) newServantAndCookie:(id*)__unused cookie
{
    NSAssert(NO, @"Subclasses need to overwrite this method");
    return nil; // To keep compiler happy
}
-(void) checkCookie:(id)__unused cookie
{
    NSAssert(NO, @"Subclasses need to overwrite this method");
}
-(void) throwTestIntfUserException
{
    NSAssert(NO, @"Subclasses need to overwrite this method");
}
-(void) exception:(ICECurrent*)current
{
    if([current.operation isEqual:@"ice_ids"])
    {
        @throw [TestServantLocatorTestIntfUserException testIntfUserException];
    }
    else if([current.operation isEqual:@"requestFailedException"])
    {
        @throw [ICEObjectNotExistException objectNotExistException:__FILE__ line:__LINE__];
    }
    else if([current.operation isEqual:@"unknownUserException"])
    {
        @throw [ICEUnknownUserException unknownUserException:__FILE__ line:__LINE__ unknown:@"reason"];
    }
    else if([current.operation isEqual:@"unknownLocalException"])
    {
        @throw [ICEUnknownLocalException unknownLocalException:__FILE__ line:__LINE__ unknown:@"reason"];
    }
    else if([current.operation isEqual:@"unknownException"])
    {
        @throw [ICEUnknownException unknownException:__FILE__ line:__LINE__ unknown:@"reason"];
    }
    else if([current.operation isEqual:@"userException"])
    {
        [self throwTestIntfUserException];
    }
    else if([current.operation isEqual:@"localException"])
    {
        @throw [ICESocketException socketException:__FILE__ line:__LINE__ error:0];
    }
    else if([current.operation isEqual:@"unknownExceptionWithServantException"])
    {
        @throw [ICEUnknownException unknownException:__FILE__ line:__LINE__ unknown:@"reason"];
    }
    else if([current.operation isEqual:@"impossibleException"])
    {
        @throw [TestServantLocatorTestIntfUserException testIntfUserException]; // Yes, it really is meant to be TestIntfUserException.
    }
    else if([current.operation isEqual:@"intfUserException"])
    {
        @throw [TestServantLocatorTestImpossibleException testImpossibleException]; // Yes, it really is meant to be TestImpossibleException.
    }
    else if([current.operation isEqual:@"asyncResponse"])
    {
        @throw [TestServantLocatorTestImpossibleException testImpossibleException];
    }
    else if([current.operation isEqual:@"asyncException"])
    {
        @throw [TestServantLocatorTestImpossibleException testImpossibleException];
    }
}
@end
