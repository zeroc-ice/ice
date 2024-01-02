//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `ConnectionInfo.ice'
#import <objc/Ice/LocalException.h>
#import <objc/Ice/Stream.h>
#import <objc/IceIAP/ConnectionInfo.h>
#import <objc/runtime.h>

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wshadow-ivar"
#endif

@implementation ICEInternalPrefixTable(C4A64A129_09C2_47C5_994A_AE6BFBE73A92)
-(void)addPrefixes_C4A64A129_09C2_47C5_994A_AE6BFBE73A92:(NSMutableDictionary*)prefixTable
{
    [prefixTable setObject:@"ICEIAP" forKey:@"::IceIAP"];
}
@end

@implementation ICEIAPConnectionInfo

@synthesize name;
@synthesize manufacturer;
@synthesize modelNumber;
@synthesize firmwareRevision;
@synthesize hardwareRevision;
@synthesize protocol;

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->name = @"";
    self->manufacturer = @"";
    self->modelNumber = @"";
    self->firmwareRevision = @"";
    self->hardwareRevision = @"";
    self->protocol = @"";
    return self;
}

-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId name:(NSString*)iceP_name manufacturer:(NSString*)iceP_manufacturer modelNumber:(NSString*)iceP_modelNumber firmwareRevision:(NSString*)iceP_firmwareRevision hardwareRevision:(NSString*)iceP_hardwareRevision protocol:(NSString*)iceP_protocol
{
    self = [super init:iceP_underlying incoming:iceP_incoming adapterName:iceP_adapterName connectionId:iceP_connectionId];
    if(!self)
    {
        return nil;
    }
    self->name = ICE_RETAIN(iceP_name);
    self->manufacturer = ICE_RETAIN(iceP_manufacturer);
    self->modelNumber = ICE_RETAIN(iceP_modelNumber);
    self->firmwareRevision = ICE_RETAIN(iceP_firmwareRevision);
    self->hardwareRevision = ICE_RETAIN(iceP_hardwareRevision);
    self->protocol = ICE_RETAIN(iceP_protocol);
    return self;
}

+(id) connectionInfo
{
    return ICE_AUTORELEASE([(ICEIAPConnectionInfo *)[self alloc] init]);
}

+(id) connectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId name:(NSString*)iceP_name manufacturer:(NSString*)iceP_manufacturer modelNumber:(NSString*)iceP_modelNumber firmwareRevision:(NSString*)iceP_firmwareRevision hardwareRevision:(NSString*)iceP_hardwareRevision protocol:(NSString*)iceP_protocol
{
    return ICE_AUTORELEASE([(ICEIAPConnectionInfo *)[self alloc] init:iceP_underlying incoming:iceP_incoming adapterName:iceP_adapterName connectionId:iceP_connectionId name:iceP_name manufacturer:iceP_manufacturer modelNumber:iceP_modelNumber firmwareRevision:iceP_firmwareRevision hardwareRevision:iceP_hardwareRevision protocol:iceP_protocol]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEIAPConnectionInfo *)[[self class] allocWithZone:zone] init:underlying incoming:incoming adapterName:adapterName connectionId:connectionId name:name manufacturer:manufacturer modelNumber:modelNumber firmwareRevision:firmwareRevision hardwareRevision:hardwareRevision protocol:protocol];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->name release];
    [self->manufacturer release];
    [self->modelNumber release];
    [self->firmwareRevision release];
    [self->hardwareRevision release];
    [self->protocol release];
    [super dealloc];
}
#endif
@end
