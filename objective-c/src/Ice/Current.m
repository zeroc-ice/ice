//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `Current.ice'
#import <objc/Ice/LocalException.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/Current.h>
#import <objc/runtime.h>

#ifndef ICE_API_EXPORTS
#   define ICE_API_EXPORTS
#endif

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wshadow-ivar"
#endif

@implementation ICEInternalPrefixTable(C423A93D9_4BCB_42BD_B85A_5BE7A86D0AC6)
-(void)addPrefixes_C423A93D9_4BCB_42BD_B85A_5BE7A86D0AC6:(NSMutableDictionary*)prefixTable
{
    [prefixTable setObject:@"ICE" forKey:@"::Ice"];
}
@end

@implementation ICECurrent

@synthesize adapter;
@synthesize con;
@synthesize id_;
@synthesize facet;
@synthesize operation;
@synthesize mode;
@synthesize ctx;
@synthesize requestId;
@synthesize encoding;

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
    self->mode = ICENormal;
    self->encoding = [[ICEEncodingVersion alloc] init];
    return self;
}

-(id) init:(ICELocalObject<ICEObjectAdapter> *)iceP_adapter con:(ICELocalObject<ICEConnection> *)iceP_con id:(ICEIdentity*)iceP_id facet:(NSString*)iceP_facet operation:(NSString*)iceP_operation mode:(ICEOperationMode)iceP_mode ctx:(ICEContext*)iceP_ctx requestId:(ICEInt)iceP_requestId encoding:(ICEEncodingVersion*)iceP_encoding
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->adapter = ICE_RETAIN(iceP_adapter);
    self->con = ICE_RETAIN(iceP_con);
    self->id_ = ICE_RETAIN(iceP_id);
    self->facet = ICE_RETAIN(iceP_facet);
    self->operation = ICE_RETAIN(iceP_operation);
    self->mode = iceP_mode;
    self->ctx = ICE_RETAIN(iceP_ctx);
    self->requestId = iceP_requestId;
    self->encoding = ICE_RETAIN(iceP_encoding);
    return self;
}

+(id) current
{
    return ICE_AUTORELEASE([(ICECurrent *)[self alloc] init]);
}

+(id) current:(ICELocalObject<ICEObjectAdapter> *)iceP_adapter con:(ICELocalObject<ICEConnection> *)iceP_con id:(ICEIdentity*)iceP_id facet:(NSString*)iceP_facet operation:(NSString*)iceP_operation mode:(ICEOperationMode)iceP_mode ctx:(ICEContext*)iceP_ctx requestId:(ICEInt)iceP_requestId encoding:(ICEEncodingVersion*)iceP_encoding
{
    return ICE_AUTORELEASE([(ICECurrent *)[self alloc] init:iceP_adapter con:iceP_con id:iceP_id facet:iceP_facet operation:iceP_operation mode:iceP_mode ctx:iceP_ctx requestId:iceP_requestId encoding:iceP_encoding]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICECurrent *)[[self class] allocWithZone:zone] init:adapter con:con id:id_ facet:facet operation:operation mode:mode ctx:ctx requestId:requestId encoding:encoding];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->adapter release];
    [self->con release];
    [self->id_ release];
    [self->facet release];
    [self->operation release];
    [self->ctx release];
    [self->encoding release];
    [super dealloc];
}
#endif

-(NSUInteger) hash
{
    NSUInteger h_ = 5381;
    h_ = ((h_ << 5) + h_) ^ [self->adapter hash];
    h_ = ((h_ << 5) + h_) ^ [self->con hash];
    h_ = ((h_ << 5) + h_) ^ [self->id_ hash];
    h_ = ((h_ << 5) + h_) ^ [self->facet hash];
    h_ = ((h_ << 5) + h_) ^ [self->operation hash];
    h_ = ((h_ << 5) + h_) ^ (uint)mode;
    h_ = ((h_ << 5) + h_) ^ [self->ctx hash];
    h_ = ((h_ << 5) + h_) ^ (2654435761u * (uint)requestId);
    h_ = ((h_ << 5) + h_) ^ [self->encoding hash];
    return h_;
}

-(BOOL) isEqual:(id)o_
{
    if(self == o_)
    {
        return YES;
    }
    if(!o_ || ![o_ isKindOfClass:[self class]])
    {
        return NO;
    }
    ICECurrent *obj_ = (ICECurrent *)o_;
    if(!adapter)
    {
        if(obj_->adapter)
        {
            return NO;
        }
    }
    else
    {
        if(![self.adapter isEqual:obj_->adapter])
        {
            return NO;
        }
    }
    if(!con)
    {
        if(obj_->con)
        {
            return NO;
        }
    }
    else
    {
        if(![self.con isEqual:obj_->con])
        {
            return NO;
        }
    }
    if(!id_)
    {
        if(obj_->id_)
        {
            return NO;
        }
    }
    else
    {
        if(![self.id_ isEqual:obj_->id_])
        {
            return NO;
        }
    }
    if(!facet)
    {
        if(obj_->facet)
        {
            return NO;
        }
    }
    else
    {
        if(![self.facet isEqualToString:obj_->facet])
        {
            return NO;
        }
    }
    if(!operation)
    {
        if(obj_->operation)
        {
            return NO;
        }
    }
    else
    {
        if(![self.operation isEqualToString:obj_->operation])
        {
            return NO;
        }
    }
    if(mode != obj_->mode)
    {
        return NO;
    }
    if(!ctx)
    {
        if(obj_->ctx)
        {
            return NO;
        }
    }
    else
    {
        if(![self.ctx isEqual:obj_->ctx])
        {
            return NO;
        }
    }
    if(requestId != obj_->requestId)
    {
        return NO;
    }
    if(!encoding)
    {
        if(obj_->encoding)
        {
            return NO;
        }
    }
    else
    {
        if(![self.encoding isEqual:obj_->encoding])
        {
            return NO;
        }
    }
    return YES;
}
@end
