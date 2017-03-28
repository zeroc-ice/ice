// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <InitializeI.h>
#import <PropertiesI.h>
#import <CommunicatorI.h>
#import <StreamI.h>
#import <LoggerI.h>
#import <IdentityI.h>
#import <DispatcherI.h>
#import <BatchRequestInterceptorI.h>
#import <Util.h>
#import <VersionI.h>
#import <LocalObjectI.h>

#import <objc/Ice/LocalException.h>

#include <Ice/Initialize.h>
#include <Ice/RegisterPlugins.h>
#include <IceUtil/UUID.h>
#include <IceUtil/MutexPtrLock.h>

#include <Availability.h>

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSThread.h>

#ifdef ICE_NO_KQUEUE
#  define ICE_USE_CFSTREAM 1
#endif

extern "C"
{

Ice::Plugin*
createIceSSL(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
Ice::Plugin*
createIceDiscovery(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
Ice::Plugin*
createIceLocatorDiscovery(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
#if defined(__APPLE__) && TARGET_OS_IPHONE > 0
Ice::Plugin*
createIceIAP(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
#endif
}

namespace
{
typedef std::map<int, std::string> CompactIdMap;

IceUtil::Mutex* _compactIdMapMutex = 0;
CompactIdMap* _compactIdMap = 0;

class CompactIdResolverI : public Ice::CompactIdResolver
{
public:

    virtual ::std::string
    resolve(::Ice::Int value) const
    {
        assert(_compactIdMapMutex);
        assert(_compactIdMap);

        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(_compactIdMapMutex);
        CompactIdMap::iterator p = _compactIdMap->find(value);
        if(p != _compactIdMap->end())
        {
            return p->second;
        }
        return ::std::string();
    }
};

//
// We don't use the constructor to initialize the compactIdMap as
// static initializtion takes place after +load is called.
//
class Init
{
public:

    ~Init()
    {
        if(_compactIdMap)
        {
            delete _compactIdMap;
            _compactIdMap = 0;
        }

        if(_compactIdMapMutex)
        {
            delete _compactIdMapMutex;
            _compactIdMapMutex = 0;
        }
    }
};
Init init;

}

@implementation CompactIdMapHelper
+(void) initialize
{
    assert(!_compactIdMapMutex);
    assert(!_compactIdMap);
    _compactIdMapMutex = new ::IceUtil::Mutex;
    _compactIdMap = new CompactIdMap;
}

+(void) registerClass:(NSString*)type value:(ICEInt)value
{
    assert(_compactIdMapMutex);
    assert(_compactIdMap);

    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(_compactIdMapMutex);
    CompactIdMap::iterator p = _compactIdMap->find(value);
    if(p != _compactIdMap->end())
    {
        _compactIdMap->erase(p);
    }
    _compactIdMap->insert(CompactIdMap::value_type(value, fromNSString(type)));
}
@end

namespace IceObjC
{

class ThreadNotification : public Ice::ThreadNotification, public IceUtil::Mutex
{
public:

    ThreadNotification()
    {
    }

    virtual void start()
    {
        Lock sync(*this);
        _pools.insert(std::make_pair(IceUtil::ThreadControl().id(), [[NSAutoreleasePool alloc] init]));
    }

    virtual void stop()
    {
        Lock sync(*this);
        std::map<IceUtil::ThreadControl::ID, NSAutoreleasePool*>::iterator p =
            _pools.find(IceUtil::ThreadControl().id());
        [p->second drain];
        _pools.erase(p);
    }

private:

    std::map<IceUtil::ThreadControl::ID, NSAutoreleasePool*> _pools;
};

};

@implementation ICEInitializationData (ICEInternal)
-(Ice::InitializationData)initializationData
{
    Ice::InitializationData data;
    data.properties = [(ICEProperties*)properties properties];
    data.logger = [ICELogger loggerWithLogger:logger];
    if(batchRequestInterceptor)
    {
        data.batchRequestInterceptor = [ICEBatchRequestInterceptor
                                           batchRequestInterceptorWithBatchRequestInterceptor:batchRequestInterceptor];
    }
    if(dispatcher)
    {
        data.dispatcher = [ICEDispatcher dispatcherWithDispatcher:dispatcher];
    }
    data.compactIdResolver = new CompactIdResolverI;
    return data;
}
@end

@implementation ICEInitializationData

@synthesize properties;
@synthesize logger;
@synthesize dispatcher;
@synthesize batchRequestInterceptor;
@synthesize prefixTable_;

-(id) init:(id<ICEProperties>)props logger:(id<ICELogger>)log dispatcher:(void(^)(id<ICEDispatcherCall>,
                                                                                  id<ICEConnection>))d;
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    properties = [props retain];
    logger = [log retain];
    dispatcher = [d copy];
    return self;
}

+(id) initializationData;
{
   ICEInitializationData *s = [[ICEInitializationData alloc] init];
   [s autorelease];
   return s;
}

+(id) initializationData:(id<ICEProperties>)p logger:(id<ICELogger>)l
              dispatcher:(void(^)(id<ICEDispatcherCall>, id<ICEConnection>))d;
{
   return [[((ICEInitializationData *)[ICEInitializationData alloc]) init:p logger:l dispatcher:d] autorelease];
}

-(id) copyWithZone:(NSZone *)zone
{
    ICEInitializationData *copy = [ICEInitializationData allocWithZone:zone];
    copy->properties = [properties retain];
    copy->logger = [logger retain];
    copy->dispatcher = [dispatcher copy];
    copy->prefixTable_ = [prefixTable_ retain];
    return copy;
}

-(NSUInteger) hash;
{
    NSUInteger h = 0;
    h = (h << 5 ^ [properties hash]);
    h = (h << 5 ^ [logger hash]);
    h = (h << 5 ^ [prefixTable_ hash]);
    return h;
}

-(BOOL) isEqual:(id)anObject;
{
    if(self == anObject)
    {
        return YES;
    }
    if(!anObject || ![anObject isKindOfClass:[self class]])
    {
        return NO;
    }
    ICEInitializationData * obj =(ICEInitializationData *)anObject;
    if(!properties)
    {
        if(obj->properties)
        {
            return NO;
        }
    }
    else
    {
        if(![properties isEqual:obj->properties])
        {
            return NO;
        }
    }
    if(!logger)
    {
        if(obj->logger)
        {
            return NO;
        }
    }
    else
    {
        if(![logger isEqual:obj->logger])
        {
            return NO;
        }
    }
    if(!dispatcher)
    {
        if(obj->dispatcher)
        {
            return NO;
        }
    }
    else
    {
        if(dispatcher == obj->dispatcher)
        {
            return NO;
        }
    }
    if(!prefixTable_)
    {
        if(obj->prefixTable_)
        {
            return NO;
        }
    }
    else
    {
        if(![prefixTable_ isEqual:obj->prefixTable_])
        {
            return NO;
        }
    }
    return YES;
}

-(void) dealloc;
{
    [properties release];
    [logger release];
    [dispatcher release];
    [prefixTable_ release];
    [super dealloc];
}
@end

@implementation ICEUtil
+(id<ICEProperties>) createProperties
{
    return [self createProperties:nil argv:nil];
}

+(id<ICEProperties>) createProperties:(int*)argc argv:(char*[])argv
{
    NSException* nsex = nil;
    try
    {
        Ice::PropertiesPtr properties;
        if(argc != nil && argv != nil)
        {
            properties = Ice::createProperties(*argc, argv);
        }
        else
        {
            properties = Ice::createProperties();
        }
        return [ICEProperties localObjectWithCxxObject:properties.get()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

+(id<ICECommunicator>) createCommunicator
{
    return [self createCommunicator:nil argv:nil initData:nil];
}

+(id<ICECommunicator>) createCommunicator:(ICEInitializationData*)initData
{
    return [self createCommunicator:nil argv:nil initData:initData];
}

+(id<ICECommunicator>) createCommunicator:(int*)argc argv:(char*[])argv
{
    return [self createCommunicator:argc argv:argv initData:nil];
}

+(id<ICECommunicator>) createCommunicator:(int*)argc argv:(char*[])argv initData:(ICEInitializationData*)initData
{
    if(![NSThread isMultiThreaded]) // Ensure sure Cocoa is multithreaded.
    {
        NSThread* thread = [[NSThread alloc] init];
        [thread start];
        [thread release];
    }

    id<ICEProperties> properties = [initData properties];
    if(properties != nil && ![properties isKindOfClass:[ICEProperties class]])
    {
        @throw [ICEInitializationException initializationException:__FILE__ line:__LINE__
                                           reason:@"properties were not created with createProperties"];
    }

    NSException* nsex = nil;
    try
    {
        Ice::InitializationData data;
        if(initData != nil)
        {
            data = [initData initializationData];
        }
        data.threadHook = new IceObjC::ThreadNotification();
        if(!data.properties)
        {
            data.properties = Ice::createProperties();
        }

        if(argc != nil && argv != nil)
        {
            data.properties = createProperties(*argc, argv, data.properties);
        }

        Ice::CommunicatorPtr communicator;
        if(argc != nil && argv != nil)
        {
            communicator = Ice::initialize(*argc, argv, data);
        }
        else
        {
            communicator = Ice::initialize(data);
        }

        ICECommunicator* c = [ICECommunicator localObjectWithCxxObject:communicator.get()];
        [c setup:initData];
        return c;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

+(id<ICECommunicator>) createCommunicator:(int*)argc argv:(char*[])argv configFile:(NSString*)configFile
{
    ICEInitializationData* initData = [ICEInitializationData initializationData];
    initData.properties = [ICEUtil createProperties];
    [initData.properties load:configFile];

    return [self createCommunicator:argc argv:argv initData:initData];
}

+(id<ICEInputStream>) createInputStream:(id<ICECommunicator>)c data:(NSData*)data
{
    NSException* nsex = nil;
    try
    {
        return [[[ICEInputStream alloc] initWithCommunicator:c data:data encoding:nil] autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

+(id<ICEInputStream>) createInputStream:(id<ICECommunicator>)c data:(NSData*)data encoding:(ICEEncodingVersion*)e
{
    NSException* nsex = nil;
    try
    {
        return [[[ICEInputStream alloc] initWithCommunicator:c data:data encoding:e] autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

+(id<ICEOutputStream>) createOutputStream:(id<ICECommunicator>)communicator
{
    NSException* nsex = nil;
    try
    {
        return [[[ICEOutputStream alloc] initWithCommunicator:communicator encoding:nil] autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

+(id<ICEOutputStream>) createOutputStream:(id<ICECommunicator>)communicator encoding:(ICEEncodingVersion*)encoding
{
    NSException* nsex = nil;
    try
    {
        return [[[ICEOutputStream alloc] initWithCommunicator:communicator encoding:encoding] autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

+(NSString*) generateUUID
{
    return [NSString stringWithUTF8String:IceUtil::generateUUID().c_str()];
}

+(NSArray*)argsToStringSeq:(int)argc argv:(char*[])argv
{
    NSMutableArray* ns = [NSMutableArray array];
    int i;
    for(i = 0; i < argc; ++i)
    {
        [ns addObject:[NSString stringWithCString:argv[i] encoding:NSUTF8StringEncoding]];
    }
    return [[ns copy] autorelease];
}

+(void)stringSeqToArgs:(NSArray*)args argc:(int*)argc argv:(char*[])argv
{
    //
    // Shift all elements in argv which are present in args to the
    // beginning of argv. We record the original value of argc so
    // that we can know later if we've shifted the array.
    //
    const int argcOrig = *argc;
    int i = 0;
    while(i < *argc)
    {
        BOOL found = NO;
        for(NSString* s in args)
        {
            if([s compare:[NSString stringWithCString:argv[i] encoding:NSUTF8StringEncoding]] == 0)
            {
                found = YES;
                break;
            }
        }
        if(!found)
        {
            int j;
            for(j = i; j < *argc - 1; j++)
            {
                argv[j] = argv[j + 1];
            }
            --(*argc);
        }
        else
        {
            ++i;
        }
    }

    //
    // Make sure that argv[*argc] == 0, the ISO C++ standard requires this.
    // We can only do this if we've shifted the array, otherwise argv[*argc]
    // may point to an invalid address.
    //
    if(argv && argcOrig != *argc)
    {
        argv[*argc] = 0;
    }
}

+(ICEIdentity*) stringToIdentity:(NSString*)str
{
    NSException* nsex = nil;
    try
    {
        return [ICEIdentity identityWithIdentity:Ice::stringToIdentity(fromNSString(str))];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

+(NSMutableString*) identityToString:(ICEIdentity*)ident toStringMode:(ICEToStringMode)toStringMode
{
    NSException* nsex = nil;
    try
    {
        return [toNSMutableString(Ice::identityToString([ident identity],
                                                        static_cast<Ice::ToStringMode>(toStringMode))) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

+(NSMutableString*) identityToString:(ICEIdentity*)ident
{
    return [ICEUtil identityToString:ident toStringMode:ICEUnicode];
}

@end

@implementation ICEEncodingVersion(StringConv)
+(ICEEncodingVersion*) encodingVersionWithString:(NSString*)str
{
    return [ICEEncodingVersion encodingVersionWithEncodingVersion:Ice::stringToEncodingVersion(fromNSString(str))];
}
@end

@implementation ICEProtocolVersion(StringConv)
+(ICEProtocolVersion*) protocolVersionWithString:(NSString*)str
{
    return [ICEProtocolVersion protocolVersionWithProtocolVersion:Ice::stringToProtocolVersion(fromNSString(str))];
}
@end
