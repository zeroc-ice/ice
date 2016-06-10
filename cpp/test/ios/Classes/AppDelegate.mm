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
#import <TestUtil.h>

struct TestData
{
    NSString* name;
    NSString* prefix;
    NSString* client;
    NSString* server;
    NSString* serverAMD;
    NSString* collocated;
    bool sslSupport;
    bool wsSupport;
    bool runWithSlicedFormat;
    bool runWith10Encoding;
    bool cpp11Support;
};

static NSString* protocols[] = { @"tcp", @"ssl", @"ws", @"wss" };
const int nProtocols = sizeof(protocols) / sizeof(NSString*);

static const struct TestData alltests[] =
{
//
// | Name | lib base name | client | server | amdserver | collocated | ssl support | ws support | sliced | encoding 1.0 | cpp11 |
//
{ @"proxy", @"Ice_proxy", @"client.bundle", @"server.bundle", @"serveramd.bundle", @"collocated.bundle", true, true, false, false, true },
{ @"operations", @"Ice_operations", @"client.bundle", @"server.bundle", @"serveramd.bundle", @"collocated.bundle", true, true, false, false, true },
{ @"exceptions", @"Ice_exceptions", @"client.bundle", @"server.bundle", @"serveramd.bundle", @"collocated.bundle", true, true, true, true, true },
{ @"ami", @"Ice_ami", @"client.bundle", @"server.bundle", 0, 0, true, true, false, false, true },
{ @"info", @"Ice_info", @"client.bundle", @"server.bundle", 0, 0, true, true, false, false, true },
{ @"inheritance", @"Ice_inheritance", @"client.bundle", @"server.bundle", 0, @"collocated.bundle", true, true, false, false, true},
{ @"facets", @"Ice_facets", @"client.bundle", @"server.bundle", 0, @"collocated.bundle", true, true, false, false, true },
{ @"objects", @"Ice_objects", @"client.bundle", @"server.bundle", 0, @"collocated.bundle", true, true, true, true, true },
{ @"optional", @"Ice_optional", @"client.bundle", @"server.bundle", 0, 0, true, true, true, false, false },
{ @"binding", @"Ice_binding", @"client.bundle", @"server.bundle", 0, 0, true, true, false, false, true },
{ @"location", @"Ice_location", @"client.bundle", @"server.bundle", 0, 0, true, true, false, false, true },
{ @"adapterDeactivation", @"Ice_adapterDeactivation", @"client.bundle", @"server.bundle", 0, @"collocated.bundle", true, true, false, false, true },
{ @"slicing/exceptions", @"Ice_slicing_exceptions", @"client.bundle", @"server.bundle", @"serveramd.bundle", 0, true, true, false, true, true },
{ @"slicing/objects", @"Ice_slicing_objects", @"client.bundle", @"server.bundle", @"serveramd.bundle", 0, true, true, false, true, true },
{ @"dispatcher", @"Ice_dispatcher", @"client.bundle", @"server.bundle", 0, 0, true, true, false, false, true },
{ @"stream", @"Ice_stream", @"client.bundle", 0, 0, 0, true, true, false, false, true },
{ @"hold", @"Ice_hold", @"client.bundle", @"server.bundle", 0, 0, true, true, false, false, true },
{ @"custom", @"Ice_custom", @"client.bundle", @"server.bundle", @"serveramd.bundle", @"collocated.bundle", true, true, false, false, false },
{ @"retry", @"Ice_retry", @"client.bundle", @"server.bundle", 0, 0, true, true, false, false, true },
{ @"timeout", @"Ice_timeout", @"client.bundle", @"server.bundle", 0, 0, true, true, false, false, true },
{ @"interceptor", @"Ice_interceptor", @"client.bundle", 0, 0, 0, true, true, false, false, true },
{ @"udp", @"Ice_udp", @"client.bundle", @"server.bundle", 0, 0, true, true, false, false, true },
{ @"defaultServant", @"Ice_defaultServant", @"client.bundle", 0, 0, 0, true, true, false, false, true },
{ @"defaultValue", @"Ice_defaultValue", @"client.bundle", 0, 0, 0, true, true, false, false, true },
{ @"servantLocator", @"Ice_servantLocator", @"client.bundle", @"server.bundle", @"serveramd.bundle", @"collocated.bundle", true, true, false, false, true },
{ @"invoke", @"Ice_invoke", @"client.bundle", @"server.bundle", 0, 0, true, true, false, false, true },
{ @"hash", @"Ice_hash", @"client.bundle", 0, 0, 0, true, true, false, false, true },
{ @"admin", @"Ice_admin", @"client.bundle", @"server.bundle", 0, 0, true, true, false, false, true },
{ @"metrics", @"Ice_metrics", @"client.bundle", @"server.bundle", 0, 0, false, false, false, false, true },
{ @"enums", @"Ice_enums", @"client.bundle", @"server.bundle", 0, 0, true, true, false, true, true },
{ @"services", @"Ice_services", @"client.bundle", 0, 0, 0, true, true, false, false, true },
};

@implementation AppDelegate

@synthesize window;
@synthesize navigationController;
@synthesize tests;
@synthesize currentTest;
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
            TestCase* test = [TestCase testWithName:alltests[i].name
                                             prefix:alltests[i].prefix
                                             client:alltests[i].client
                                             server:alltests[i].server
                                          serveramd:alltests[i].serverAMD
                                         collocated:alltests[i].collocated
                                         sslSupport:alltests[i].sslSupport
                                          wsSupport:alltests[i].wsSupport
                                runWithSlicedFormat:alltests[i].runWithSlicedFormat
                                  runWith10Encoding:alltests[i].runWith10Encoding
                                       cpp11Support:alltests[i].cpp11Support];
            [theTests addObject:test];
        }
        tests = [[theTests copy] retain];

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

- (void)dealloc
{
    [tests release];
    [protocol release];
    [navigationController release];
    [window release];
    [super dealloc];
}

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

-(void)setProtocol:(NSString*)v
{
    protocol = [v retain];
    [[NSUserDefaults standardUserDefaults] setObject:v forKey:protocolKey];
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
                    std::cout << "\n*** Finished running all tests" << std::endl;
                    return NO;
                }
                else
                {
                    protocol = protocols[i % nProtocols];
                    return YES;
                }
            }
            return YES;
        }
    }
    return NO;
}

@end
