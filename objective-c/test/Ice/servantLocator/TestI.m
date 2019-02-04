//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <servantLocator/TestI.h>

@implementation TestServantLocatorTestIntfI
-(void) requestFailedException:(ICECurrent*)__unused current
{
}
-(void) unknownUserException:(ICECurrent*)__unused current
{
}
-(void) unknownLocalException:(ICECurrent*)__unused current
{
}
-(void) unknownException:(ICECurrent*)__unused current
{
}
-(void) userException:(ICECurrent*)__unused current
{
}
-(void) localException:(ICECurrent*)__unused current
{
}
-(void) stdException:(ICECurrent*)__unused current
{
}
-(void) cppException:(ICECurrent*)__unused current
{
}
-(void) unknownExceptionWithServantException:(ICECurrent*)__unused current
{
}
-(NSString*) impossibleException:(BOOL)throw current:(ICECurrent*)__unused current
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
-(NSString*) intfUserException:(BOOL)throw current:(ICECurrent*)__unused current
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
-(void) asyncResponse:(ICECurrent*)__unused current
{
}
-(void) asyncException:(ICECurrent*)__unused current
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
