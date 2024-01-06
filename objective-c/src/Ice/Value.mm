//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <StreamI.h>
#import <Util.h>

#import <objc/Ice/LocalException.h>
#import <objc/Ice/Value.h>

@implementation ICEValue
-(id)init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    return self;
}
-(void) dealloc
{
    [super dealloc];
}
-(void) ice_preMarshal
{
}
-(void) ice_postUnmarshal
{
}
-(id<ICESlicedData>) ice_getSlicedData
{
    return nil;
}
-(NSString*) ice_id
{
    NSAssert(NO, @"ice_id requires override");
    return nil;
}
+(NSString*) ice_staticId
{
    return @"::Ice::Object";
}
-(void) iceWrite:(id<ICEOutputStream>)os
{
    [os startValue:nil];
    [self iceWriteImpl:os];
    [os endValue];
}
-(void) iceRead:(id<ICEInputStream>)is
{
    [is startValue];
    [self iceReadImpl:is];
    [is endValue:NO];
}
-(void) iceWriteImpl:(id<ICEOutputStream>)__unused os
{
    NSAssert(NO, @"iceWriteImpl requires override");
}
-(void) iceReadImpl:(id<ICEInputStream>)__unused is
{
    NSAssert(NO, @"iceReadImpl requires override");
}
-(id) copyWithZone:(NSZone*)zone
{
    return [[[self class] allocWithZone:zone] init];
}
@end
