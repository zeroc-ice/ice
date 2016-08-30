// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <ProxyI.h>
#import <Util.h>
#import <StreamI.h>
#import <VersionI.h>
#import <CommunicatorI.h>
#import <IdentityI.h>
#import <ConnectionI.h>
#import <LocalObjectI.h>

#import <objc/Ice/Object.h>
#import <objc/Ice/LocalException.h>
#import <objc/Ice/Router.h>
#import <objc/Ice/Locator.h>

#include <Ice/Initialize.h>
#include <Ice/Proxy.h>
#include <Ice/LocalException.h>
#include <Ice/Router.h>
#include <Ice/Locator.h>

#import <objc/runtime.h>
#import <objc/message.h>

#import <Foundation/NSThread.h>
#import <Foundation/NSInvocation.h>

#include <Block.h>

#define OBJECTPRX ((IceProxy::Ice::Object*)objectPrx__)
#define ASYNCRESULT ((Ice::AsyncResult*)asyncResult__)

namespace
{

class BeginInvokeAsyncCallback : public IceUtil::Shared
{
public:

BeginInvokeAsyncCallback(void (^completed)(id<ICEInputStream>, BOOL),
                         void (^exception)(ICEException*),
                         void (^sent)(BOOL),
                         BOOL returnsData) :
    _completed(Block_copy(completed)),
    _exception(Block_copy(exception)),
    _sent(Block_copy(sent)),
    _returnsData(returnsData)
{
}

virtual ~BeginInvokeAsyncCallback()
{
    Block_release(_completed);
    Block_release(_exception);
    Block_release(_sent);
}

void completed(const Ice::AsyncResultPtr& result)
{
    BOOL ok = YES; // Keep the compiler happy.
    id<ICEInputStream> is = nil;
    NSException* nsex = nil;
    Ice::ObjectPrx proxy = result->getProxy();
    try
    {
        std::vector<Ice::Byte> outParams;
        ok = proxy->end_ice_invoke(outParams, result);
        if(_returnsData)
        {
            Ice::InputStreamPtr s = Ice::createInputStream(proxy->ice_getCommunicator(), outParams);
            is = [ICEInputStream localObjectWithCxxObjectNoAutoRelease:s.get()];
        }
        else if(!outParams.empty())
        {
            if(ok)
            {
                if(outParams.size() != 6)
                {
                    throw Ice::EncapsulationException(__FILE__, __LINE__);
                }
            }
            else
            {
                Ice::InputStreamPtr s = Ice::createInputStream(proxy->ice_getCommunicator(), outParams);
                is = [ICEInputStream localObjectWithCxxObjectNoAutoRelease:s.get()];
                @try
                {
                    [is startEncapsulation];
                    [is throwException];
                }
                @catch(ICEUserException* ex)
                {
                    [is endEncapsulation];
                    [is release];
                    is = nil;
                    nsex = [ICEUnknownUserException unknownUserException:__FILE__ line:__LINE__ unknown:[ex ice_name]];
                }
            }
        }
    }
    catch(const std::exception& ex)
    {
        if(is != nil)
        {
            [is release];
            is = nil;
        }
        nsex = toObjCException(ex);
    }

    NSException* exception = nil;
    @autoreleasepool
    {
        @try
        {
            if(nsex != nil)
            {
                @try
                {
                    @throw nsex;
                }
                @catch(ICEException* ex)
                {
                    if(_exception)
                    {
                        _exception(ex);
                    }
                    return;
                }
            }

            _completed(is, ok);
        }
        @catch(id e)
        {
            exception = [e retain];
        }
        @finally
        {
            [is release];
        }
    }

    if(exception != nil)
    {
        rethrowCxxException(exception, true); // True = release the exception.
    }
}

void sent(const Ice::AsyncResultPtr& result)
{
    if(!_sent)
    {
        return;
    }

    NSException* exception = nil;
    @autoreleasepool
    {
        @try
        {
            _sent(result->sentSynchronously());
        }
        @catch(id e)
        {
            exception = [e retain];
        }
    }

    if(exception != nil)
    {
        rethrowCxxException(exception, true); // True = release the exception.
    }
}

private:

void (^_completed)(id<ICEInputStream>, BOOL);
void (^_exception)(ICEException*);
void (^_sent)(BOOL);
BOOL _returnsData;

};

};

@implementation ICEAsyncResult
-(ICEAsyncResult*) initWithAsyncResult__:(const Ice::AsyncResultPtr&)arg
                               operation:(NSString*)op
                                   proxy:(id<ICEObjectPrx>)p;
{
    self = [super init];
    if(!self)
    {
        return nil;
    }

    asyncResult__ = arg.get();
    ASYNCRESULT->__incRef();
    operation_ = [op retain];
    proxy_ = [p retain];
    return self;
}

-(Ice::AsyncResult*) asyncResult__
{
    return ASYNCRESULT;
}

-(void) dealloc
{
    ASYNCRESULT->__decRef();
    asyncResult__ = 0;
    [operation_ release];
    [proxy_ release];
    [super dealloc];
}

+(ICEAsyncResult*) asyncResultWithAsyncResult__:(const Ice::AsyncResultPtr&)arg
{
    return [self asyncResultWithAsyncResult__:arg operation:nil proxy:nil];
}
+(ICEAsyncResult*) asyncResultWithAsyncResult__:(const Ice::AsyncResultPtr&)arg
                                      operation:(NSString*)op
                                          proxy:(id<ICEObjectPrx>)p
{
    if(!arg)
    {
        return nil;
    }
    else
    {
        return [[[self alloc] initWithAsyncResult__:arg operation:op proxy:p] autorelease];
    }
}
-(NSString*) operation
{
    return operation_;
}

-(void) cancel
{
    ASYNCRESULT->cancel();
}

-(id<ICECommunicator>) getCommunicator
{
    return [ICECommunicator localObjectWithCxxObject:ASYNCRESULT->getCommunicator().get()];
}

-(id<ICEConnection>) getConnection
{
    return [ICEConnection localObjectWithCxxObject:ASYNCRESULT->getConnection().get()];
}

-(id<ICEObjectPrx>) getProxy
{
    return [[proxy_ retain] autorelease];
}

-(BOOL) isCompleted
{
    return ASYNCRESULT->isCompleted();
}

-(void) waitForCompleted
{
    ASYNCRESULT->waitForCompleted();
}

-(BOOL) isSent
{
    return ASYNCRESULT->isSent();
}

-(void) waitForSent
{
    ASYNCRESULT->waitForSent();
}

-(BOOL) sentSynchronously
{
    return ASYNCRESULT->sentSynchronously();
}

-(void) throwLocalException
{
    NSException* nsex;
    try
    {
        ASYNCRESULT->throwLocalException();
        return;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}

-(NSString*) getOperation
{
    if(operation_ != nil)
    {
        return [[operation_ retain] autorelease];
    }
    else
    {
        return [toNSString(ASYNCRESULT->getOperation()) autorelease];
    }
}
@end

@implementation ICEObjectPrx

-(ICEObjectPrx*) initWithObjectPrx__:(const Ice::ObjectPrx&)arg
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    communicator__ = [ICECommunicator localObjectWithCxxObjectNoAutoRelease:arg->ice_getCommunicator().get()];
    objectPrx__ = arg.get();
    OBJECTPRX->__incRef();
    return self;
}

-(IceProxy::Ice::Object*) objectPrx__
{
    return (IceProxy::Ice::Object*)objectPrx__;
}

-(void) dealloc
{
    OBJECTPRX->__decRef();
    objectPrx__ = 0;
    [communicator__ release];
    [super dealloc];
}

+(ICEObjectPrx*) objectPrxWithObjectPrx__:(const Ice::ObjectPrx&)arg
{
    if(!arg)
    {
        return nil;
    }
    else
    {
        return [[[self alloc] initWithObjectPrx__:arg] autorelease];
    }
}

+(id) uncheckedCast:(id<ICEObjectPrx>)proxy
{
    if(proxy != nil)
    {
        if([(ICEObjectPrx*)proxy isKindOfClass:self])
        {
            return [[proxy retain] autorelease];
        }
        else
        {
            return [[[self alloc] initWithObjectPrx__:[(ICEObjectPrx*)proxy objectPrx__]] autorelease];
        }
    }
    return nil;
}
+(id) uncheckedCast:(id<ICEObjectPrx>)proxy facet:(NSString*)facet
{
    return [self uncheckedCast:[proxy ice_facet:facet]];
}
+(id) checkedCast:(id<ICEObjectPrx>)proxy
{
    if(proxy != nil)
    {
        if([(ICEObjectPrx*)proxy isKindOfClass:self])
        {
            return [[proxy retain] autorelease];
        }
        else if([(ICEObjectPrx*)proxy conformsToProtocol:[self protocol__]] ||
                [proxy ice_isA:[self ice_staticId]])
        {
            return [[[self alloc] initWithObjectPrx__:[(ICEObjectPrx*)proxy objectPrx__]] autorelease];
        }
    }
    return nil;
}
+(id) checkedCast:(id<ICEObjectPrx>)proxy facet:(NSString*)facet
{
    @try
    {
        return [self checkedCast:[proxy ice_facet:facet]];
    }
    @catch(ICEFacetNotExistException* ex)
    {
        return nil;
    }
}
+(id) checkedCast:(id<ICEObjectPrx>)proxy context:(ICEContext*)context
{
    if(proxy != nil)
    {
        if([(ICEObjectPrx*)proxy isKindOfClass:self])
        {
            return [[proxy retain] autorelease];
        }
        else if([(ICEObjectPrx*)proxy conformsToProtocol:[self protocol__]] ||
                [proxy ice_isA:[self ice_staticId] context:context])
        {
            return [[[self alloc] initWithObjectPrx__:[(ICEObjectPrx*)proxy objectPrx__]] autorelease];
        }
    }
    return nil;
}
+(id) checkedCast:(id<ICEObjectPrx>)proxy facet:(NSString*)facet context:(ICEContext*)context
{
    @try
    {
        return [self checkedCast:[proxy ice_facet:facet] context:context];
    }
    @catch(ICEFacetNotExistException* ex)
    {
        return nil;
    }
}
+(NSString*) ice_staticId
{
    return @"::Ice::Object";
}

+(Protocol*) protocol__
{
    return objc_getProtocol(class_getName([self class]));
}

-(id<ICEOutputStream>) createOutputStream__
{
    NSException* nsex = nil;
    try
    {
        Ice::OutputStreamPtr os = Ice::createOutputStream(OBJECTPRX->ice_getCommunicator());
        return [ICEOutputStream localObjectWithCxxObjectNoAutoRelease:os.get()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(void) checkAsyncTwowayOnly__:(NSString*)operation
{
    //
    // No mutex lock necessary, there is nothing mutable in this
    // operation.
    //

    if(![self ice_isTwoway])
    {
        @throw [NSException exceptionWithName:NSInvalidArgumentException
                            reason:[NSString stringWithFormat:@"`%@' can only be called with a twoway proxy", operation]
                            userInfo:nil];
    }
}

-(void) invoke__:(NSString*)operation
            mode:(ICEOperationMode)mode
          format:(ICEFormatType)format
         marshal:(ICEMarshalCB)marshal
       unmarshal:(ICEUnmarshalCB)unmarshal
         context:(ICEContext*)context
{
    if(unmarshal && !OBJECTPRX->ice_isTwoway())
    {
        @throw [ICETwowayOnlyException twowayOnlyException:__FILE__ line:__LINE__ operation:operation];
    }

    ICEOutputStream<ICEOutputStream>* os = nil;
    if(marshal)
    {
        os = [self createOutputStream__];
        try
        {
            [os os]->startEncapsulation(IceInternal::getCompatibleEncoding(OBJECTPRX->ice_getEncodingVersion()),
                                        (Ice::FormatType)format);
        }
        catch(const std::exception& ex)
        {
            [os release];
            @throw toObjCException(ex);
        }

        @try
        {
            marshal(os);
        }
        @catch(id ex)
        {
            [os release];
            @throw ex;
        }
    }

    BOOL ok = YES; // Keep the compiler happy.
    ICEInputStream<ICEInputStream>* is = nil;
    NSException* nsex = nil;
    try
    {
        std::vector<Ice::Byte> inParams;
        if(os)
        {
            [os os]->endEncapsulation();
            [os os]->finished(inParams);
            [os release];
            os = nil;
        }

        std::vector<Ice::Byte> outParams;
        if(context != nil)
        {
            Ice::Context ctx;
            fromNSDictionary(context, ctx);
            ok = OBJECTPRX->ice_invoke(fromNSString(operation), (Ice::OperationMode)mode, inParams, outParams, ctx);
        }
        else
        {
            ok = OBJECTPRX->ice_invoke(fromNSString(operation), (Ice::OperationMode)mode, inParams, outParams);
        }

        if(unmarshal)
        {
            Ice::InputStreamPtr s = Ice::createInputStream(OBJECTPRX->ice_getCommunicator(), outParams);
            is = [ICEInputStream localObjectWithCxxObjectNoAutoRelease:s.get()];
        }
        else if(!outParams.empty())
        {
            if(ok)
            {
                if(outParams.size() != 6)
                {
                    throw Ice::EncapsulationException(__FILE__, __LINE__);
                }
            }
            else
            {
                Ice::InputStreamPtr s = Ice::createInputStream(OBJECTPRX->ice_getCommunicator(), outParams);
                is = [ICEInputStream localObjectWithCxxObjectNoAutoRelease:s.get()];
                @try
                {
                    [is startEncapsulation];
                    [is throwException];
                }
                @catch(ICEUserException* ex)
                {
                    [is endEncapsulation];
                    [is release];
                    is = nil;
                    nsex = [ICEUnknownUserException unknownUserException:__FILE__ line:__LINE__ unknown:[ex ice_name]];
                }
            }
        }
    }
    catch(const std::exception& ex)
    {
        if(os != nil)
        {
            [os release];
            os = nil;
        }
        if(is != nil)
        {
            [is release];
            is = nil;
        }
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }

    NSAssert(os == nil, @"output stream not cleared");
    if(is)
    {
        @try
        {
            unmarshal(is, ok);
        }
        @finally
        {
            [is release];
        }
    }
}

-(id<ICEAsyncResult>) begin_invoke__:(NSString*)operation
                                mode:(ICEOperationMode)mode
                              format:(ICEFormatType)format
                             marshal:(void(^)(id<ICEOutputStream>))marshal
                         returnsData:(BOOL)returnsData
                             context:(ICEContext*)context
{
    if(returnsData)
    {
        [self checkAsyncTwowayOnly__:operation];
    }

    ICEOutputStream<ICEOutputStream>* os = nil;
    if(marshal)
    {
        os = [self createOutputStream__];
        try
        {
            [os os]->startEncapsulation(IceInternal::getCompatibleEncoding(OBJECTPRX->ice_getEncodingVersion()),
                                        (Ice::FormatType)format);
        }
        catch(const std::exception& ex)
        {
            [os release];
            @throw toObjCException(ex);
        }

        @try
        {
            marshal(os);
        }
        @catch(id ex)
        {
            [os release];
            @throw ex;
        }
    }

    NSException* nsex = nil;
    try
    {
        std::vector<Ice::Byte> inParams;
        if(os)
        {
            [os os]->endEncapsulation();
            [os os]->finished(inParams);
            [os release];
            os = nil;
        }

        Ice::AsyncResultPtr r;
        if(context != nil)
        {
            Ice::Context ctx;
            fromNSDictionary(context, ctx);
            r = OBJECTPRX->begin_ice_invoke(fromNSString(operation), (Ice::OperationMode)mode, inParams, ctx);
        }
        else
        {
            r = OBJECTPRX->begin_ice_invoke(fromNSString(operation), (Ice::OperationMode)mode, inParams);
        }
        return [ICEAsyncResult asyncResultWithAsyncResult__:r operation:operation proxy:self];
    }
    catch(const std::exception& ex)
    {
        if(os != nil)
        {
            [os release];
            os = nil;
        }
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return nil; // Keep the compiler happy.
}

-(id<ICEAsyncResult>) begin_invoke__:(NSString*)operation
                                mode:(ICEOperationMode)mode
                              format:(ICEFormatType)format
                             marshal:(void(^)(id<ICEOutputStream>))marshal
                         returnsData:(BOOL)returnsData
                           completed:(void(^)(id<ICEInputStream>, BOOL))completed
                            response:(BOOL)response
                           exception:(void(^)(ICEException*))exception
                                sent:(void(^)(BOOL))sent
                             context:(ICEContext*)context
{
    if(returnsData)
    {
        [self checkAsyncTwowayOnly__:operation];
        if(!response)
        {
            @throw [NSException exceptionWithName:NSInvalidArgumentException
                                reason:@"Response callback is nil"
                                userInfo:nil];
        }
    }

    if(!exception)
    {
        @throw [NSException exceptionWithName:NSInvalidArgumentException
                            reason:@"Exception callback is nil"
                            userInfo:nil];
    }

    ICEOutputStream<ICEOutputStream>* os = nil;
    if(marshal)
    {
        os = [self createOutputStream__];
        try
        {
            [os os]->startEncapsulation(IceInternal::getCompatibleEncoding(OBJECTPRX->ice_getEncodingVersion()),
                                        (Ice::FormatType)format);
        }
        catch(const std::exception& ex)
        {
            [os release];
            @throw toObjCException(ex);
        }

        @try
        {
            marshal(os);
        }
        @catch(id ex)
        {
            [os release];
            @throw ex;
        }
    }

    NSException* nsex = nil;
    try
    {
        std::vector<Ice::Byte> inParams;
        if(os)
        {
            [os os]->endEncapsulation();
            [os os]->finished(inParams);
            [os release];
            os = nil;
        }

        Ice::CallbackPtr cb = Ice::newCallback(new BeginInvokeAsyncCallback(completed, exception, sent, returnsData),
                                               &BeginInvokeAsyncCallback::completed,
                                               &BeginInvokeAsyncCallback::sent);
        Ice::AsyncResultPtr r;
        if(context != nil)
        {
            Ice::Context ctx;
            fromNSDictionary(context, ctx);
            r = OBJECTPRX->begin_ice_invoke(fromNSString(operation), (Ice::OperationMode)mode, inParams, ctx, cb);
        }
        else
        {
            r = OBJECTPRX->begin_ice_invoke(fromNSString(operation), (Ice::OperationMode)mode, inParams, cb);
        }
        return [ICEAsyncResult asyncResultWithAsyncResult__:r operation:operation proxy:self];
    }
    catch(const std::exception& ex)
    {
        if(os != nil)
        {
            [os release];
            os = nil;
        }
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return nil; // Keep the compiler happy.
}
-(id<ICEAsyncResult>) begin_invoke__:(NSString*)op
                                mode:(ICEOperationMode)mode
                              format:(ICEFormatType)format
                             marshal:(void(^)(id<ICEOutputStream>))marshal
                            response:(void(^)())response
                           exception:(void(^)(ICEException*))exception
                                sent:(void(^)(BOOL))sent
                             context:(ICEContext*)ctx
{
    void(^completed)(id<ICEInputStream>, BOOL) = ^(id<ICEInputStream>, BOOL) {
        if(response)
        {
            response();
        }
    };
    return [self begin_invoke__:op
                 mode:mode
                 format:format
                 marshal:marshal
                 returnsData:NO
                 completed:completed
                 response:TRUE
                 exception:exception
                 sent:sent
                 context:ctx];
}

-(id<ICEAsyncResult>) begin_invoke__:(NSString*)op
                                mode:(ICEOperationMode)mode
                              format:(ICEFormatType)format
                             marshal:(void(^)(id<ICEOutputStream>))marshal
                           completed:(void(^)(id<ICEInputStream>, BOOL))completed
                            response:(BOOL)response
                           exception:(void(^)(ICEException*))exception
                                sent:(void(^)(BOOL))sent
                             context:(ICEContext*)ctx
{
    return [self begin_invoke__:op
                 mode:mode
                 format:format
                 marshal:marshal
                 returnsData:TRUE
                 completed:completed
                 response:response
                 exception:exception
                 sent:sent context:ctx];
}

-(void)end_invoke__:(NSString*)operation unmarshal:(ICEUnmarshalCB)unmarshal result:(id<ICEAsyncResult>)r
{
    ICEAsyncResult* result = (ICEAsyncResult*)r;
    if(operation != [result operation])
    {
        @throw [NSException exceptionWithName:NSInvalidArgumentException
                            reason:[NSString stringWithFormat:@"Incorrect operation for end_%@ method: %@",
                                             operation, [result operation]]
                            userInfo:nil];
    }
    if(result == nil)
    {
        @throw [NSException exceptionWithName:NSInvalidArgumentException
                            reason:@"ICEAsyncResult is nil"
                            userInfo:nil];
    }

    BOOL ok = YES; // Keep the compiler happy.
    NSException* nsex = nil;
    ICEInputStream* is = nil;
    try
    {
        std::vector<Ice::Byte> outParams;
        ok = OBJECTPRX->end_ice_invoke(outParams, [result asyncResult__]);

        if(unmarshal)
        {
            Ice::InputStreamPtr s = Ice::createInputStream(OBJECTPRX->ice_getCommunicator(), outParams);
            is = [ICEInputStream localObjectWithCxxObjectNoAutoRelease:s.get()];
        }
        else if(!outParams.empty())
        {
            if(ok)
            {
                if(outParams.size() != 6)
                {
                    throw Ice::EncapsulationException(__FILE__, __LINE__);
                }
            }
            else
            {
                Ice::InputStreamPtr s = Ice::createInputStream(OBJECTPRX->ice_getCommunicator(), outParams);
                is = [ICEInputStream localObjectWithCxxObjectNoAutoRelease:s.get()];
                @try
                {
                    [is startEncapsulation];
                    [is throwException];
                }
                @catch(ICEUserException* ex)
                {
                    [is endEncapsulation];
                    [is release];
                    is = nil;
                    nsex = [ICEUnknownUserException unknownUserException:__FILE__ line:__LINE__ unknown:[ex ice_name]];
                }
            }
        }
    }
    catch(const std::exception& ex)
    {
        if(is != nil)
        {
            [is release];
            is = nil;
        }
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }

    if(is != nil)
    {
        @try
        {
            unmarshal(is, ok);
        }
        @finally
        {
            [is release];
        }
    }
}

-(id) copyWithZone:(NSZone *)zone
{
    return [self retain];
}

-(NSUInteger) hash
{
    return (NSUInteger)OBJECTPRX->__hash();
}
-(NSString*) description
{
    return [toNSString(OBJECTPRX->ice_toString()) autorelease];
}
-(BOOL) isEqual:(id)o_
{
    if(self == o_)
    {
        return YES;
    }
    if(!o_ || ![o_ isKindOfClass:[ICEObjectPrx class]])
    {
        return NO;
    }
    return *OBJECTPRX == *[o_ objectPrx__];
}

-(NSComparisonResult) compareIdentity:(id<ICEObjectPrx>)aProxy
{
    IceProxy::Ice::Object* lhs = OBJECTPRX;
    IceProxy::Ice::Object* rhs = [(ICEObjectPrx*)aProxy objectPrx__];
    if(Ice::proxyIdentityEqual(lhs, rhs))
    {
        return NSOrderedSame;
    }
    else if(Ice::proxyIdentityLess(lhs, rhs))
    {
        return NSOrderedAscending;
    }
    else
    {
        return NSOrderedDescending;
    }
}

-(NSComparisonResult) compareIdentityAndFacet:(id<ICEObjectPrx>)aProxy
{
    IceProxy::Ice::Object* lhs = OBJECTPRX;
    IceProxy::Ice::Object* rhs = [(ICEObjectPrx*)aProxy objectPrx__];
    if(Ice::proxyIdentityAndFacetEqual(lhs, rhs))
    {
        return NSOrderedSame;
    }
    else if(Ice::proxyIdentityAndFacetLess(lhs, rhs))
    {
        return NSOrderedAscending;
    }
    else
    {
        return NSOrderedDescending;
    }
}

-(id<ICECommunicator>) ice_getCommunicator
{
    return [[communicator__ retain] autorelease];
}

-(NSMutableString*) ice_toString
{
    return [toNSMutableString(OBJECTPRX->ice_toString()) autorelease];
}

-(BOOL) ice_isA:(NSString*)typeId
{
    __block BOOL ret__;
    cppCall(^ { ret__ = OBJECTPRX->ice_isA(fromNSString(typeId)); });
    return ret__;
}
-(BOOL) ice_isA:(NSString*)typeId context:(ICEContext*)context
{
    __block BOOL ret__;
    cppCall(^(const Ice::Context& ctx) { ret__ = OBJECTPRX->ice_isA(fromNSString(typeId), ctx); }, context);
    return ret__;
}
-(id<ICEAsyncResult>) begin_ice_isA:(NSString*)typeId
{
    return beginCppCall(^(Ice::AsyncResultPtr& result)
                        {
                            result = OBJECTPRX->begin_ice_isA(fromNSString(typeId));
                        }, self);
}
-(id<ICEAsyncResult>) begin_ice_isA:(NSString*)typeId context:(ICEContext*)context
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::Context& ctx)
                        {
                            result = OBJECTPRX->begin_ice_isA(fromNSString(typeId), ctx);
                        }, context, self);
}
-(id<ICEAsyncResult>) begin_ice_isA:(NSString*)typeId
                           response:(void(^)(BOOL))response
                          exception:(void(^)(ICEException*))exception
{
    return [self begin_ice_isA:typeId response:response exception:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_ice_isA:(NSString*)typeId
                            context:(ICEContext*)context
                           response:(void(^)(BOOL))response
                          exception:(void(^)(ICEException*))exception

{
    return [self begin_ice_isA:typeId context:context response:response exception:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_ice_isA:(NSString*)typeId
                           response:(void(^)(BOOL))response
                          exception:(void(^)(ICEException*))exception
                               sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::CallbackPtr& cb)
                        {
                            result = OBJECTPRX->begin_ice_isA(fromNSString(typeId), cb);
                        },
                        ^(const Ice::AsyncResultPtr& result) {
                            BOOL ret__ = OBJECTPRX->end_ice_isA(result);
                            if(response)
                            {
                                response(ret__);
                            }
                        },
                        exception, sent, self);

}
-(id<ICEAsyncResult>) begin_ice_isA:(NSString*)typeId
                            context:(ICEContext*)context
                           response:(void(^)(BOOL))response
                          exception:(void(^)(ICEException*))exception
                               sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::Context& ctx, const Ice::CallbackPtr& cb)
                        {
                            result = OBJECTPRX->begin_ice_isA(fromNSString(typeId), ctx, cb);
                        },
                        context,
                        ^(const Ice::AsyncResultPtr& result) {
                            BOOL ret__ = OBJECTPRX->end_ice_isA(result);
                            if(response)
                            {
                                response(ret__);
                            }
                        },
                        exception, sent, self);
}
-(BOOL) end_ice_isA:(id<ICEAsyncResult>)result
{
    __block BOOL ret__;
    endCppCall(^(const Ice::AsyncResultPtr& r) { ret__ = OBJECTPRX->end_ice_isA(r); }, result);
    return ret__;
}

-(void) ice_ping
{
    cppCall(^ { OBJECTPRX->ice_ping(); });
}
-(void) ice_ping:(ICEContext*)context
{
    cppCall(^(const Ice::Context& ctx) { OBJECTPRX->ice_ping(ctx); }, context);
}
-(id<ICEAsyncResult>) begin_ice_ping
{
    return beginCppCall(^(Ice::AsyncResultPtr& result) { result = OBJECTPRX->begin_ice_ping(); }, self);
}
-(id<ICEAsyncResult>) begin_ice_ping:(ICEContext*)context
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::Context& ctx)
                        {
                            result = OBJECTPRX->begin_ice_ping(ctx);
                        }, context, self);
}

-(id<ICEAsyncResult>) begin_ice_ping:(void(^)())response exception:(void(^)(ICEException*))exception
{
    return [self begin_ice_ping:response exception:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_ice_ping:(ICEContext*)context
                            response:(void(^)())response
                           exception:(void(^)(ICEException*))exception

{
    return [self begin_ice_ping:context response:response exception:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_ice_ping:(void(^)())response
                           exception:(void(^)(ICEException*))exception
                                sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::CallbackPtr& cb)
                        {
                            result = OBJECTPRX->begin_ice_ping(cb);
                        },
                        ^(const Ice::AsyncResultPtr& result) {
                            OBJECTPRX->end_ice_ping(result);
                            if(response)
                            {
                                response();
                            }
                        },
                        exception, sent, self);
}

-(id<ICEAsyncResult>) begin_ice_ping:(ICEContext*)context
                            response:(void(^)())response
                           exception:(void(^)(ICEException*))exception
                                sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::Context& ctx, const Ice::CallbackPtr& cb)
                        {
                            result = OBJECTPRX->begin_ice_ping(ctx, cb);
                        },
                        context,
                        ^(const Ice::AsyncResultPtr& result) {
                            OBJECTPRX->end_ice_ping(result);
                            if(response)
                            {
                                response();
                            }
                        },
                        exception, sent, self);
}
-(void) end_ice_ping:(id<ICEAsyncResult>)result
{
    endCppCall(^(const Ice::AsyncResultPtr& r) { OBJECTPRX->end_ice_ping(r); }, result);
}

-(NSMutableArray*) ice_ids
{
    __block NSMutableArray* ret__;
    cppCall(^ { ret__ = [toNSArray(OBJECTPRX->ice_ids()) autorelease]; });
    return ret__;
}
-(NSMutableArray*) ice_ids:(ICEContext*)context
{
    __block NSMutableArray* ret__;
    cppCall(^(const Ice::Context& ctx) { ret__ = [toNSArray(OBJECTPRX->ice_ids(ctx)) autorelease]; }, context);
    return ret__;
}
-(id<ICEAsyncResult>) begin_ice_ids
{
    return beginCppCall(^(Ice::AsyncResultPtr& result) { result = OBJECTPRX->begin_ice_ids(); } , self);
}

-(id<ICEAsyncResult>) begin_ice_ids:(ICEContext*)context
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::Context& ctx)
                        {
                            result = OBJECTPRX->begin_ice_ids(ctx);
                        }, context, self);
}
-(id<ICEAsyncResult>) begin_ice_ids:(void(^)(NSArray*))response exception:(void(^)(ICEException*))exception
{
    return [self begin_ice_ids:response exception:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_ice_ids:(ICEContext*)context
                           response:(void(^)(NSArray*))response
                          exception:(void(^)(ICEException*))exception
{
    return [self begin_ice_ids:context response:response exception:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_ice_ids:(void(^)(NSArray*))response
                          exception:(void(^)(ICEException*))exception
                               sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::CallbackPtr& cb)
                        {
                            result = OBJECTPRX->begin_ice_ids(cb);
                        },
                        ^(const Ice::AsyncResultPtr& result) {
                            NSMutableArray* ret__ = [toNSArray(OBJECTPRX->end_ice_ids(result)) autorelease];
                            if(response)
                            {
                                response(ret__);
                            }
                        },
                        exception, sent, self);
}
-(id<ICEAsyncResult>) begin_ice_ids:(ICEContext*)context
                           response:(void(^)(NSArray*))response
                          exception:(void(^)(ICEException*))exception
                               sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::Context& ctx, const Ice::CallbackPtr& cb)
                        {
                            result = OBJECTPRX->begin_ice_ids(ctx, cb);
                        },
                        context,
                        ^(const Ice::AsyncResultPtr& result) {
                            NSMutableArray* ret__ = [toNSArray(OBJECTPRX->end_ice_ids(result)) autorelease];
                            if(response)
                            {
                                response(ret__);
                            }
                        },
                        exception, sent, self);
}
-(NSMutableArray*) end_ice_ids:(id<ICEAsyncResult>)result
{
    __block NSMutableArray* ret__;
    endCppCall(^(const Ice::AsyncResultPtr& r) { ret__ = [toNSArray(OBJECTPRX->end_ice_ids(r)) autorelease]; }, result);
    return ret__;
}

-(NSMutableString*) ice_id
{
    __block NSMutableString* ret__;
    cppCall(^ { ret__ = [toNSMutableString(OBJECTPRX->ice_id()) autorelease]; });
    return ret__;
}
-(NSMutableString*) ice_id:(ICEContext*)context
{
    __block NSMutableString* ret__;
    cppCall(^(const Ice::Context& ctx) { ret__ = [toNSMutableString(OBJECTPRX->ice_id(ctx)) autorelease]; }, context);
    return ret__;
}
-(id<ICEAsyncResult>) begin_ice_id
{
    return beginCppCall(^(Ice::AsyncResultPtr& result) { result = OBJECTPRX->begin_ice_id(); } , self);
}
-(id<ICEAsyncResult>) begin_ice_id:(ICEContext*)context
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::Context& ctx)
                        {
                            result = OBJECTPRX->begin_ice_id(ctx);
                        }, context, self);
}
-(id<ICEAsyncResult>) begin_ice_id:(void(^)(NSString*))response exception:(void(^)(ICEException*))exception
{
    return [self begin_ice_id:response exception:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_ice_id:(ICEContext*)context
                          response:(void(^)(NSString*))response
                         exception:(void(^)(ICEException*))exception

{
    return [self begin_ice_id:context response:response exception:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_ice_id:(void(^)(NSString*))response
                         exception:(void(^)(ICEException*))exception
                              sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::CallbackPtr& cb)
                        {
                            result = OBJECTPRX->begin_ice_id(cb);
                        },
                        ^(const Ice::AsyncResultPtr& result) {
                            NSString* ret__ = [toNSString(OBJECTPRX->end_ice_id(result)) autorelease];
                            if(response)
                            {
                                response(ret__);
                            }
                        },
                        exception, sent, self);
}
-(id<ICEAsyncResult>) begin_ice_id:(ICEContext*)context
                          response:(void(^)(NSString*))response
                         exception:(void(^)(ICEException*))exception
                              sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::Context& ctx, const Ice::CallbackPtr& cb)
                        {
                            result = OBJECTPRX->begin_ice_id(ctx, cb);
                        },
                        context,
                        ^(const Ice::AsyncResultPtr& result) {
                            NSString* ret__ = [toNSString(OBJECTPRX->end_ice_id(result)) autorelease];
                            if(response)
                            {
                                response(ret__);
                            }
                        },
                        exception, sent, self);
}
-(NSMutableString*) end_ice_id:(id<ICEAsyncResult>)result
{
    __block NSMutableString* ret__;
    endCppCall(^(const Ice::AsyncResultPtr& r) { ret__ = [toNSMutableString(OBJECTPRX->end_ice_id(r)) autorelease]; },
               result);
    return ret__;
}

-(BOOL) ice_invoke:(NSString*)operation
              mode:(ICEOperationMode)mode
          inEncaps:(NSData*)inEncaps
         outEncaps:(NSMutableData**)outEncaps
{
    __block BOOL ret__;
    cppCall(^ {
            std::pair<const Ice::Byte*, const Ice::Byte*> inP((ICEByte*)[inEncaps bytes],
                                                              (ICEByte*)[inEncaps bytes] + [inEncaps length]);
            std::vector<Ice::Byte> outP;
            ret__ = OBJECTPRX->ice_invoke(fromNSString(operation), (Ice::OperationMode)mode, inP, outP);
            *outEncaps = [NSMutableData dataWithBytes:&outP[0] length:outP.size()];
        });
    return ret__;
}

-(BOOL) ice_invoke:(NSString*)operation
              mode:(ICEOperationMode)mode
          inEncaps:(NSData*)inEncaps
         outEncaps:(NSMutableData**)outEncaps
           context:(ICEContext*)context
{
    __block BOOL ret__;
    cppCall(^(const Ice::Context& ctx) {
            std::pair<const Ice::Byte*, const Ice::Byte*> inP((ICEByte*)[inEncaps bytes],
                                                              (ICEByte*)[inEncaps bytes] + [inEncaps length]);
            std::vector<Ice::Byte> outP;
            ret__ = OBJECTPRX->ice_invoke(fromNSString(operation), (Ice::OperationMode)mode, inP, outP, ctx);
            *outEncaps = [NSMutableData dataWithBytes:&outP[0] length:outP.size()];
        }, context);
    return ret__;
}
-(id<ICEAsyncResult>) begin_ice_invoke:(NSString*)operation mode:(ICEOperationMode)mode inEncaps:(NSData*)inEncaps
{
    return beginCppCall(^(Ice::AsyncResultPtr& result)
                        {
                            std::pair<const Ice::Byte*, const Ice::Byte*>
                                inP((ICEByte*)[inEncaps bytes], (ICEByte*)[inEncaps bytes] + [inEncaps length]);
                            result = OBJECTPRX->begin_ice_invoke(fromNSString(operation),
                                                                 (Ice::OperationMode)mode,
                                                                 inP);
                        }, self);
}
-(id<ICEAsyncResult>) begin_ice_invoke:(NSString*)operation
                                  mode:(ICEOperationMode)mode
                              inEncaps:(NSData*)inEncaps
                               context:(ICEContext*)context
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::Context& ctx)
                        {
                            std::pair<const Ice::Byte*, const Ice::Byte*>
                                inP((ICEByte*)[inEncaps bytes], (ICEByte*)[inEncaps bytes] + [inEncaps length]);
                            result = OBJECTPRX->begin_ice_invoke(fromNSString(operation),
                                                                 (Ice::OperationMode)mode,
                                                                 inP,
                                                                 ctx);
                        }, context, self);
}
-(id<ICEAsyncResult>) begin_ice_invoke:(NSString*)operation
                                  mode:(ICEOperationMode)mode
                              inEncaps:(NSData*)inEncaps
                              response:(void(^)(BOOL, NSMutableData*))response
                             exception:(void(^)(ICEException*))exception
{
    return [self begin_ice_invoke:operation mode:mode inEncaps:inEncaps response:response exception:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_ice_invoke:(NSString*)operation
                                  mode:(ICEOperationMode)mode
                              inEncaps:(NSData*)inEncaps
                               context:(ICEContext*)context
                              response:(void(^)(BOOL, NSMutableData*))response
                             exception:(void(^)(ICEException*))exception
{
    return [self begin_ice_invoke:operation mode:mode inEncaps:inEncaps context:context response:response
                 exception:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_ice_invoke:(NSString*)operation
                                  mode:(ICEOperationMode)mode
                              inEncaps:(NSData*)inEncaps
                              response:(void(^)(BOOL, NSMutableData*))response
                             exception:(void(^)(ICEException*))exception
                                  sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::CallbackPtr& cb)
                        {
                            std::pair<const Ice::Byte*, const Ice::Byte*>
                                inP((ICEByte*)[inEncaps bytes], (ICEByte*)[inEncaps bytes] + [inEncaps length]);
                            result = OBJECTPRX->begin_ice_invoke(fromNSString(operation),
                                                                 (Ice::OperationMode)mode,
                                                                 inP,
                                                                 cb);
                        },
                        ^(const Ice::AsyncResultPtr& result) {
                            std::pair<const ::Ice::Byte*, const ::Ice::Byte*> outP;
                            BOOL ret__ = OBJECTPRX->___end_ice_invoke(outP, result);
                            NSMutableData* outEncaps =
                                [NSMutableData dataWithBytes:outP.first length:(outP.second - outP.first)];
                            if(response)
                            {
                                response(ret__, outEncaps);
                            }
                        },
                        exception, sent, self);
}
-(id<ICEAsyncResult>) begin_ice_invoke:(NSString*)operation
                                  mode:(ICEOperationMode)mode
                              inEncaps:(NSData*)inEncaps
                               context:(ICEContext*)context
                              response:(void(^)(BOOL, NSMutableData*))response
                             exception:(void(^)(ICEException*))exception
                                  sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::Context& ctx, const Ice::CallbackPtr& cb)
                        {
                            std::pair<const Ice::Byte*, const Ice::Byte*>
                                inP((ICEByte*)[inEncaps bytes], (ICEByte*)[inEncaps bytes] + [inEncaps length]);
                            result = OBJECTPRX->begin_ice_invoke(fromNSString(operation),
                                                                 (Ice::OperationMode)mode,
                                                                 inP,
                                                                 ctx,
                                                                 cb);
                        },
                        context,
                        ^(const Ice::AsyncResultPtr& result) {
                            std::pair<const ::Ice::Byte*, const ::Ice::Byte*> outP;
                            BOOL ret__ = OBJECTPRX->___end_ice_invoke(outP, result);
                            NSMutableData* outEncaps =
                                [NSMutableData dataWithBytes:outP.first length:(outP.second - outP.first)];
                            if(response)
                            {
                                response(ret__, outEncaps);
                            }
                        },
                        exception, sent, self);
}
-(BOOL) end_ice_invoke:(NSMutableData**)outEncaps result:(id<ICEAsyncResult>)result

{
    __block BOOL ret__;
    endCppCall(^(const Ice::AsyncResultPtr& r)
               {
                   std::vector<Ice::Byte> outP;
                   ret__ = OBJECTPRX->end_ice_invoke(outP, r);
                   *outEncaps = [NSMutableData dataWithBytes:&outP[0] length:outP.size()];
               }, result);
    return ret__;
}

-(ICEIdentity*) ice_getIdentity
{
    return [ICEIdentity identityWithIdentity:OBJECTPRX->ice_getIdentity()];
}
-(id) ice_identity:(ICEIdentity*)identity
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_identity([identity identity])];
}
-(ICEMutableContext*) ice_getContext
{
    return [toNSDictionary(OBJECTPRX->ice_getContext()) autorelease];
}
-(id) ice_context:(ICEContext*)context
{
    Ice::Context ctx;
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_context(fromNSDictionary(context, ctx))];
}
-(NSMutableString*) ice_getFacet
{
    return [toNSMutableString(OBJECTPRX->ice_getFacet()) autorelease];
}
-(id) ice_facet:(NSString*)facet
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_facet(fromNSString(facet))];
}
-(NSMutableString*) ice_getAdapterId
{
    return [toNSMutableString(OBJECTPRX->ice_getAdapterId()) autorelease];
}

-(ICEMutableEndpointSeq*) ice_getEndpoints
{
    return [toNSArray(OBJECTPRX->ice_getEndpoints()) autorelease];
}

-(id) ice_endpoints:(ICEEndpointSeq*)endpoints
{
    Ice::EndpointSeq cxxEndpoints;
    fromNSArray(endpoints, cxxEndpoints);
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_endpoints(cxxEndpoints)];
}

-(id) ice_adapterId:(NSString*)adapterId
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_adapterId(fromNSString(adapterId))];
}

-(ICEInt) ice_getLocatorCacheTimeout
{
    return OBJECTPRX->ice_getLocatorCacheTimeout();
}
-(id) ice_locatorCacheTimeout:(ICEInt)timeout
{
    NSException* nsex;
    try
    {
        return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_locatorCacheTimeout(timeout)];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(BOOL) ice_isConnectionCached
{
    return OBJECTPRX->ice_isConnectionCached();
}
-(id) ice_connectionCached:(BOOL)cached
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_connectionCached(cached)];
}
-(ICEEndpointSelectionType) ice_getEndpointSelection
{
    return (ICEEndpointSelectionType)OBJECTPRX->ice_getEndpointSelection();
}
-(id) ice_endpointSelection:(ICEEndpointSelectionType)type
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_endpointSelection((Ice::EndpointSelectionType)type)];
}
-(BOOL) ice_isSecure
{
    return OBJECTPRX->ice_isSecure();
}
-(id) ice_secure:(BOOL)secure
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_secure(secure)];
}

-(ICEEncodingVersion*) ice_getEncodingVersion
{
    return [ICEEncodingVersion encodingVersionWithEncodingVersion:OBJECTPRX->ice_getEncodingVersion()];
}

-(id) ice_encodingVersion:(ICEEncodingVersion*)encoding;
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_encodingVersion([encoding encodingVersion])];
}

-(BOOL) ice_isPreferSecure
{
    return OBJECTPRX->ice_isPreferSecure();
}
-(id) ice_preferSecure:(BOOL)preferSecure
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_preferSecure(preferSecure)];
}
-(id<ICERouterPrx>) ice_getRouter
{
    return (id<ICERouterPrx>)[ICERouterPrx objectPrxWithObjectPrx__:OBJECTPRX->ice_getRouter()];
}
-(id) ice_router:(id<ICERouterPrx>)router
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_router(
            Ice::RouterPrx::uncheckedCast(Ice::ObjectPrx([(ICEObjectPrx*)router objectPrx__])))];
}
-(id<ICELocatorPrx>) ice_getLocator
{
    return (id<ICELocatorPrx>)[ICELocatorPrx objectPrxWithObjectPrx__:OBJECTPRX->ice_getLocator()];
}
-(id) ice_locator:(id<ICELocatorPrx>)locator
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_locator(
            Ice::LocatorPrx::uncheckedCast(Ice::ObjectPrx([(ICEObjectPrx*)locator objectPrx__])))];
}
-(BOOL) ice_isCollocationOptimized
{
    return OBJECTPRX->ice_isCollocationOptimized();
}
-(id) ice_collocationOptimized:(BOOL)collocOptimized
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_collocationOptimized(collocOptimized)];
}
-(ICEInt) ice_getInvocationTimeout
{
    return OBJECTPRX->ice_getInvocationTimeout();
}
-(id) ice_invocationTimeout:(ICEInt)timeout
{
    NSException* nsex;
    try
    {
        return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_invocationTimeout(timeout)];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(id) ice_twoway
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_twoway()];
}
-(BOOL) ice_isTwoway
{
    return OBJECTPRX->ice_isTwoway();
}
-(id) ice_oneway
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_oneway()];
}
-(BOOL) ice_isOneway
{
    return OBJECTPRX->ice_isOneway();
}
-(id) ice_batchOneway
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_batchOneway()];
}
-(BOOL) ice_isBatchOneway
{
    return OBJECTPRX->ice_isBatchOneway();
}
-(id) ice_datagram
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_datagram()];
}
-(BOOL) ice_isDatagram
{
    return OBJECTPRX->ice_isDatagram();
}
-(id) ice_batchDatagram
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_batchDatagram()];
}
-(BOOL) ice_isBatchDatagram
{
    return OBJECTPRX->ice_isBatchDatagram();
}
-(id) ice_compress:(BOOL)compress
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_compress(compress)];
}
-(id) ice_timeout:(int)timeout
{
    NSException* nsex;
    try
    {
        return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_timeout(timeout)];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(id) ice_connectionId:(NSString*)connectionId
{
    return [[self class] objectPrxWithObjectPrx__:OBJECTPRX->ice_connectionId(fromNSString(connectionId))];
}
-(id<ICEConnection>) ice_getConnection
{
    NSException* nsex = nil;
    try
    {
        return [ICEConnection localObjectWithCxxObject:OBJECTPRX->ice_getConnection().get()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(id<ICEAsyncResult>) begin_ice_getConnection
{
    return beginCppCall(^(Ice::AsyncResultPtr& result)
                        {
                            result = OBJECTPRX->begin_ice_getConnection();
                        }, self);
}
-(id<ICEAsyncResult>) begin_ice_getConnection:(void(^)(id<ICEConnection>))response
                                    exception:(void(^)(ICEException*))exception
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::CallbackPtr& cb)
                        {
                            result = OBJECTPRX->begin_ice_getConnection(cb);
                        },
                        ^(const Ice::AsyncResultPtr& result) {
                            id<ICEConnection> ret__ =
                                [ICEConnection localObjectWithCxxObject:OBJECTPRX->end_ice_getConnection(result).get()];
                            if(response)
                            {
                                response(ret__);
                            }
                        },
                        exception, nil, self);
}
-(id<ICEConnection>) end_ice_getConnection:(id<ICEAsyncResult>)result
{
    __block id<ICEConnection> ret__;
    endCppCall(^(const Ice::AsyncResultPtr& r)
               {
                   ret__ = [ICEConnection localObjectWithCxxObject:OBJECTPRX->end_ice_getConnection(r).get()];
               }, result);
    return ret__;
}

-(id<ICEConnection>) ice_getCachedConnection
{
    NSException* nsex = nil;
    try
    {
        return [ICEConnection localObjectWithCxxObject:OBJECTPRX->ice_getCachedConnection().get()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(void) ice_flushBatchRequests
{
    NSException* nsex = nil;
    try
    {
        OBJECTPRX->ice_flushBatchRequests();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(id<ICEAsyncResult>) begin_ice_flushBatchRequests
{
    return beginCppCall(^(Ice::AsyncResultPtr& result)
                        {
                            result = OBJECTPRX->begin_ice_flushBatchRequests();
                        }, self);
}
-(id<ICEAsyncResult>) begin_ice_flushBatchRequests:(void(^)(ICEException*))exception
{
    return [self begin_ice_flushBatchRequests:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_ice_flushBatchRequests:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::CallbackPtr& cb)
                        {
                            result = OBJECTPRX->begin_ice_flushBatchRequests(cb);
                        },
                        ^(const Ice::AsyncResultPtr& result) {
                            OBJECTPRX->end_ice_flushBatchRequests(result);
                        },
                        exception, sent, self);
}
-(void) end_ice_flushBatchRequests:(id<ICEAsyncResult>)result
{
    endCppCall(^(const Ice::AsyncResultPtr& r)
               {
                   OBJECTPRX->end_ice_flushBatchRequests(r);
               }, result);
}
@end
