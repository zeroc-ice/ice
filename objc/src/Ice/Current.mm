// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <CurrentI.h>
#import <VersionI.h>
#import <ObjectAdapterI.h>
#import <ConnectionI.h>
#import <IdentityI.h>
#import <Util.h>

//
// NOTE: Current.h/Current.m are not generated from the Slice
// Current.ice file for few reasons:
//
// - Current.ice includes a number of other Slice files that we don't need to generate. 
// - the Current structure is local and slice2objc doesn't support local structures.
//

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

-(id) init:(id<ICEObjectAdapter>)adapter_ 
       con:(id<ICEConnection>)con_ 
       id_:(ICEIdentity *)id__ 
     facet:(NSString *)facet_ 
 operation:(NSString *)operation_ 
      mode:(ICEOperationMode)mode_ 
       ctx:(NSDictionary *)ctx_ 
 requestId:(ICEInt)requestId_
  encoding:(ICEEncodingVersion*)encoding_
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    adapter = [adapter_ retain];
    con = [con_ retain];
    id_ = [id__ retain];
    facet = [facet_ retain];
    operation = [operation_ retain];
    mode = mode_;
    ctx = [ctx_ retain];
    requestId = requestId_;
    encoding = [encoding_ retain];
    return self;
}

+(id) current:(id<ICEObjectAdapter>)adapter_ 
          con:(id<ICEConnection>)con_ 
          id_:(ICEIdentity *)id__
        facet:(NSString *)facet_ 
    operation:(NSString *)operation_ 
         mode:(ICEOperationMode)mode_ 
          ctx:(NSDictionary *)ctx_ 
    requestId:(ICEInt)requestId_
     encoding:(ICEEncodingVersion*)encoding_
{
    ICECurrent *s__ = [((ICECurrent *)[ICECurrent alloc]) init:adapter_ 
                                                           con:con_ 
                                                           id_:id__ 
                                                         facet:facet_ 
                                                     operation:operation_ 
                                                          mode:mode_ 
                                                           ctx:ctx_ 
                                                     requestId:requestId_
                                                      encoding:encoding_];
    [s__ autorelease];
    return s__;
}

+(id) current
{
    ICECurrent *s__ = [[ICECurrent alloc] init];
    [s__ autorelease];
    return s__;
}

-(id) copyWithZone:(NSZone *)zone
{
    ICECurrent *copy_ = [ICECurrent allocWithZone:zone];
    copy_->adapter = [adapter retain];
    copy_->con = [con retain];
    copy_->id_ = [id_ retain];
    copy_->facet = [facet retain];
    copy_->operation = [operation retain];
    copy_->mode = mode;
    copy_->ctx = [ctx retain];
    copy_->requestId = requestId;
    copy_->encoding = [encoding retain];
    return copy_;
}

-(NSUInteger) hash
{
    NSUInteger h_ = 0;
    h_ = (h_ << 1) ^ (!adapter ? 0 : [adapter hash]);
    h_ = (h_ << 1) ^ (!con ? 0 : [con hash]);
    h_ = (h_ << 1) ^ (!id_ ? 0 : [id_ hash]);
    h_ = (h_ << 1) ^ (!facet ? 0 : [facet hash]);
    h_ = (h_ << 1) ^ (!operation ? 0 : [operation hash]);
    h_ = (h_ << 1) ^ mode;
    h_ = (h_ << 1) ^ (!ctx ? 0 : [ctx hash]);
    h_ = (h_ << 1) ^ requestId;
    h_ = (h_ << 1) ^ (!encoding ? 0 : [encoding hash]);
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
        if(![adapter isEqual:obj_->adapter])
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
        if(![con isEqual:obj_->con])
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
        if(![id_ isEqual:obj_->id_])
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
        if(![facet isEqualToString:obj_->facet])
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
        if(![operation isEqualToString:obj_->operation])
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
        if(![ctx isEqual:obj_->ctx])
        {
            return NO;
        }
    }
    if(requestId != obj_->requestId)
    {
        return NO;
    }
    return YES;
}

-(void) dealloc
{
    [adapter release];
    [con release];
    [id_ release];
    [facet release];
    [operation release];
    [ctx release];
    [encoding release];
    [super dealloc];
}

-(ICECurrent*) initWithCurrent:(const Ice::Current&)current
{
    self = [super init];
    if(!self)
    {
        return nil;
    }

    //
    // TODO: Optimize: the servant blobject should cache an ICECurrent object to
    // avoid re-creating the wrappers for each dispatched invocation.
    //
    adapter = [ICEObjectAdapter wrapperWithCxxObjectNoAutoRelease:current.adapter.get()];
    con = [ICEConnection wrapperWithCxxObjectNoAutoRelease:current.con.get()];
    id_ = [[ICEIdentity alloc] initWithIdentity:current.id];
    facet = [[NSString alloc] initWithUTF8String:current.facet.c_str()];
    operation = [[NSString alloc] initWithUTF8String:current.operation.c_str()];
    mode = (ICEOperationMode)current.mode;
    ctx = toNSDictionary(current.ctx);
    requestId = current.requestId;
    encoding = [[ICEEncodingVersion encodingVersionWithEncodingVersion:current.encoding] retain];
    return self;
}
@end

@implementation ICEContextHelper
+(ICEKeyValueTypeHelper) getKeyValueHelper
{
    ICEKeyValueTypeHelper c;
    c.key = [ICEStringHelper class];
    c.value = [ICEStringHelper class];
    return c;
}
@end

@implementation ICEOperationModeHelper
+(ICEInt) getMinValue
{
    return 0;
}

+(ICEInt) getMaxValue
{
    return 3;
}
@end
