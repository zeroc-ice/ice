// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcConfig.h"
#import "IceObjcObjectAdapter.h"
#import "IceObjcUtil.h"
#import "IceObjcObjectPrx.h"
#import "IceObjcInputStream.h"
#import "IceObjcConnection.h"

namespace
{
    class DefaultServant : public Ice::BlobjectArrayAsync
    {
    public:

        DefaultServant(id<ICEObjectAdapterFacade> facade): _facade(facade)
        {
        }

        ~DefaultServant()
        {
            [_facade facadeRemoved];
        }

        virtual void
        ice_invokeAsync(std::pair<const Byte*, const Byte*> inEncaps,
                        std::function<void(bool, const std::pair<const Byte*, const Byte*>&)> response,
                        std::function<void(std::exception_ptr)> error,
                        const Ice::Current& current)
        {
            void(^responseCallback)(bool, const void*, size_t) = ^(bool ok, const void* outParams, size_t outSize) {
                const Ice::Byte* start = reinterpret_cast<const Ice::Byte*>(outParams);
                response(ok, std::make_pair(start, start + outSize));
            };

            void(^exceptionCallback)(ICERuntimeException*) = ^(ICERuntimeException* e) {
                error(convertException(e));
            };

            // Copy the bytes
            std::vector<Ice::Byte> inBytes(inEncaps.first, inEncaps.second);

            [_facade facadeInvoke:[[ICEInputStream alloc] initWithBytes:std::move(inBytes)]
                         con:[[ICEConnection alloc] initWithCppConnection:current.con]
                        name:toNSString(current.id.name) category:toNSString(current.id.category)
                       facet:toNSString(current.facet)
                   operation:toNSString(current.operation)
                        mode:static_cast<uint8_t>(current.mode)
                     context:toNSDictionary(current.ctx)
                   requestId:current.requestId
               encodingMajor:current.encoding.major
               encodingMinor:current.encoding.minor
                    response:responseCallback
                   exception:exceptionCallback];
        }

    private:
        id<ICEObjectAdapterFacade> _facade;
    };
}

@implementation ICEObjectAdapter

-(instancetype) initWithCppObjectAdapter:(std::shared_ptr<Ice::ObjectAdapter>)objectAdapter
{
    self = [super initWithLocalObject:objectAdapter.get()];
    if(self)
    {
        self->_objectAdapter = objectAdapter;
    }
    return self;
}

-(NSString*) getName
{
    return toNSString(_objectAdapter->getName());
}

-(BOOL) activate:(NSError* _Nullable * _Nullable)error
{
    try
    {
        _objectAdapter->activate();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(BOOL) hold:(NSError* _Nullable * _Nullable)error
{
    try
    {
        _objectAdapter->hold();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(BOOL) waitForHold:(NSError* _Nullable * _Nullable)error
{
    try
    {
        _objectAdapter->waitForHold();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(void) deactivate
{
    _objectAdapter->deactivate();
}

-(void) waitForDeactivate
{

    _objectAdapter->deactivate();
}

-(BOOL) isDeactivated
{
    return _objectAdapter->isDeactivated();
}

-(void) destroy
{
    _objectAdapter->destroy();
}

-(nullable ICEObjectPrx*) createProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto prx = _objectAdapter->createProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(nullable ICEObjectPrx*) createDirectProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto prx = _objectAdapter->createDirectProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(nullable ICEObjectPrx*) createIndirectProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto prx = _objectAdapter->createIndirectProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(NSArray<ICEEndpoint*>*) getEndpoints
{
    return toNSArray(_objectAdapter->getEndpoints());
}

-(BOOL) refreshPublishedEndpoints:(NSError* _Nullable * _Nullable)error
{
    try
    {
        _objectAdapter->refreshPublishedEndpoints();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(NSArray<ICEEndpoint*>*) getPublishedEndpoints
{
    return toNSArray(_objectAdapter->getPublishedEndpoints());
}

-(BOOL) setPublishedEndpoints:(NSArray<ICEEndpoint*>*)newEndpoints error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        Ice::EndpointSeq endpts;
        fromNSArray(newEndpoints, endpts);

        _objectAdapter->setPublishedEndpoints(endpts);
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(void) registerFacade:(id<ICEObjectAdapterFacade>)facade
{
    auto servant = std::make_shared<DefaultServant>(facade);
    _objectAdapter->addDefaultServant(servant, "");
}

@end
