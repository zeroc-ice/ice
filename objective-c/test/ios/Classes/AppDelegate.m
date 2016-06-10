// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <AppDelegate.h>
#import <TestViewController.h>
#import <Test.h>

struct TestCases
{
    NSString* __unsafe_unretained name;
    int (*startServer)(int, char**);
    int (*startClient)(int, char**);
    bool sslSupport;
    bool wsSupport;
    bool runWithSlicedFormat;
    bool runWith10Encoding;
};

static NSString* protocols[] = { @"tcp", @"ssl", @"ws", @"wss" };
const int nProtocols = sizeof(protocols) / sizeof(NSString*);

int adapterDeactivationServer(int, char**);
int adapterDeactivationClient(int, char**);
int adminServer(int, char**);
int adminClient(int, char**);
int amiServer(int, char**);
int amiClient(int, char**);
int bindingServer(int, char**);
int bindingClient(int, char**);
int defaultServantServer(int, char**);
int defaultServantClient(int, char**);
int defaultValueClient(int, char**);
int dispatcherServer(int, char**);
int dispatcherClient(int, char**);
int enumServer(int, char**);
int enumClient(int, char**);
int exceptionsServer(int, char**);
int exceptionsClient(int, char**);
int facetsServer(int, char**);
int facetsClient(int, char**);
int holdServer(int, char**);
int holdClient(int, char**);
int inheritanceServer(int, char**);
int inheritanceClient(int, char**);
int interceptorServer(int, char**);
int interceptorClient(int, char**);
int invokeServer(int, char**);
int invokeClient(int, char**);
int locationServer(int, char**);
int locationClient(int, char**);
int objectsServer(int, char**);
int objectsClient(int, char**);
int operationsServer(int, char**);
int operationsClient(int, char**);
int optionalServer(int, char**);
int optionalClient(int, char**);
int proxyServer(int, char**);
int proxyClient(int, char**);
int retryServer(int, char**);
int retryClient(int, char**);
int streamClient(int, char**);
int timeoutServer(int, char**);
int timeoutClient(int, char**);
int slicingExceptionsServer(int, char**);
int slicingExceptionsClient(int, char**);
int hashClient(int, char**);
int infoServer(int, char**);
int infoClient(int, char**);
int metricsServer(int, char**);
int metricsClient(int, char**);
int servicesClient(int, char**);

static const struct TestCases alltests[] =
{
//
// Name | Server | Client | SSL Support | WS Support | Sliced | 1.0 Encoding |
//
{ @"proxy", proxyServer, proxyClient, true, true, false, false },
{ @"admin", adminServer, adminClient, true, true, false, false },
{ @"ami", amiServer, amiClient, true, true, false, false },
{ @"operations", operationsServer, operationsClient, true, true, false, false },
{ @"exceptions", exceptionsServer, exceptionsClient, true, true, true, true },
{ @"inheritance", inheritanceServer, inheritanceClient, true, true, false, false },
{ @"invoke", invokeServer, invokeClient, true, true, false, false },
{ @"metrics", metricsServer, metricsClient, false, false, false, false},
{ @"facets", facetsServer, facetsClient, true, true, false, false },
{ @"objects", objectsServer, objectsClient, true, true, true, true },
{ @"optional", optionalServer, optionalClient, true, true, true, false },
{ @"interceptor", interceptorServer, interceptorClient, true, true, false, false },
{ @"dispatcher", dispatcherServer, dispatcherClient, true, true, false, false },
{ @"defaultServant", defaultServantServer, defaultServantClient, true, true, false, false },
{ @"defaultValue", 0, defaultValueClient, true, true, false, false },
{ @"binding", bindingServer, bindingClient, true, true, false, false },
{ @"hold", holdServer, holdClient, true, true, false, false },
{ @"location", locationServer, locationClient, true, true, false, false },
{ @"adapterDeactivation", adapterDeactivationServer, adapterDeactivationClient, true, true, false, false },
{ @"stream", 0, streamClient, true, true, false, true },
{ @"slicing/exceptions", slicingExceptionsServer, slicingExceptionsClient, true, true, false, true },
//
// Slicing objects will not work as both applications are linked in the same executable
// and have knowledge of the same Slice types.
//
//{ @"slicing/objects",slicingObjectsServer, slicingObjectsClient, true, false, true },
{ @"retry",retryServer, retryClient, true, true, false, false },
{ @"timeout",timeoutServer, timeoutClient, true, true, false, false },
{ @"hash", 0, hashClient, true, true, false, false },
{ @"info",infoServer, infoClient, true, true, false , false },
{ @"enums", enumServer, enumClient, true, true, false, true },
{ @"services", 0, servicesClient, true, true, false, false }
};

@implementation AppDelegate

@synthesize window;
@synthesize navigationController;
@synthesize tests;
@synthesize currentTest;
@synthesize protocol;
@synthesize loop;
@synthesize runAll;

static NSString* currentTestKey = @"currentTestKey";
static NSString* protocolKey = @"protocolKey";

+(void)initialize
{
    NSDictionary* appDefaults = [NSDictionary dictionaryWithObjectsAndKeys:
                                 @"0", currentTestKey,
                                 @"tcp", protocolKey,
                                 nil];
    [[NSUserDefaults standardUserDefaults] registerDefaults:appDefaults];
}

-(id)init
{
    if(self = [super init])
    {
        self->runAll = getenv("RUNALL") != NULL;

        NSMutableArray* theTests = [NSMutableArray array];
        for(int i = 0; i < sizeof(alltests)/sizeof(alltests[0]); ++i)
        {
            Test* test = [Test testWithName:alltests[i].name
                                     server:alltests[i].startServer
                                     client:alltests[i].startClient
                                 sslSupport:alltests[i].sslSupport
                                  wsSupport:alltests[i].wsSupport
                        runWithSlicedFormat:alltests[i].runWithSlicedFormat
                          runWith10Encoding:alltests[i].runWith10Encoding];
            [theTests addObject:test];
        }
         tests = [theTests copy];
#if defined(__clang__) && !__has_feature(objc_arc)
        [tests retain];
#endif
        // Initialize the application defaults.
        currentTest = [[NSUserDefaults standardUserDefaults] integerForKey:currentTestKey];
        if(runAll || currentTest < 0 || currentTest > tests.count)
        {
            currentTest = 0;
        }

        protocol = [[NSUserDefaults standardUserDefaults] stringForKey:protocolKey];
        int i = 0;
        for(; i < nProtocols; ++i)
        {
            if([protocols[i] isEqualToString:protocol])
            {
                break;
            }
        }
        if(i == nProtocols)
        {
            protocol = @"tcp";
        }

        loop = NO;
    }
    return self;
}

- (void)applicationDidFinishLaunching:(UIApplication *)application
{

    // Override point for customization after app launch
    [window setRootViewController:navigationController];
    [window makeKeyAndVisible];
}

#if defined(__clang__) && !__has_feature(objc_arc)
- (void)dealloc
{
    [tests release];
    [protocol release];
    [navigationController release];
    [window release];
    [super dealloc];
}
#endif

-(NSInteger)currentTest
{
    return currentTest;
}

-(void)setCurrentTest:(NSInteger)test
{
    currentTest = test;

    [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithInteger:currentTest] forKey:currentTestKey];
}

-(NSString*)protocol
{
    return protocol;
}

-(void)setprotocol:(NSString*)v
{
    protocol = v;
    [[NSUserDefaults standardUserDefaults] setObject:protocol forKey:protocolKey];
}

-(BOOL)testCompleted:(BOOL)success
{
    if(success)
    {
        self.currentTest = (currentTest+1) % tests.count;
        if(runAll || loop)
        {
            if(self.currentTest == 0)
            {
                int i = 0;
                for(; i < nProtocols; ++i)
                {
                    if([protocols[i] isEqualToString:protocol])
                    {
                        break;
                    }
                }

                if(++i == nProtocols && !loop)
                {
                    return NO;
                }
                else
                {
                    protocol = protocols[i % nProtocols];
                }
            }
            return YES;
        }
    }
    return NO;
}

@end
