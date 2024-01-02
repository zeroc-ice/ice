//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `EndpointInfo.ice'
#import <objc/Ice/LocalException.h>
#import <objc/Ice/Stream.h>
#import <objc/IceIAP/EndpointInfo.h>
#import <objc/runtime.h>

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wshadow-ivar"
#endif

@implementation ICEInternalPrefixTable(C27EA20F4_929A_4198_9B23_EC4A2A20BAB3)
-(void)addPrefixes_C27EA20F4_929A_4198_9B23_EC4A2A20BAB3:(NSMutableDictionary*)prefixTable
{
    [prefixTable setObject:@"ICEIAP" forKey:@"::IceIAP"];
}
@end

@implementation ICEIAPEndpointInfo

@synthesize manufacturer;
@synthesize modelNumber;
@synthesize name;
@synthesize protocol;

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->manufacturer = @"";
    self->modelNumber = @"";
    self->name = @"";
    self->protocol = @"";
    return self;
}

-(id) init:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress manufacturer:(NSString*)iceP_manufacturer modelNumber:(NSString*)iceP_modelNumber name:(NSString*)iceP_name protocol:(NSString*)iceP_protocol
{
    self = [super init:iceP_underlying timeout:iceP_timeout compress:iceP_compress];
    if(!self)
    {
        return nil;
    }
    self->manufacturer = ICE_RETAIN(iceP_manufacturer);
    self->modelNumber = ICE_RETAIN(iceP_modelNumber);
    self->name = ICE_RETAIN(iceP_name);
    self->protocol = ICE_RETAIN(iceP_protocol);
    return self;
}

+(id) endpointInfo
{
    return ICE_AUTORELEASE([(ICEIAPEndpointInfo *)[self alloc] init]);
}

+(id) endpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress manufacturer:(NSString*)iceP_manufacturer modelNumber:(NSString*)iceP_modelNumber name:(NSString*)iceP_name protocol:(NSString*)iceP_protocol
{
    return ICE_AUTORELEASE([(ICEIAPEndpointInfo *)[self alloc] init:iceP_underlying timeout:iceP_timeout compress:iceP_compress manufacturer:iceP_manufacturer modelNumber:iceP_modelNumber name:iceP_name protocol:iceP_protocol]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEIAPEndpointInfo *)[[self class] allocWithZone:zone] init:underlying timeout:timeout compress:compress manufacturer:manufacturer modelNumber:modelNumber name:name protocol:protocol];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->manufacturer release];
    [self->modelNumber release];
    [self->name release];
    [self->protocol release];
    [super dealloc];
}
#endif
@end
