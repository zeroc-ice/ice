//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <CommunicatorI.h>
#import <PropertiesI.h>
#import <ProxyI.h>
#import <IdentityI.h>
#import <LoggerI.h>
#import <ObjectAdapterI.h>
#import <Util.h>
#import <StreamI.h>
#import <SlicedDataI.h>
#import <ImplicitContextI.h>
#import <ProxyI.h>
#import <LocalObjectI.h>
#import <ObjectI.h>
#import <ValueFactoryI.h>

#include <Ice/Router.h>
#include <Ice/Locator.h>

#import <objc/Ice/Router.h>
#import <objc/Ice/Locator.h>
#import <objc/Ice/Initialize.h>

#import <objc/runtime.h>

#define COMMUNICATOR dynamic_cast<Ice::Communicator*>(static_cast<IceUtil::Shared*>(cxxObject_))

@interface ICEInternalPrefixTable(ICEInternal)
+(NSDictionary*) newPrefixTable;
@end

@implementation ICEInternalPrefixTable(ICEInternal)
+(NSDictionary*) newPrefixTable
{
    NSMutableDictionary* prefixTable = [[NSMutableDictionary alloc] init];
    ICEInternalPrefixTable* table = [[ICEInternalPrefixTable alloc] init];
    unsigned int count;
    Method* methods = class_copyMethodList([ICEInternalPrefixTable class], &count);
    for(unsigned int i = 0; i < count; ++i)
    {
        SEL selector = method_getName(methods[i]);
        const char* methodName = sel_getName(selector);
        if(strncmp(methodName, "addPrefixes_C", 13) == 0)
        {
            [table performSelector:selector withObject:prefixTable];
        }
    }
    free(methods);
    [table release];
    return prefixTable;
}
@end

@implementation ICECommunicator
-(void)setup:(ICEInitializationData*)initData
{
    if(initData.prefixTable_)
    {
        prefixTable_ = [initData.prefixTable_ retain];
    }
    else
    {
        prefixTable_ = [ICEInternalPrefixTable newPrefixTable];
    }
    adminFacets_ = [[NSMutableDictionary alloc] init];

    valueFactoryManager_ = [[ICEValueFactoryManager alloc] init:COMMUNICATOR prefixTable:prefixTable_];
    objectFactories_ = [[NSMutableDictionary alloc] init];
}
-(void) dealloc
{
    [valueFactoryManager_ release];
    [objectFactories_ release];
    [prefixTable_ release];
    [adminFacets_ release];
    [super dealloc];
}
-(Ice::Communicator*) communicator
{
    return COMMUNICATOR;
}
-(NSDictionary*) getPrefixTable
{
    return prefixTable_;
}

//
// Methods from @protocol ICECommunicator
//

-(void) destroy
{
    NSException* nsex = nil;
    try
    {
        COMMUNICATOR->destroy();
        @synchronized(adminFacets_)
        {
            [adminFacets_ removeAllObjects];
        }
        @synchronized(objectFactories_)
        {
            for(NSString* k in objectFactories_)
            {
                [[objectFactories_ objectForKey:k] destroy];
            }
        }
        return;
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

-(void) shutdown
{
    NSException* nsex = nil;
    try
    {
        COMMUNICATOR->shutdown();
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

-(void) waitForShutdown
{
    NSException* nsex = nil;
    try
    {
        COMMUNICATOR->waitForShutdown();
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

-(BOOL) isShutdown
{
    NSException* nsex = nil;
    try
    {
        return COMMUNICATOR->isShutdown();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return NO; // Keep the compiler happy.
}

-(id<ICEObjectPrx>) stringToProxy:(NSString*)str
{
    NSException* nsex = nil;
    try
    {
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:COMMUNICATOR->stringToProxy(fromNSString(str))];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(NSMutableString*)proxyToString:(id<ICEObjectPrx>)obj
{
    NSException* nsex = nil;
    try
    {
        return [toNSMutableString(COMMUNICATOR->proxyToString([(ICEObjectPrx*)obj iceObjectPrx])) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(id<ICEObjectPrx>)propertyToProxy:(NSString*)property
{
    NSException* nsex = nil;
    try
    {
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:COMMUNICATOR->propertyToProxy(fromNSString(property))];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(NSMutableDictionary*)proxyToProperty:(id<ICEObjectPrx>)prx property:(NSString*)property
{
    NSException* nsex = nil;
    try
    {
        return [toNSDictionary(COMMUNICATOR->proxyToProperty([(ICEObjectPrx*)prx iceObjectPrx],
                                                             fromNSString(property))) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(ICEIdentity*) stringToIdentity:(NSString*)str
{
    return [ICEUtil stringToIdentity:str];
}

-(NSMutableString*) identityToString:(ICEIdentity*)ident
{
    NSException* nsex = nil;
    try
    {
        return [toNSMutableString(COMMUNICATOR->identityToString([ident identity])) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(id<ICEObjectAdapter>) createObjectAdapter:(NSString*)name
{
    NSException* nsex = nil;
    try
    {
        ICEObjectAdapter* adapter = [ICEObjectAdapter localObjectWithCxxObject:
                                                          COMMUNICATOR->createObjectAdapter(
                                                              fromNSString(name)).get()];
        return adapter;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(id<ICEObjectAdapter>) createObjectAdapterWithEndpoints:(NSString*)name endpoints:(NSString*)endpoints
{
    NSException* nsex = nil;
    try
    {
        ICEObjectAdapter* adapter = [ICEObjectAdapter localObjectWithCxxObject:
                                                          COMMUNICATOR->createObjectAdapterWithEndpoints(
                                                              fromNSString(name), fromNSString(endpoints)).get()];
        return adapter;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(id<ICEObjectAdapter>) createObjectAdapterWithRouter:(NSString*)name router:(id<ICERouterPrx>)rtr
{
    NSException* nsex = nil;
    try
    {
        Ice::RouterPrx router = Ice::RouterPrx::uncheckedCast(Ice::ObjectPrx([(ICEObjectPrx*)rtr iceObjectPrx]));
        ICEObjectAdapter* adapter = [ICEObjectAdapter localObjectWithCxxObject:
                                                          COMMUNICATOR->createObjectAdapterWithRouter(
                                                              fromNSString(name), router).get()];
        return adapter;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(void) addObjectFactory:(id<ICEObjectFactory>)factory sliceId:(NSString*)sliceId
{
    @synchronized(objectFactories_)
    {
        [objectFactories_ setObject:factory forKey:sliceId];
    }
    ICEValueFactory valueFactoryWrapper = ^(NSString* s)
    {
        return [factory create:s];
    };
    [valueFactoryManager_ add:valueFactoryWrapper sliceId:sliceId];
}

-(id<ICEObjectFactory>) findObjectFactory:(NSString*)sliceId
{
    @synchronized(objectFactories_)
    {
        return [objectFactories_ objectForKey:sliceId];
    }
    return nil; // Keep the compiler happy.
}

-(id<ICEValueFactoryManager>) getValueFactoryManager
{
    return valueFactoryManager_;
}

-(id<ICEImplicitContext>) getImplicitContext
{
    return [ICEImplicitContext implicitContextWithImplicitContext:COMMUNICATOR->getImplicitContext().get()];
}

-(id<ICEProperties>) getProperties
{
    NSException* nsex = nil;
    try
    {
        return [ICEProperties localObjectWithCxxObject:COMMUNICATOR->getProperties().get()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(id<ICELogger>) getLogger
{
    NSException* nsex = nil;
    try
    {
        return [ICELoggerWrapper loggerWithLogger:COMMUNICATOR->getLogger().get()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(id<ICEINSTRUMENTATIONCommunicatorObserver>) getObserver
{
    return nil;
}

-(id<ICERouterPrx>) getDefaultRouter
{
    NSException* nsex = nil;
    try
    {
        return (id<ICERouterPrx>)[ICERouterPrx iceObjectPrxWithObjectPrx:COMMUNICATOR->getDefaultRouter()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(void) setDefaultRouter:(id<ICERouterPrx>)rtr
{
    NSException* nsex = nil;
    try
    {
        COMMUNICATOR->setDefaultRouter(Ice::RouterPrx::uncheckedCast(Ice::ObjectPrx([(ICEObjectPrx*)rtr iceObjectPrx])));
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

-(id<ICELocatorPrx>) getDefaultLocator
{
    NSException* nsex = nil;
    try
    {
        return (id<ICELocatorPrx>)[ICELocatorPrx iceObjectPrxWithObjectPrx:COMMUNICATOR->getDefaultLocator()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(void) setDefaultLocator:(id<ICELocatorPrx>)loc
{
    NSException* nsex = nil;
    try
    {
        COMMUNICATOR->setDefaultLocator(Ice::LocatorPrx::uncheckedCast(
                                            Ice::ObjectPrx([(ICEObjectPrx*)loc iceObjectPrx])));
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
-(id<ICEPluginManager>) getPluginManager
{
    @throw [ICEFeatureNotSupportedException featureNotSupportedException:__FILE__ line:__LINE__];
}
-(void) flushBatchRequests:(ICECompressBatch)compress
{
    NSException* nsex = nil;
    try
    {
        COMMUNICATOR->flushBatchRequests((Ice::CompressBatch)compress);
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
-(id<ICEAsyncResult>) begin_flushBatchRequests:(ICECompressBatch)compress
{
    return beginCppCall(^(Ice::AsyncResultPtr& result)
                        {
                            result = COMMUNICATOR->begin_flushBatchRequests((Ice::CompressBatch)compress);
                        });
}
-(id<ICEAsyncResult>) begin_flushBatchRequests:(ICECompressBatch)compress exception:(void(^)(ICEException*))exception
{
    return [self begin_flushBatchRequests:compress exception:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_flushBatchRequests:(ICECompressBatch)compress
                                     exception:(void(^)(ICEException*))exception
                                          sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::CallbackPtr& cb)
                        {
                            result = COMMUNICATOR->begin_flushBatchRequests((Ice::CompressBatch)compress, cb);
                        },
                        ^(const Ice::AsyncResultPtr& result) {
                            COMMUNICATOR->end_flushBatchRequests(result);
                        },
                        exception, sent);
}
-(void) end_flushBatchRequests:(id<ICEAsyncResult>)result
{
    endCppCall(^(const Ice::AsyncResultPtr& r)
               {
                   COMMUNICATOR->end_flushBatchRequests(r);
               }, result);
}
-(id<ICEObjectPrx>) createAdmin:(id<ICEObjectAdapter>)adapter adminId:(ICEIdentity*)adminId
{
    NSException* nsex;
    try
    {
        Ice::ObjectAdapterPtr adminAdapter = [(ICEObjectAdapter*)adapter adapter];
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:COMMUNICATOR->createAdmin(adminAdapter, [adminId identity])];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(id<ICEObjectPrx>) getAdmin
{
    NSException* nsex;
    try
    {
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:COMMUNICATOR->getAdmin()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(void) addAdminFacet:(ICEObject*)servant facet:(NSString*)facet
{
    NSException* nsex;
    try
    {
        COMMUNICATOR->addAdminFacet([servant iceObject], fromNSString(facet));
        @synchronized(adminFacets_)
        {
            [adminFacets_ setObject:servant forKey:facet];
        }
        return;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(ICEObject*) removeAdminFacet:(NSString*)facet
{
    NSException* nsex;
    try
    {
        @synchronized(adminFacets_)
        {
            [adminFacets_ removeObjectForKey:facet];
        }
        return [toObjC(COMMUNICATOR->removeAdminFacet(fromNSString(facet))) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(ICEObject*) findAdminFacet:(NSString*)facet
{
    NSException* nsex;
    try
    {
        @synchronized(adminFacets_)
        {
            ICEObject* obj = [adminFacets_ objectForKey:facet];
            if(obj != nil)
            {
                return [[obj retain] autorelease];
            }
            obj = toObjC(COMMUNICATOR->findAdminFacet(fromNSString(facet)));
            if(obj != nil)
            {
                [adminFacets_ setObject:obj forKey:facet];
            }
            return [obj autorelease];
        }
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(ICEMutableFacetMap*) findAllAdminFacets
{
    NSException* nsex;
    try
    {
        ICEMutableFacetMap* facetMap = toNSDictionary(COMMUNICATOR->findAllAdminFacets());
        @synchronized(adminFacets_)
        {
            [adminFacets_ addEntriesFromDictionary:facetMap];
        }
        return [facetMap autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
@end
