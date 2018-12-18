// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

#define OBJECTPRX ((IceProxy::Ice::Object*)objectPrx_)
#define ASYNCRESULT ((Ice::AsyncResult*)asyncResult_)

namespace
{

class BeginInvokeAsyncCallback : public IceUtil::Shared
{
public:

BeginInvokeAsyncCallback(const Ice::CommunicatorPtr& communicator,
                         void (^completed)(id<ICEInputStream>, BOOL),
                         void (^exception)(ICEException*),
                         void (^sent)(BOOL),
                         BOOL returnsData) :
    _communicator(communicator),
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

void response(bool ok, const std::pair<const Ice::Byte*, const Ice::Byte*>& outParams)
{
    id<ICEInputStream> is = [[ICEInputStream alloc] initWithCxxCommunicator:_communicator.get() data:outParams];
    NSException* exception = nil;
    @autoreleasepool
    {
        @try
        {
            if(_returnsData)
            {
                _completed(is, ok);
            }
            else if(outParams.first != outParams.second)
            {
                if(ok)
                {
                    [is skipEmptyEncapsulation];
                }
                else
                {
                    @try
                    {
                        [is startEncapsulation];
                        [is throwException];
                    }
                    @catch(ICEUserException *ex)
                    {
                        [is endEncapsulation];
                        @throw [ICEUnknownUserException unknownUserException:__FILE__ line:__LINE__ unknown:[ex ice_id]];
                    }
                }
                _completed(nil, ok);
            }
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

void exception(const Ice::Exception& ex)
{
    NSException* exception = nil;
    @autoreleasepool
    {
        @try
        {
            @throw toObjCException(ex);
        }
        @catch(ICEException* ex)
        {
            if(_exception)
            {
                @try
                {
                    _exception(ex);
                }
                @catch(id e)
                {
                    exception = [e retain];
                }
            }
        }
    }
    if(exception != nil)
    {
        rethrowCxxException(exception, true); // True = release the exception.
    }
}

void sent(bool sentSynchronously)
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
            _sent(sentSynchronously);
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

const Ice::CommunicatorPtr _communicator;
void (^_completed)(id<ICEInputStream>, BOOL);
void (^_exception)(ICEException*);
void (^_sent)(BOOL);
BOOL _returnsData;

};

};

@implementation ICEAsyncResult
-(ICEAsyncResult*) initWithAsyncResult:(const Ice::AsyncResultPtr&)arg
                               operation:(NSString*)op
                                   proxy:(id<ICEObjectPrx>)p
{
    self = [super init];
    if(!self)
    {
        return nil;
    }

    asyncResult_ = arg.get();
    ASYNCRESULT->__incRef();
    operation_ = [op retain];
    proxy_ = [p retain];
    return self;
}

-(Ice::AsyncResult*) asyncResult
{
    return ASYNCRESULT;
}

-(void) dealloc
{
    ASYNCRESULT->__decRef();
    asyncResult_ = 0;
    [operation_ release];
    [proxy_ release];
    [super dealloc];
}

+(ICEAsyncResult*) asyncResultWithAsyncResult:(const Ice::AsyncResultPtr&)arg
{
    return [self asyncResultWithAsyncResult:arg operation:nil proxy:nil];
}
+(ICEAsyncResult*) asyncResultWithAsyncResult:(const Ice::AsyncResultPtr&)arg
                                      operation:(NSString*)op
                                          proxy:(id<ICEObjectPrx>)p
{
    if(!arg)
    {
        return nil;
    }
    else
    {
        return [[[self alloc] initWithAsyncResult:arg operation:op proxy:p] autorelease];
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

-(ICEObjectPrx*) iceInitWithObjectPrx:(const Ice::ObjectPrx&)arg
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    communicator_ = [ICECommunicator localObjectWithCxxObjectNoAutoRelease:arg->ice_getCommunicator().get()];
    objectPrx_ = arg.get();
    OBJECTPRX->__incRef();
    return self;
}

-(IceProxy::Ice::Object*) iceObjectPrx
{
    return (IceProxy::Ice::Object*)objectPrx_;
}

-(void) dealloc
{
    OBJECTPRX->__decRef();
    objectPrx_ = 0;
    [communicator_ release];
    [super dealloc];
}

+(ICEObjectPrx*) iceObjectPrxWithObjectPrx:(const Ice::ObjectPrx&)arg
{
    if(!arg)
    {
        return nil;
    }
    else
    {
        return [[[self alloc] iceInitWithObjectPrx:arg] autorelease];
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
            return [[[self alloc] iceInitWithObjectPrx:[(ICEObjectPrx*)proxy iceObjectPrx]] autorelease];
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
        else if([(ICEObjectPrx*)proxy conformsToProtocol:[self iceProtocol]] ||
                [proxy ice_isA:[self ice_staticId]])
        {
            return [[[self alloc] iceInitWithObjectPrx:[(ICEObjectPrx*)proxy iceObjectPrx]] autorelease];
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
        else if([(ICEObjectPrx*)proxy conformsToProtocol:[self iceProtocol]] ||
                [proxy ice_isA:[self ice_staticId] context:context])
        {
            return [[[self alloc] iceInitWithObjectPrx:[(ICEObjectPrx*)proxy iceObjectPrx]] autorelease];
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

+(Protocol*) iceProtocol
{
    return objc_getProtocol(class_getName([self class]));
}

-(id<ICEOutputStream>) iceCreateOutputStream
{
    NSException* nsex = nil;
    try
    {
        return [[ICEOutputStream alloc] initWithCxxCommunicator:OBJECTPRX->ice_getCommunicator().get()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(void) iceCheckAsyncTwowayOnly:(NSString*)operation
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

-(void) iceInvoke:(NSString*)operation
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
        os = [self iceCreateOutputStream];
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

        BOOL ok = YES; // Keep the compiler happy.
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

        std::pair<const Ice::Byte*, const Ice::Byte*> p(&outParams[0], &outParams[0] + outParams.size());
        ICEInputStream<ICEInputStream>* is;
        is = [[ICEInputStream alloc] initWithCxxCommunicator:OBJECTPRX->ice_getCommunicator().get() data:p];
        @try
        {
            if(unmarshal)
            {
                unmarshal(is, ok);
            }
            else if(!outParams.empty())
            {
                if(ok)
                {
                    [is skipEmptyEncapsulation];
                }
                else
                {
                    @try
                    {
                        [is startEncapsulation];
                        [is throwException];
                    }
                    @catch(ICEUserException* ex)
                    {
                        [is endEncapsulation];
                        @throw [ICEUnknownUserException unknownUserException:__FILE__ line:__LINE__ unknown:[ex ice_id]];
                    }
                }
            }
        }
        @catch(id e)
        {
            nsex = e;
        }
        [is release];
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

    NSAssert(os == nil, @"output stream not cleared");
}

-(id<ICEAsyncResult>) iceI_begin_invoke:(NSString*)operation
                                mode:(ICEOperationMode)mode
                              format:(ICEFormatType)format
                             marshal:(void(^)(id<ICEOutputStream>))marshal
                         returnsData:(BOOL)returnsData
                             context:(ICEContext*)context
{
    if(returnsData)
    {
        [self iceCheckAsyncTwowayOnly:operation];
    }

    ICEOutputStream<ICEOutputStream>* os = nil;
    if(marshal)
    {
        os = [self iceCreateOutputStream];
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
        return [ICEAsyncResult asyncResultWithAsyncResult:r operation:operation proxy:self];
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

-(id<ICEAsyncResult>) iceI_begin_invoke:(NSString*)operation
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
        [self iceCheckAsyncTwowayOnly:operation];
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
        os = [self iceCreateOutputStream];
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

        Ice::Callback_Object_ice_invokePtr cb = Ice::newCallback_Object_ice_invoke(
            new BeginInvokeAsyncCallback(OBJECTPRX->ice_getCommunicator(), completed, exception, sent, returnsData),
            &BeginInvokeAsyncCallback::response,
            &BeginInvokeAsyncCallback::exception,
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
        return [ICEAsyncResult asyncResultWithAsyncResult:r operation:operation proxy:self];
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
-(id<ICEAsyncResult>) iceI_begin_invoke:(NSString*)op
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
    return [self iceI_begin_invoke:op
                 mode:mode
                 format:format
                 marshal:marshal
                 returnsData:NO
                 completed:completed
                 response:YES
                 exception:exception
                 sent:sent
                 context:ctx];
}

-(id<ICEAsyncResult>) iceI_begin_invoke:(NSString*)op
                                mode:(ICEOperationMode)mode
                              format:(ICEFormatType)format
                             marshal:(void(^)(id<ICEOutputStream>))marshal
                           completed:(void(^)(id<ICEInputStream>, BOOL))completed
                            response:(BOOL)response
                           exception:(void(^)(ICEException*))exception
                                sent:(void(^)(BOOL))sent
                             context:(ICEContext*)ctx
{
    return [self iceI_begin_invoke:op
                 mode:mode
                 format:format
                 marshal:marshal
                 returnsData:YES
                 completed:completed
                 response:response
                 exception:exception
                 sent:sent context:ctx];
}

-(void)iceI_end_invoke:(NSString*)operation unmarshal:(ICEUnmarshalCB)unmarshal result:(id<ICEAsyncResult>)r
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

    NSException* nsex = nil;
    try
    {
        std::pair<const Ice::Byte*, const Ice::Byte*> outParams;
        BOOL ok = OBJECTPRX->_iceI_end_ice_invoke(outParams, [result asyncResult]);

        ICEInputStream* is;
        is = [[ICEInputStream alloc] initWithCxxCommunicator:OBJECTPRX->ice_getCommunicator().get() data:outParams];
        @try
        {
            if(unmarshal)
            {
                unmarshal(is, ok);
            }
            else if(outParams.first != outParams.second)
            {
                if(ok)
                {
                    [is skipEmptyEncapsulation];
                }
                else
                {
                    @try
                    {
                        [is startEncapsulation];
                        [is throwException];
                    }
                    @catch(ICEUserException* ex)
                    {
                        [is endEncapsulation];
                        @throw [ICEUnknownUserException unknownUserException:__FILE__ line:__LINE__ unknown:[ex ice_id]];
                    }
                }
            }
        }
        @catch(id e)
        {
            nsex = e;
        }
        [is release];
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

-(id) copyWithZone:(NSZone *)__unused zone
{
    return [self retain];
}

-(NSUInteger) hash
{
    return (NSUInteger)OBJECTPRX->_hash();
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
    return *OBJECTPRX == *[o_ iceObjectPrx];
}

-(NSComparisonResult) compareIdentity:(id<ICEObjectPrx>)aProxy
{
    IceProxy::Ice::Object* lhs = OBJECTPRX;
    IceProxy::Ice::Object* rhs = [(ICEObjectPrx*)aProxy iceObjectPrx];
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
    IceProxy::Ice::Object* rhs = [(ICEObjectPrx*)aProxy iceObjectPrx];
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
    return [[communicator_ retain] autorelease];
}

-(NSMutableString*) ice_toString
{
    return [toNSMutableString(OBJECTPRX->ice_toString()) autorelease];
}

-(BOOL) ice_isA:(NSString*)typeId
{
    __block BOOL ret;
    cppCall(^ { ret = OBJECTPRX->ice_isA(fromNSString(typeId)); });
    return ret;
}
-(BOOL) ice_isA:(NSString*)typeId context:(ICEContext*)context
{
    __block BOOL ret;
    cppCall(^(const Ice::Context& ctx) { ret = OBJECTPRX->ice_isA(fromNSString(typeId), ctx); }, context);
    return ret;
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
                            BOOL ret = OBJECTPRX->end_ice_isA(result);
                            if(response)
                            {
                                response(ret);
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
                            BOOL ret = OBJECTPRX->end_ice_isA(result);
                            if(response)
                            {
                                response(ret);
                            }
                        },
                        exception, sent, self);
}
-(BOOL) end_ice_isA:(id<ICEAsyncResult>)result
{
    __block BOOL ret;
    endCppCall(^(const Ice::AsyncResultPtr& r) { ret = OBJECTPRX->end_ice_isA(r); }, result);
    return ret;
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
    __block NSMutableArray* ret;
    cppCall(^ { ret = [toNSArray(OBJECTPRX->ice_ids()) autorelease]; });
    return ret;
}
-(NSMutableArray*) ice_ids:(ICEContext*)context
{
    __block NSMutableArray* ret;
    cppCall(^(const Ice::Context& ctx) { ret = [toNSArray(OBJECTPRX->ice_ids(ctx)) autorelease]; }, context);
    return ret;
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
                            NSMutableArray* ret = [toNSArray(OBJECTPRX->end_ice_ids(result)) autorelease];
                            if(response)
                            {
                                response(ret);
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
                            NSMutableArray* ret = [toNSArray(OBJECTPRX->end_ice_ids(result)) autorelease];
                            if(response)
                            {
                                response(ret);
                            }
                        },
                        exception, sent, self);
}
-(NSMutableArray*) end_ice_ids:(id<ICEAsyncResult>)result
{
    __block NSMutableArray* ret;
    endCppCall(^(const Ice::AsyncResultPtr& r) { ret = [toNSArray(OBJECTPRX->end_ice_ids(r)) autorelease]; }, result);
    return ret;
}

-(NSMutableString*) ice_id
{
    __block NSMutableString* ret;
    cppCall(^ { ret = [toNSMutableString(OBJECTPRX->ice_id()) autorelease]; });
    return ret;
}
-(NSMutableString*) ice_id:(ICEContext*)context
{
    __block NSMutableString* ret;
    cppCall(^(const Ice::Context& ctx) { ret = [toNSMutableString(OBJECTPRX->ice_id(ctx)) autorelease]; }, context);
    return ret;
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
                            NSString* ret = [toNSString(OBJECTPRX->end_ice_id(result)) autorelease];
                            if(response)
                            {
                                response(ret);
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
                            NSString* ret = [toNSString(OBJECTPRX->end_ice_id(result)) autorelease];
                            if(response)
                            {
                                response(ret);
                            }
                        },
                        exception, sent, self);
}
-(NSMutableString*) end_ice_id:(id<ICEAsyncResult>)result
{
    __block NSMutableString* ret;
    endCppCall(^(const Ice::AsyncResultPtr& r) { ret = [toNSMutableString(OBJECTPRX->end_ice_id(r)) autorelease]; },
               result);
    return ret;
}

-(BOOL) ice_invoke:(NSString*)operation
              mode:(ICEOperationMode)mode
          inEncaps:(NSData*)inEncaps
         outEncaps:(NSMutableData**)outEncaps
{
    return [self end_ice_invoke:outEncaps result:[self begin_ice_invoke:operation mode:mode inEncaps:inEncaps]];
}

-(BOOL) ice_invoke:(NSString*)operation
              mode:(ICEOperationMode)mode
          inEncaps:(NSData*)inEncaps
         outEncaps:(NSMutableData**)outEncaps
           context:(ICEContext*)context
{
    return [self end_ice_invoke:outEncaps
        result:[self begin_ice_invoke:operation mode:mode inEncaps:inEncaps context:context]];
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
                            BOOL ret = OBJECTPRX->_iceI_end_ice_invoke(outP, result);
                            if(response)
                            {
                                NSMutableData* outEncaps =
                                    [NSMutableData dataWithBytes:outP.first length:(outP.second - outP.first)];
                                response(ret, outEncaps);
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
                            BOOL ret = OBJECTPRX->_iceI_end_ice_invoke(outP, result);
                            if(response)
                            {
                                NSMutableData* outEncaps =
                                    [NSMutableData dataWithBytes:outP.first length:(outP.second - outP.first)];
                                response(ret, outEncaps);
                            }
                        },
                        exception, sent, self);
}
-(BOOL) end_ice_invoke:(NSMutableData**)outEncaps result:(id<ICEAsyncResult>)result

{
    __block BOOL ret;
    endCppCall(^(const Ice::AsyncResultPtr& r)
               {
                   std::pair<const ::Ice::Byte*, const ::Ice::Byte*> outP;
                   ret = OBJECTPRX->_iceI_end_ice_invoke(outP, r);
                   *outEncaps = [NSMutableData dataWithBytes:outP.first length:(outP.second - outP.first)];
               }, result);
    return ret;
}

-(ICEIdentity*) ice_getIdentity
{
    return [ICEIdentity identityWithIdentity:OBJECTPRX->ice_getIdentity()];
}
-(id) ice_identity:(ICEIdentity*)identity
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_identity([identity identity])];
}
-(ICEMutableContext*) ice_getContext
{
    return [toNSDictionary(OBJECTPRX->ice_getContext()) autorelease];
}
-(id) ice_context:(ICEContext*)context
{
    Ice::Context ctx;
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_context(fromNSDictionary(context, ctx))];
}
-(NSMutableString*) ice_getFacet
{
    return [toNSMutableString(OBJECTPRX->ice_getFacet()) autorelease];
}
-(id) ice_facet:(NSString*)facet
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_facet(fromNSString(facet))];
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
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_endpoints(cxxEndpoints)];
}

-(id) ice_adapterId:(NSString*)adapterId
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_adapterId(fromNSString(adapterId))];
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
        return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_locatorCacheTimeout(timeout)];
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
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_connectionCached(cached)];
}
-(ICEEndpointSelectionType) ice_getEndpointSelection
{
    return (ICEEndpointSelectionType)OBJECTPRX->ice_getEndpointSelection();
}
-(id) ice_endpointSelection:(ICEEndpointSelectionType)type
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_endpointSelection((Ice::EndpointSelectionType)type)];
}
-(BOOL) ice_isSecure
{
    return OBJECTPRX->ice_isSecure();
}
-(id) ice_secure:(BOOL)secure
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_secure(secure)];
}

-(ICEEncodingVersion*) ice_getEncodingVersion
{
    return [ICEEncodingVersion encodingVersionWithEncodingVersion:OBJECTPRX->ice_getEncodingVersion()];
}

-(id) ice_encodingVersion:(ICEEncodingVersion*)encoding
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_encodingVersion([encoding encodingVersion])];
}

-(BOOL) ice_isPreferSecure
{
    return OBJECTPRX->ice_isPreferSecure();
}
-(id) ice_preferSecure:(BOOL)preferSecure
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_preferSecure(preferSecure)];
}
-(id<ICERouterPrx>) ice_getRouter
{
    return (id<ICERouterPrx>)[ICERouterPrx iceObjectPrxWithObjectPrx:OBJECTPRX->ice_getRouter()];
}
-(id) ice_router:(id<ICERouterPrx>)router
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_router(
            Ice::RouterPrx::uncheckedCast(Ice::ObjectPrx([(ICEObjectPrx*)router iceObjectPrx])))];
}
-(id<ICELocatorPrx>) ice_getLocator
{
    return (id<ICELocatorPrx>)[ICELocatorPrx iceObjectPrxWithObjectPrx:OBJECTPRX->ice_getLocator()];
}
-(id) ice_locator:(id<ICELocatorPrx>)locator
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_locator(
            Ice::LocatorPrx::uncheckedCast(Ice::ObjectPrx([(ICEObjectPrx*)locator iceObjectPrx])))];
}
-(BOOL) ice_isCollocationOptimized
{
    return OBJECTPRX->ice_isCollocationOptimized();
}
-(id) ice_collocationOptimized:(BOOL)collocOptimized
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_collocationOptimized(collocOptimized)];
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
        return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_invocationTimeout(timeout)];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(id) ice_twoway
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_twoway()];
}
-(BOOL) ice_isTwoway
{
    return OBJECTPRX->ice_isTwoway();
}
-(id) ice_oneway
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_oneway()];
}
-(BOOL) ice_isOneway
{
    return OBJECTPRX->ice_isOneway();
}
-(id) ice_batchOneway
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_batchOneway()];
}
-(BOOL) ice_isBatchOneway
{
    return OBJECTPRX->ice_isBatchOneway();
}
-(id) ice_datagram
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_datagram()];
}
-(BOOL) ice_isDatagram
{
    return OBJECTPRX->ice_isDatagram();
}
-(id) ice_batchDatagram
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_batchDatagram()];
}
-(BOOL) ice_isBatchDatagram
{
    return OBJECTPRX->ice_isBatchDatagram();
}
-(id) ice_compress:(BOOL)compress
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_compress(compress)];
}
-(id) ice_getCompress
{
    IceUtil::Optional<bool> compress = OBJECTPRX->ice_getCompress();
    return compress ? @(*compress) : nil;
}
-(id) ice_timeout:(int)timeout
{
    NSException* nsex;
    try
    {
        return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_timeout(timeout)];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(id) ice_getTimeout
{
    IceUtil::Optional<int> timeout = OBJECTPRX->ice_getTimeout();
    return timeout ? @(*timeout) : nil;
}
-(id) ice_fixed:(id<ICEConnection>)connection
{
    Ice::ConnectionPtr con =
        dynamic_cast<Ice::Connection*>(static_cast<IceUtil::Shared*>([(ICELocalObject*)connection cxxObject]));
    if(!con)
    {
        @throw [NSException exceptionWithName:NSInvalidArgumentException
                                       reason:@"invalid connection passed to ice_fixed"
                                     userInfo:nil];
    }
    NSException* nsex = nil;
    try
    {
        return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_fixed(con)];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(id) ice_connectionId:(NSString*)connectionId
{
    return [[self class] iceObjectPrxWithObjectPrx:OBJECTPRX->ice_connectionId(fromNSString(connectionId))];
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
                            id<ICEConnection> ret =
                                [ICEConnection localObjectWithCxxObject:OBJECTPRX->end_ice_getConnection(result).get()];
                            if(response)
                            {
                                response(ret);
                            }
                        },
                        exception, nil, self);
}
-(id<ICEConnection>) end_ice_getConnection:(id<ICEAsyncResult>)result
{
    __block id<ICEConnection> ret;
    endCppCall(^(const Ice::AsyncResultPtr& r)
               {
                   ret = [ICEConnection localObjectWithCxxObject:OBJECTPRX->end_ice_getConnection(r).get()];
               }, result);
    return ret;
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
