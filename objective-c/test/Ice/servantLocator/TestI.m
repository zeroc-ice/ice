// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <servantLocator/TestI.h>

@implementation TestServantLocatorTestIntfI
-(void) requestFailedException:(ICECurrent*)current
{
}
-(void) unknownUserException:(ICECurrent*)current
{
}
-(void) unknownLocalException:(ICECurrent*)current
{
}
-(void) unknownException:(ICECurrent*)current
{
}
-(void) userException:(ICECurrent*)current
{
}
-(void) localException:(ICECurrent*)current
{
}
-(void) stdException:(ICECurrent*)current
{
}
-(void) cppException:(ICECurrent*)current
{
}
-(void) csException:(ICECurrent*)current
{
}
-(void) unknownExceptionWithServantException:(ICECurrent*)current
{
}
-(NSString*) impossibleException:(BOOL)throw current:(ICECurrent*)current
{
    if(throw)
    {
        @throw [TestServantLocatorTestImpossibleException testImpossibleException];
    }
    //
    // Return a value so we can be sure that the stream position
    // is reset correctly if finished() throws.
    //
    return @"Hello";
}
-(NSString*) intfUserException:(BOOL)throw current:(ICECurrent*)current
{
    if(throw)
    {
        @throw [TestServantLocatorTestIntfUserException testIntfUserException];
    }
    //
    // Return a value so we can be sure that the stream position
    // is reset correctly if finished() throws.
    //
    return @"Hello";
}
-(void) asyncResponse:(ICECurrent*)current
{
}
-(void) asyncException:(ICECurrent*)current
{
}
-(void) shutdown:(ICECurrent*)current
{
    [current.adapter deactivate];
}
@end

@implementation TestServantLocatorCookieI
-(NSString*) message
{
    return @"blahblah";
}
@end
