//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `LocalException.ice'
#import <objc/Ice/LocalException.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalException.h>
#import <objc/runtime.h>

#ifndef ICE_API_EXPORTS
#   define ICE_API_EXPORTS
#endif

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wshadow-ivar"
#endif

@implementation ICEInternalPrefixTable(C5FAFF4D0_DE68_4963_A50D_1851E8858BD8)
-(void)addPrefixes_C5FAFF4D0_DE68_4963_A50D_1851E8858BD8:(NSMutableDictionary*)prefixTable
{
    [prefixTable setObject:@"ICE" forKey:@"::Ice"];
}
@end

@implementation ICEInitializationException

@synthesize reason_;

-(NSString *) ice_id
{
    return @"::Ice::InitializationException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->reason_ = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->reason_ = ICE_RETAIN(iceP_reason);
    return self;
}

+(id) initializationException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEInitializationException *)[self alloc] init:file_ line:line_]);
}

+(id) initializationException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEInitializationException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEInitializationException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->reason_ release];
    [super dealloc];
}
#endif
@end

@implementation ICEPluginInitializationException

@synthesize reason_;

-(NSString *) ice_id
{
    return @"::Ice::PluginInitializationException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->reason_ = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->reason_ = ICE_RETAIN(iceP_reason);
    return self;
}

+(id) pluginInitializationException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEPluginInitializationException *)[self alloc] init:file_ line:line_]);
}

+(id) pluginInitializationException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEPluginInitializationException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEPluginInitializationException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->reason_ release];
    [super dealloc];
}
#endif
@end

@implementation ICECollocationOptimizationException

-(NSString *) ice_id
{
    return @"::Ice::CollocationOptimizationException";
}

+(id) collocationOptimizationException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICECollocationOptimizationException *)[self alloc] init:file_ line:line_]);
}
@end

@implementation ICEAlreadyRegisteredException

@synthesize kindOfObject;
@synthesize id_;

-(NSString *) ice_id
{
    return @"::Ice::AlreadyRegisteredException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->kindOfObject = @"";
    self->id_ = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ kindOfObject:(NSString*)iceP_kindOfObject id:(NSString*)iceP_id
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->kindOfObject = ICE_RETAIN(iceP_kindOfObject);
    self->id_ = ICE_RETAIN(iceP_id);
    return self;
}

+(id) alreadyRegisteredException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEAlreadyRegisteredException *)[self alloc] init:file_ line:line_]);
}

+(id) alreadyRegisteredException:(const char*)file_ line:(int)line_ kindOfObject:(NSString*)iceP_kindOfObject id:(NSString*)iceP_id
{
    return ICE_AUTORELEASE([(ICEAlreadyRegisteredException *)[self alloc] init:file_ line:line_ kindOfObject:iceP_kindOfObject id:iceP_id]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEAlreadyRegisteredException *)[[self class] allocWithZone:zone] init:file line:line kindOfObject:kindOfObject id:id_];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->kindOfObject release];
    [self->id_ release];
    [super dealloc];
}
#endif
@end

@implementation ICENotRegisteredException

@synthesize kindOfObject;
@synthesize id_;

-(NSString *) ice_id
{
    return @"::Ice::NotRegisteredException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->kindOfObject = @"";
    self->id_ = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ kindOfObject:(NSString*)iceP_kindOfObject id:(NSString*)iceP_id
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->kindOfObject = ICE_RETAIN(iceP_kindOfObject);
    self->id_ = ICE_RETAIN(iceP_id);
    return self;
}

+(id) notRegisteredException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICENotRegisteredException *)[self alloc] init:file_ line:line_]);
}

+(id) notRegisteredException:(const char*)file_ line:(int)line_ kindOfObject:(NSString*)iceP_kindOfObject id:(NSString*)iceP_id
{
    return ICE_AUTORELEASE([(ICENotRegisteredException *)[self alloc] init:file_ line:line_ kindOfObject:iceP_kindOfObject id:iceP_id]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICENotRegisteredException *)[[self class] allocWithZone:zone] init:file line:line kindOfObject:kindOfObject id:id_];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->kindOfObject release];
    [self->id_ release];
    [super dealloc];
}
#endif
@end

@implementation ICETwowayOnlyException

@synthesize operation;

-(NSString *) ice_id
{
    return @"::Ice::TwowayOnlyException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->operation = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ operation:(NSString*)iceP_operation
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->operation = ICE_RETAIN(iceP_operation);
    return self;
}

+(id) twowayOnlyException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICETwowayOnlyException *)[self alloc] init:file_ line:line_]);
}

+(id) twowayOnlyException:(const char*)file_ line:(int)line_ operation:(NSString*)iceP_operation
{
    return ICE_AUTORELEASE([(ICETwowayOnlyException *)[self alloc] init:file_ line:line_ operation:iceP_operation]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICETwowayOnlyException *)[[self class] allocWithZone:zone] init:file line:line operation:operation];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->operation release];
    [super dealloc];
}
#endif
@end

@implementation ICECloneNotImplementedException

-(NSString *) ice_id
{
    return @"::Ice::CloneNotImplementedException";
}

+(id) cloneNotImplementedException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICECloneNotImplementedException *)[self alloc] init:file_ line:line_]);
}
@end

@implementation ICEUnknownException

@synthesize unknown;

-(NSString *) ice_id
{
    return @"::Ice::UnknownException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->unknown = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ unknown:(NSString*)iceP_unknown
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->unknown = ICE_RETAIN(iceP_unknown);
    return self;
}

+(id) unknownException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEUnknownException *)[self alloc] init:file_ line:line_]);
}

+(id) unknownException:(const char*)file_ line:(int)line_ unknown:(NSString*)iceP_unknown
{
    return ICE_AUTORELEASE([(ICEUnknownException *)[self alloc] init:file_ line:line_ unknown:iceP_unknown]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEUnknownException *)[[self class] allocWithZone:zone] init:file line:line unknown:unknown];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->unknown release];
    [super dealloc];
}
#endif
@end

@implementation ICEUnknownLocalException

-(NSString *) ice_id
{
    return @"::Ice::UnknownLocalException";
}

+(id) unknownLocalException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEUnknownLocalException *)[self alloc] init:file_ line:line_]);
}

+(id) unknownLocalException:(const char*)file_ line:(int)line_ unknown:(NSString*)iceP_unknown
{
    return ICE_AUTORELEASE([(ICEUnknownLocalException *)[self alloc] init:file_ line:line_ unknown:iceP_unknown]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEUnknownLocalException *)[[self class] allocWithZone:zone] init:file line:line unknown:unknown];
}
@end

@implementation ICEUnknownUserException

-(NSString *) ice_id
{
    return @"::Ice::UnknownUserException";
}

+(id) unknownUserException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEUnknownUserException *)[self alloc] init:file_ line:line_]);
}

+(id) unknownUserException:(const char*)file_ line:(int)line_ unknown:(NSString*)iceP_unknown
{
    return ICE_AUTORELEASE([(ICEUnknownUserException *)[self alloc] init:file_ line:line_ unknown:iceP_unknown]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEUnknownUserException *)[[self class] allocWithZone:zone] init:file line:line unknown:unknown];
}
@end

@implementation ICEVersionMismatchException

-(NSString *) ice_id
{
    return @"::Ice::VersionMismatchException";
}

+(id) versionMismatchException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEVersionMismatchException *)[self alloc] init:file_ line:line_]);
}
@end

@implementation ICECommunicatorDestroyedException

-(NSString *) ice_id
{
    return @"::Ice::CommunicatorDestroyedException";
}

+(id) communicatorDestroyedException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICECommunicatorDestroyedException *)[self alloc] init:file_ line:line_]);
}
@end

@implementation ICEObjectAdapterDeactivatedException

@synthesize name_;

-(NSString *) ice_id
{
    return @"::Ice::ObjectAdapterDeactivatedException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->name_ = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ name:(NSString*)iceP_name
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->name_ = ICE_RETAIN(iceP_name);
    return self;
}

+(id) objectAdapterDeactivatedException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEObjectAdapterDeactivatedException *)[self alloc] init:file_ line:line_]);
}

+(id) objectAdapterDeactivatedException:(const char*)file_ line:(int)line_ name:(NSString*)iceP_name
{
    return ICE_AUTORELEASE([(ICEObjectAdapterDeactivatedException *)[self alloc] init:file_ line:line_ name:iceP_name]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEObjectAdapterDeactivatedException *)[[self class] allocWithZone:zone] init:file line:line name:name_];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->name_ release];
    [super dealloc];
}
#endif
@end

@implementation ICEObjectAdapterIdInUseException

@synthesize id_;

-(NSString *) ice_id
{
    return @"::Ice::ObjectAdapterIdInUseException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->id_ = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ id:(NSString*)iceP_id
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->id_ = ICE_RETAIN(iceP_id);
    return self;
}

+(id) objectAdapterIdInUseException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEObjectAdapterIdInUseException *)[self alloc] init:file_ line:line_]);
}

+(id) objectAdapterIdInUseException:(const char*)file_ line:(int)line_ id:(NSString*)iceP_id
{
    return ICE_AUTORELEASE([(ICEObjectAdapterIdInUseException *)[self alloc] init:file_ line:line_ id:iceP_id]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEObjectAdapterIdInUseException *)[[self class] allocWithZone:zone] init:file line:line id:id_];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->id_ release];
    [super dealloc];
}
#endif
@end

@implementation ICENoEndpointException

@synthesize proxy;

-(NSString *) ice_id
{
    return @"::Ice::NoEndpointException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->proxy = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ proxy:(NSString*)iceP_proxy
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->proxy = ICE_RETAIN(iceP_proxy);
    return self;
}

+(id) noEndpointException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICENoEndpointException *)[self alloc] init:file_ line:line_]);
}

+(id) noEndpointException:(const char*)file_ line:(int)line_ proxy:(NSString*)iceP_proxy
{
    return ICE_AUTORELEASE([(ICENoEndpointException *)[self alloc] init:file_ line:line_ proxy:iceP_proxy]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICENoEndpointException *)[[self class] allocWithZone:zone] init:file line:line proxy:proxy];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->proxy release];
    [super dealloc];
}
#endif
@end

@implementation ICEEndpointParseException

@synthesize str;

-(NSString *) ice_id
{
    return @"::Ice::EndpointParseException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->str = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ str:(NSString*)iceP_str
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->str = ICE_RETAIN(iceP_str);
    return self;
}

+(id) endpointParseException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEEndpointParseException *)[self alloc] init:file_ line:line_]);
}

+(id) endpointParseException:(const char*)file_ line:(int)line_ str:(NSString*)iceP_str
{
    return ICE_AUTORELEASE([(ICEEndpointParseException *)[self alloc] init:file_ line:line_ str:iceP_str]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEEndpointParseException *)[[self class] allocWithZone:zone] init:file line:line str:str];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->str release];
    [super dealloc];
}
#endif
@end

@implementation ICEEndpointSelectionTypeParseException

@synthesize str;

-(NSString *) ice_id
{
    return @"::Ice::EndpointSelectionTypeParseException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->str = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ str:(NSString*)iceP_str
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->str = ICE_RETAIN(iceP_str);
    return self;
}

+(id) endpointSelectionTypeParseException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEEndpointSelectionTypeParseException *)[self alloc] init:file_ line:line_]);
}

+(id) endpointSelectionTypeParseException:(const char*)file_ line:(int)line_ str:(NSString*)iceP_str
{
    return ICE_AUTORELEASE([(ICEEndpointSelectionTypeParseException *)[self alloc] init:file_ line:line_ str:iceP_str]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEEndpointSelectionTypeParseException *)[[self class] allocWithZone:zone] init:file line:line str:str];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->str release];
    [super dealloc];
}
#endif
@end

@implementation ICEVersionParseException

@synthesize str;

-(NSString *) ice_id
{
    return @"::Ice::VersionParseException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->str = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ str:(NSString*)iceP_str
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->str = ICE_RETAIN(iceP_str);
    return self;
}

+(id) versionParseException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEVersionParseException *)[self alloc] init:file_ line:line_]);
}

+(id) versionParseException:(const char*)file_ line:(int)line_ str:(NSString*)iceP_str
{
    return ICE_AUTORELEASE([(ICEVersionParseException *)[self alloc] init:file_ line:line_ str:iceP_str]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEVersionParseException *)[[self class] allocWithZone:zone] init:file line:line str:str];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->str release];
    [super dealloc];
}
#endif
@end

@implementation ICEIdentityParseException

@synthesize str;

-(NSString *) ice_id
{
    return @"::Ice::IdentityParseException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->str = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ str:(NSString*)iceP_str
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->str = ICE_RETAIN(iceP_str);
    return self;
}

+(id) identityParseException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEIdentityParseException *)[self alloc] init:file_ line:line_]);
}

+(id) identityParseException:(const char*)file_ line:(int)line_ str:(NSString*)iceP_str
{
    return ICE_AUTORELEASE([(ICEIdentityParseException *)[self alloc] init:file_ line:line_ str:iceP_str]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEIdentityParseException *)[[self class] allocWithZone:zone] init:file line:line str:str];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->str release];
    [super dealloc];
}
#endif
@end

@implementation ICEProxyParseException

@synthesize str;

-(NSString *) ice_id
{
    return @"::Ice::ProxyParseException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->str = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ str:(NSString*)iceP_str
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->str = ICE_RETAIN(iceP_str);
    return self;
}

+(id) proxyParseException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEProxyParseException *)[self alloc] init:file_ line:line_]);
}

+(id) proxyParseException:(const char*)file_ line:(int)line_ str:(NSString*)iceP_str
{
    return ICE_AUTORELEASE([(ICEProxyParseException *)[self alloc] init:file_ line:line_ str:iceP_str]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEProxyParseException *)[[self class] allocWithZone:zone] init:file line:line str:str];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->str release];
    [super dealloc];
}
#endif
@end

@implementation ICEIllegalIdentityException

@synthesize id_;

-(NSString *) ice_id
{
    return @"::Ice::IllegalIdentityException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->id_ = [[ICEIdentity alloc] init];
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ id:(ICEIdentity*)iceP_id
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->id_ = ICE_RETAIN(iceP_id);
    return self;
}

+(id) illegalIdentityException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEIllegalIdentityException *)[self alloc] init:file_ line:line_]);
}

+(id) illegalIdentityException:(const char*)file_ line:(int)line_ id:(ICEIdentity*)iceP_id
{
    return ICE_AUTORELEASE([(ICEIllegalIdentityException *)[self alloc] init:file_ line:line_ id:iceP_id]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEIllegalIdentityException *)[[self class] allocWithZone:zone] init:file line:line id:id_];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->id_ release];
    [super dealloc];
}
#endif
@end

@implementation ICEIllegalServantException

@synthesize reason_;

-(NSString *) ice_id
{
    return @"::Ice::IllegalServantException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->reason_ = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->reason_ = ICE_RETAIN(iceP_reason);
    return self;
}

+(id) illegalServantException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEIllegalServantException *)[self alloc] init:file_ line:line_]);
}

+(id) illegalServantException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEIllegalServantException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEIllegalServantException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->reason_ release];
    [super dealloc];
}
#endif
@end

@implementation ICERequestFailedException

@synthesize id_;
@synthesize facet;
@synthesize operation;

-(NSString *) ice_id
{
    return @"::Ice::RequestFailedException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->id_ = [[ICEIdentity alloc] init];
    self->facet = @"";
    self->operation = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ id:(ICEIdentity*)iceP_id facet:(NSString*)iceP_facet operation:(NSString*)iceP_operation
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->id_ = ICE_RETAIN(iceP_id);
    self->facet = ICE_RETAIN(iceP_facet);
    self->operation = ICE_RETAIN(iceP_operation);
    return self;
}

+(id) requestFailedException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICERequestFailedException *)[self alloc] init:file_ line:line_]);
}

+(id) requestFailedException:(const char*)file_ line:(int)line_ id:(ICEIdentity*)iceP_id facet:(NSString*)iceP_facet operation:(NSString*)iceP_operation
{
    return ICE_AUTORELEASE([(ICERequestFailedException *)[self alloc] init:file_ line:line_ id:iceP_id facet:iceP_facet operation:iceP_operation]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICERequestFailedException *)[[self class] allocWithZone:zone] init:file line:line id:id_ facet:facet operation:operation];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->id_ release];
    [self->facet release];
    [self->operation release];
    [super dealloc];
}
#endif
@end

@implementation ICEObjectNotExistException

-(NSString *) ice_id
{
    return @"::Ice::ObjectNotExistException";
}

+(id) objectNotExistException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEObjectNotExistException *)[self alloc] init:file_ line:line_]);
}

+(id) objectNotExistException:(const char*)file_ line:(int)line_ id:(ICEIdentity*)iceP_id facet:(NSString*)iceP_facet operation:(NSString*)iceP_operation
{
    return ICE_AUTORELEASE([(ICEObjectNotExistException *)[self alloc] init:file_ line:line_ id:iceP_id facet:iceP_facet operation:iceP_operation]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEObjectNotExistException *)[[self class] allocWithZone:zone] init:file line:line id:id_ facet:facet operation:operation];
}
@end

@implementation ICEFacetNotExistException

-(NSString *) ice_id
{
    return @"::Ice::FacetNotExistException";
}

+(id) facetNotExistException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEFacetNotExistException *)[self alloc] init:file_ line:line_]);
}

+(id) facetNotExistException:(const char*)file_ line:(int)line_ id:(ICEIdentity*)iceP_id facet:(NSString*)iceP_facet operation:(NSString*)iceP_operation
{
    return ICE_AUTORELEASE([(ICEFacetNotExistException *)[self alloc] init:file_ line:line_ id:iceP_id facet:iceP_facet operation:iceP_operation]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEFacetNotExistException *)[[self class] allocWithZone:zone] init:file line:line id:id_ facet:facet operation:operation];
}
@end

@implementation ICEOperationNotExistException

-(NSString *) ice_id
{
    return @"::Ice::OperationNotExistException";
}

+(id) operationNotExistException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEOperationNotExistException *)[self alloc] init:file_ line:line_]);
}

+(id) operationNotExistException:(const char*)file_ line:(int)line_ id:(ICEIdentity*)iceP_id facet:(NSString*)iceP_facet operation:(NSString*)iceP_operation
{
    return ICE_AUTORELEASE([(ICEOperationNotExistException *)[self alloc] init:file_ line:line_ id:iceP_id facet:iceP_facet operation:iceP_operation]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEOperationNotExistException *)[[self class] allocWithZone:zone] init:file line:line id:id_ facet:facet operation:operation];
}
@end

@implementation ICESyscallException

@synthesize error;

-(NSString *) ice_id
{
    return @"::Ice::SyscallException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->error = 0;
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->error = iceP_error;
    return self;
}

+(id) syscallException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICESyscallException *)[self alloc] init:file_ line:line_]);
}

+(id) syscallException:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error
{
    return ICE_AUTORELEASE([(ICESyscallException *)[self alloc] init:file_ line:line_ error:iceP_error]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICESyscallException *)[[self class] allocWithZone:zone] init:file line:line error:error];
}
@end

@implementation ICESocketException

-(NSString *) ice_id
{
    return @"::Ice::SocketException";
}

+(id) socketException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICESocketException *)[self alloc] init:file_ line:line_]);
}

+(id) socketException:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error
{
    return ICE_AUTORELEASE([(ICESocketException *)[self alloc] init:file_ line:line_ error:iceP_error]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICESocketException *)[[self class] allocWithZone:zone] init:file line:line error:error];
}
@end

@implementation ICECFNetworkException

@synthesize domain;

-(NSString *) ice_id
{
    return @"::Ice::CFNetworkException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->domain = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error domain:(NSString*)iceP_domain
{
    self = [super init:file_ line:line_ error:iceP_error];
    if(!self)
    {
        return nil;
    }
    self->domain = ICE_RETAIN(iceP_domain);
    return self;
}

+(id) cfNetworkException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICECFNetworkException *)[self alloc] init:file_ line:line_]);
}

+(id) cFNetworkException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([self cfNetworkException:file_ line:line_]);
}

+(id) cfNetworkException:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error domain:(NSString*)iceP_domain
{
    return ICE_AUTORELEASE([(ICECFNetworkException *)[self alloc] init:file_ line:line_ error:iceP_error domain:iceP_domain]);
}

+(id) cFNetworkException:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error domain:(NSString*)iceP_domain
{
    return ICE_AUTORELEASE([self cfNetworkException:file_ line:line_ error:iceP_error domain:iceP_domain]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICECFNetworkException *)[[self class] allocWithZone:zone] init:file line:line error:error domain:domain];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->domain release];
    [super dealloc];
}
#endif
@end

@implementation ICEFileException

@synthesize path;

-(NSString *) ice_id
{
    return @"::Ice::FileException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->path = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error path:(NSString*)iceP_path
{
    self = [super init:file_ line:line_ error:iceP_error];
    if(!self)
    {
        return nil;
    }
    self->path = ICE_RETAIN(iceP_path);
    return self;
}

+(id) fileException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEFileException *)[self alloc] init:file_ line:line_]);
}

+(id) fileException:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error path:(NSString*)iceP_path
{
    return ICE_AUTORELEASE([(ICEFileException *)[self alloc] init:file_ line:line_ error:iceP_error path:iceP_path]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEFileException *)[[self class] allocWithZone:zone] init:file line:line error:error path:path];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->path release];
    [super dealloc];
}
#endif
@end

@implementation ICEConnectFailedException

-(NSString *) ice_id
{
    return @"::Ice::ConnectFailedException";
}

+(id) connectFailedException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEConnectFailedException *)[self alloc] init:file_ line:line_]);
}

+(id) connectFailedException:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error
{
    return ICE_AUTORELEASE([(ICEConnectFailedException *)[self alloc] init:file_ line:line_ error:iceP_error]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEConnectFailedException *)[[self class] allocWithZone:zone] init:file line:line error:error];
}
@end

@implementation ICEConnectionRefusedException

-(NSString *) ice_id
{
    return @"::Ice::ConnectionRefusedException";
}

+(id) connectionRefusedException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEConnectionRefusedException *)[self alloc] init:file_ line:line_]);
}

+(id) connectionRefusedException:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error
{
    return ICE_AUTORELEASE([(ICEConnectionRefusedException *)[self alloc] init:file_ line:line_ error:iceP_error]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEConnectionRefusedException *)[[self class] allocWithZone:zone] init:file line:line error:error];
}
@end

@implementation ICEConnectionLostException

-(NSString *) ice_id
{
    return @"::Ice::ConnectionLostException";
}

+(id) connectionLostException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEConnectionLostException *)[self alloc] init:file_ line:line_]);
}

+(id) connectionLostException:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error
{
    return ICE_AUTORELEASE([(ICEConnectionLostException *)[self alloc] init:file_ line:line_ error:iceP_error]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEConnectionLostException *)[[self class] allocWithZone:zone] init:file line:line error:error];
}
@end

@implementation ICEDNSException

@synthesize error;
@synthesize host;

-(NSString *) ice_id
{
    return @"::Ice::DNSException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->error = 0;
    self->host = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error host:(NSString*)iceP_host
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->error = iceP_error;
    self->host = ICE_RETAIN(iceP_host);
    return self;
}

+(id) dnsException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEDNSException *)[self alloc] init:file_ line:line_]);
}

+(id) dNSException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([self dnsException:file_ line:line_]);
}

+(id) dnsException:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error host:(NSString*)iceP_host
{
    return ICE_AUTORELEASE([(ICEDNSException *)[self alloc] init:file_ line:line_ error:iceP_error host:iceP_host]);
}

+(id) dNSException:(const char*)file_ line:(int)line_ error:(ICEInt)iceP_error host:(NSString*)iceP_host
{
    return ICE_AUTORELEASE([self dnsException:file_ line:line_ error:iceP_error host:iceP_host]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEDNSException *)[[self class] allocWithZone:zone] init:file line:line error:error host:host];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->host release];
    [super dealloc];
}
#endif
@end

@implementation ICEOperationInterruptedException

-(NSString *) ice_id
{
    return @"::Ice::OperationInterruptedException";
}

+(id) operationInterruptedException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEOperationInterruptedException *)[self alloc] init:file_ line:line_]);
}
@end

@implementation ICETimeoutException

-(NSString *) ice_id
{
    return @"::Ice::TimeoutException";
}

+(id) timeoutException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICETimeoutException *)[self alloc] init:file_ line:line_]);
}
@end

@implementation ICEConnectTimeoutException

-(NSString *) ice_id
{
    return @"::Ice::ConnectTimeoutException";
}

+(id) connectTimeoutException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEConnectTimeoutException *)[self alloc] init:file_ line:line_]);
}
@end

@implementation ICECloseTimeoutException

-(NSString *) ice_id
{
    return @"::Ice::CloseTimeoutException";
}

+(id) closeTimeoutException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICECloseTimeoutException *)[self alloc] init:file_ line:line_]);
}
@end

@implementation ICEConnectionTimeoutException

-(NSString *) ice_id
{
    return @"::Ice::ConnectionTimeoutException";
}

+(id) connectionTimeoutException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEConnectionTimeoutException *)[self alloc] init:file_ line:line_]);
}
@end

@implementation ICEInvocationTimeoutException

-(NSString *) ice_id
{
    return @"::Ice::InvocationTimeoutException";
}

+(id) invocationTimeoutException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEInvocationTimeoutException *)[self alloc] init:file_ line:line_]);
}
@end

@implementation ICEInvocationCanceledException

-(NSString *) ice_id
{
    return @"::Ice::InvocationCanceledException";
}

+(id) invocationCanceledException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEInvocationCanceledException *)[self alloc] init:file_ line:line_]);
}
@end

@implementation ICEProtocolException

@synthesize reason_;

-(NSString *) ice_id
{
    return @"::Ice::ProtocolException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->reason_ = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->reason_ = ICE_RETAIN(iceP_reason);
    return self;
}

+(id) protocolException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEProtocolException *)[self alloc] init:file_ line:line_]);
}

+(id) protocolException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEProtocolException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEProtocolException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->reason_ release];
    [super dealloc];
}
#endif
@end

@implementation ICEBadMagicException

@synthesize badMagic;

-(NSString *) ice_id
{
    return @"::Ice::BadMagicException";
}

-(id) init:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason badMagic:(ICEByteSeq*)iceP_badMagic
{
    self = [super init:file_ line:line_ reason:iceP_reason];
    if(!self)
    {
        return nil;
    }
    self->badMagic = ICE_RETAIN(iceP_badMagic);
    return self;
}

+(id) badMagicException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEBadMagicException *)[self alloc] init:file_ line:line_]);
}

+(id) badMagicException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason badMagic:(ICEByteSeq*)iceP_badMagic
{
    return ICE_AUTORELEASE([(ICEBadMagicException *)[self alloc] init:file_ line:line_ reason:iceP_reason badMagic:iceP_badMagic]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEBadMagicException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_ badMagic:badMagic];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->badMagic release];
    [super dealloc];
}
#endif
@end

@implementation ICEUnsupportedProtocolException

@synthesize bad;
@synthesize supported;

-(NSString *) ice_id
{
    return @"::Ice::UnsupportedProtocolException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->bad = [[ICEProtocolVersion alloc] init];
    self->supported = [[ICEProtocolVersion alloc] init];
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason bad:(ICEProtocolVersion*)iceP_bad supported:(ICEProtocolVersion*)iceP_supported
{
    self = [super init:file_ line:line_ reason:iceP_reason];
    if(!self)
    {
        return nil;
    }
    self->bad = ICE_RETAIN(iceP_bad);
    self->supported = ICE_RETAIN(iceP_supported);
    return self;
}

+(id) unsupportedProtocolException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEUnsupportedProtocolException *)[self alloc] init:file_ line:line_]);
}

+(id) unsupportedProtocolException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason bad:(ICEProtocolVersion*)iceP_bad supported:(ICEProtocolVersion*)iceP_supported
{
    return ICE_AUTORELEASE([(ICEUnsupportedProtocolException *)[self alloc] init:file_ line:line_ reason:iceP_reason bad:iceP_bad supported:iceP_supported]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEUnsupportedProtocolException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_ bad:bad supported:supported];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->bad release];
    [self->supported release];
    [super dealloc];
}
#endif
@end

@implementation ICEUnsupportedEncodingException

@synthesize bad;
@synthesize supported;

-(NSString *) ice_id
{
    return @"::Ice::UnsupportedEncodingException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->bad = [[ICEEncodingVersion alloc] init];
    self->supported = [[ICEEncodingVersion alloc] init];
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason bad:(ICEEncodingVersion*)iceP_bad supported:(ICEEncodingVersion*)iceP_supported
{
    self = [super init:file_ line:line_ reason:iceP_reason];
    if(!self)
    {
        return nil;
    }
    self->bad = ICE_RETAIN(iceP_bad);
    self->supported = ICE_RETAIN(iceP_supported);
    return self;
}

+(id) unsupportedEncodingException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEUnsupportedEncodingException *)[self alloc] init:file_ line:line_]);
}

+(id) unsupportedEncodingException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason bad:(ICEEncodingVersion*)iceP_bad supported:(ICEEncodingVersion*)iceP_supported
{
    return ICE_AUTORELEASE([(ICEUnsupportedEncodingException *)[self alloc] init:file_ line:line_ reason:iceP_reason bad:iceP_bad supported:iceP_supported]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEUnsupportedEncodingException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_ bad:bad supported:supported];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->bad release];
    [self->supported release];
    [super dealloc];
}
#endif
@end

@implementation ICEUnknownMessageException

-(NSString *) ice_id
{
    return @"::Ice::UnknownMessageException";
}

+(id) unknownMessageException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEUnknownMessageException *)[self alloc] init:file_ line:line_]);
}

+(id) unknownMessageException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEUnknownMessageException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEUnknownMessageException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICEConnectionNotValidatedException

-(NSString *) ice_id
{
    return @"::Ice::ConnectionNotValidatedException";
}

+(id) connectionNotValidatedException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEConnectionNotValidatedException *)[self alloc] init:file_ line:line_]);
}

+(id) connectionNotValidatedException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEConnectionNotValidatedException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEConnectionNotValidatedException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICEUnknownRequestIdException

-(NSString *) ice_id
{
    return @"::Ice::UnknownRequestIdException";
}

+(id) unknownRequestIdException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEUnknownRequestIdException *)[self alloc] init:file_ line:line_]);
}

+(id) unknownRequestIdException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEUnknownRequestIdException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEUnknownRequestIdException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICEUnknownReplyStatusException

-(NSString *) ice_id
{
    return @"::Ice::UnknownReplyStatusException";
}

+(id) unknownReplyStatusException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEUnknownReplyStatusException *)[self alloc] init:file_ line:line_]);
}

+(id) unknownReplyStatusException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEUnknownReplyStatusException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEUnknownReplyStatusException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICECloseConnectionException

-(NSString *) ice_id
{
    return @"::Ice::CloseConnectionException";
}

+(id) closeConnectionException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICECloseConnectionException *)[self alloc] init:file_ line:line_]);
}

+(id) closeConnectionException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICECloseConnectionException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICECloseConnectionException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICEConnectionManuallyClosedException

@synthesize graceful;

-(NSString *) ice_id
{
    return @"::Ice::ConnectionManuallyClosedException";
}

-(id) init:(const char*)file_ line:(int)line_ graceful:(BOOL)iceP_graceful
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->graceful = iceP_graceful;
    return self;
}

+(id) connectionManuallyClosedException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEConnectionManuallyClosedException *)[self alloc] init:file_ line:line_]);
}

+(id) connectionManuallyClosedException:(const char*)file_ line:(int)line_ graceful:(BOOL)iceP_graceful
{
    return ICE_AUTORELEASE([(ICEConnectionManuallyClosedException *)[self alloc] init:file_ line:line_ graceful:iceP_graceful]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEConnectionManuallyClosedException *)[[self class] allocWithZone:zone] init:file line:line graceful:graceful];
}
@end

@implementation ICEIllegalMessageSizeException

-(NSString *) ice_id
{
    return @"::Ice::IllegalMessageSizeException";
}

+(id) illegalMessageSizeException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEIllegalMessageSizeException *)[self alloc] init:file_ line:line_]);
}

+(id) illegalMessageSizeException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEIllegalMessageSizeException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEIllegalMessageSizeException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICECompressionException

-(NSString *) ice_id
{
    return @"::Ice::CompressionException";
}

+(id) compressionException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICECompressionException *)[self alloc] init:file_ line:line_]);
}

+(id) compressionException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICECompressionException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICECompressionException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICEDatagramLimitException

-(NSString *) ice_id
{
    return @"::Ice::DatagramLimitException";
}

+(id) datagramLimitException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEDatagramLimitException *)[self alloc] init:file_ line:line_]);
}

+(id) datagramLimitException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEDatagramLimitException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEDatagramLimitException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICEMarshalException

-(NSString *) ice_id
{
    return @"::Ice::MarshalException";
}

+(id) marshalException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEMarshalException *)[self alloc] init:file_ line:line_]);
}

+(id) marshalException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEMarshalException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEMarshalException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICEProxyUnmarshalException

-(NSString *) ice_id
{
    return @"::Ice::ProxyUnmarshalException";
}

+(id) proxyUnmarshalException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEProxyUnmarshalException *)[self alloc] init:file_ line:line_]);
}

+(id) proxyUnmarshalException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEProxyUnmarshalException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEProxyUnmarshalException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICEUnmarshalOutOfBoundsException

-(NSString *) ice_id
{
    return @"::Ice::UnmarshalOutOfBoundsException";
}

+(id) unmarshalOutOfBoundsException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEUnmarshalOutOfBoundsException *)[self alloc] init:file_ line:line_]);
}

+(id) unmarshalOutOfBoundsException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEUnmarshalOutOfBoundsException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEUnmarshalOutOfBoundsException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICENoValueFactoryException

@synthesize type;

-(NSString *) ice_id
{
    return @"::Ice::NoValueFactoryException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->type = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason type:(NSString*)iceP_type
{
    self = [super init:file_ line:line_ reason:iceP_reason];
    if(!self)
    {
        return nil;
    }
    self->type = ICE_RETAIN(iceP_type);
    return self;
}

+(id) noValueFactoryException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICENoValueFactoryException *)[self alloc] init:file_ line:line_]);
}

+(id) noValueFactoryException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason type:(NSString*)iceP_type
{
    return ICE_AUTORELEASE([(ICENoValueFactoryException *)[self alloc] init:file_ line:line_ reason:iceP_reason type:iceP_type]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICENoValueFactoryException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_ type:type];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->type release];
    [super dealloc];
}
#endif
@end

@implementation ICEUnexpectedObjectException

@synthesize type;
@synthesize expectedType;

-(NSString *) ice_id
{
    return @"::Ice::UnexpectedObjectException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->type = @"";
    self->expectedType = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason type:(NSString*)iceP_type expectedType:(NSString*)iceP_expectedType
{
    self = [super init:file_ line:line_ reason:iceP_reason];
    if(!self)
    {
        return nil;
    }
    self->type = ICE_RETAIN(iceP_type);
    self->expectedType = ICE_RETAIN(iceP_expectedType);
    return self;
}

+(id) unexpectedObjectException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEUnexpectedObjectException *)[self alloc] init:file_ line:line_]);
}

+(id) unexpectedObjectException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason type:(NSString*)iceP_type expectedType:(NSString*)iceP_expectedType
{
    return ICE_AUTORELEASE([(ICEUnexpectedObjectException *)[self alloc] init:file_ line:line_ reason:iceP_reason type:iceP_type expectedType:iceP_expectedType]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEUnexpectedObjectException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_ type:type expectedType:expectedType];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->type release];
    [self->expectedType release];
    [super dealloc];
}
#endif
@end

@implementation ICEMemoryLimitException

-(NSString *) ice_id
{
    return @"::Ice::MemoryLimitException";
}

+(id) memoryLimitException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEMemoryLimitException *)[self alloc] init:file_ line:line_]);
}

+(id) memoryLimitException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEMemoryLimitException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEMemoryLimitException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICEStringConversionException

-(NSString *) ice_id
{
    return @"::Ice::StringConversionException";
}

+(id) stringConversionException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEStringConversionException *)[self alloc] init:file_ line:line_]);
}

+(id) stringConversionException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEStringConversionException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEStringConversionException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICEEncapsulationException

-(NSString *) ice_id
{
    return @"::Ice::EncapsulationException";
}

+(id) encapsulationException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEEncapsulationException *)[self alloc] init:file_ line:line_]);
}

+(id) encapsulationException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICEEncapsulationException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEEncapsulationException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}
@end

@implementation ICEFeatureNotSupportedException

@synthesize unsupportedFeature;

-(NSString *) ice_id
{
    return @"::Ice::FeatureNotSupportedException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->unsupportedFeature = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ unsupportedFeature:(NSString*)iceP_unsupportedFeature
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->unsupportedFeature = ICE_RETAIN(iceP_unsupportedFeature);
    return self;
}

+(id) featureNotSupportedException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEFeatureNotSupportedException *)[self alloc] init:file_ line:line_]);
}

+(id) featureNotSupportedException:(const char*)file_ line:(int)line_ unsupportedFeature:(NSString*)iceP_unsupportedFeature
{
    return ICE_AUTORELEASE([(ICEFeatureNotSupportedException *)[self alloc] init:file_ line:line_ unsupportedFeature:iceP_unsupportedFeature]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEFeatureNotSupportedException *)[[self class] allocWithZone:zone] init:file line:line unsupportedFeature:unsupportedFeature];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->unsupportedFeature release];
    [super dealloc];
}
#endif
@end

@implementation ICESecurityException

@synthesize reason_;

-(NSString *) ice_id
{
    return @"::Ice::SecurityException";
}

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->reason_ = @"";
    return self;
}

-(id) init:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    self = [super init:file_ line:line_];
    if(!self)
    {
        return nil;
    }
    self->reason_ = ICE_RETAIN(iceP_reason);
    return self;
}

+(id) securityException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICESecurityException *)[self alloc] init:file_ line:line_]);
}

+(id) securityException:(const char*)file_ line:(int)line_ reason:(NSString*)iceP_reason
{
    return ICE_AUTORELEASE([(ICESecurityException *)[self alloc] init:file_ line:line_ reason:iceP_reason]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICESecurityException *)[[self class] allocWithZone:zone] init:file line:line reason:reason_];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->reason_ release];
    [super dealloc];
}
#endif
@end

@implementation ICEFixedProxyException

-(NSString *) ice_id
{
    return @"::Ice::FixedProxyException";
}

+(id) fixedProxyException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEFixedProxyException *)[self alloc] init:file_ line:line_]);
}
@end

@implementation ICEResponseSentException

-(NSString *) ice_id
{
    return @"::Ice::ResponseSentException";
}

+(id) responseSentException:(const char*)file_ line:(int)line_
{
    return ICE_AUTORELEASE([(ICEResponseSentException *)[self alloc] init:file_ line:line_]);
}
@end
