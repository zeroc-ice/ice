// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <AppDelegate.h>
#import <TestUtil.h>

static NSArray* protocols = @[ @"tcp", @"ssl", @"ws", @"wss" ];

static NSArray* clientServer = @[ [TestCase testCase:@"client/server" client:nil server:nil args:nil] ];

static NSArray* clientServerAndCollocated = @[
    [TestCase testCase:@"client/server" client:nil server:nil args:nil],
    [TestCase testCase:@"collocated" client:nil server:nil args:nil],
];

static NSArray* clientOnly = @[ [TestCase testCase:@"client" client:nil server:nil args:nil] ];

static NSArray* slicedArgs = @[@"--Ice.Default.SlicedFormat"];
static NSArray* encoding10Args = @[@"--Ice.Default.EncodingVersion=1.0"];

static NSArray* allTests = @[
    [TestSuite testSuite:@"Ice/proxy" testCases:clientServerAndCollocated],
    [TestSuite testSuite:@"Ice/operations" testCases:@[
        [TestCase testCase:@"client/server" client:nil server:nil args:nil],
        [TestCase testCase:@"client/amd server" client:nil server:nil args:nil],
        [TestCase testCase:@"collocated" client:nil server:nil args:nil],
    ]],
    [TestSuite testSuite:@"Ice/exceptions" testCases:@[
        [TestCase testCase:@"client/server with compact format" client:nil server:nil args:nil],
        [TestCase testCase:@"client/server with sliced format" client:nil server:nil args:slicedArgs],
        [TestCase testCase:@"client/server with 1.0 encoding" client:nil server:nil args:encoding10Args],
        [TestCase testCase:@"client/amd server with compact format" client:nil server:nil args:nil],
        [TestCase testCase:@"client/amd server with sliced format" client:nil server:nil args:slicedArgs],
        [TestCase testCase:@"client/amd server with 1.0 encoding" client:nil server:nil args:encoding10Args],
        [TestCase testCase:@"collocated" client:nil server:nil args:nil],
    ]],
    [TestSuite testSuite:@"Ice/ami" testCases:clientServer],
    [TestSuite testSuite:@"Ice/info" testCases:clientServer],
    [TestSuite testSuite:@"Ice/inheritance" testCases:clientServerAndCollocated],
    [TestSuite testSuite:@"Ice/facets" testCases:clientServerAndCollocated],
    [TestSuite testSuite:@"Ice/objects" testCases:@[
        [TestCase testCase:@"client/server with compact format" client:nil server:nil args:nil],
        [TestCase testCase:@"client/server with sliced format" client:nil server:nil args:slicedArgs],
        [TestCase testCase:@"client/server with 1.0 encoding" client:nil server:nil args:encoding10Args],
        [TestCase testCase:@"collocated" client:nil server:nil args:nil],
    ]],
    [TestSuite testSuite:@"Ice/optional" testCases:@[
        [TestCase testCase:@"client/server with compact format" client:nil server:nil args:nil],
        [TestCase testCase:@"client/server with sliced format" client:nil server:nil args:slicedArgs],
        [TestCase testCase:@"client/amd server with compact format" client:nil server:nil args:nil],
        [TestCase testCase:@"client/amd server with sliced format" client:nil server:nil args:slicedArgs],
    ]],
    [TestSuite testSuite:@"Ice/binding" testCases:clientServer],
    [TestSuite testSuite:@"Ice/location" testCases:clientServer],
    [TestSuite testSuite:@"Ice/adapterDeactivation" testCases:clientServerAndCollocated],
    [TestSuite testSuite:@"Ice/slicing/objects" testCases:@[
        [TestCase testCase:@"client/server with sliced format" client:nil server:nil args:slicedArgs],
        [TestCase testCase:@"client/server with 1.0 encoding" client:nil server:nil args:encoding10Args],
        [TestCase testCase:@"client/amd server with sliced format" client:nil server:nil args:slicedArgs],
        [TestCase testCase:@"client/amd server with 1.0 encoding" client:nil server:nil args:encoding10Args],
    ]],
    [TestSuite testSuite:@"Ice/slicing/exceptions" testCases:@[
        [TestCase testCase:@"client/server with sliced format" client:nil server:nil args:slicedArgs],
        [TestCase testCase:@"client/server with 1.0 encoding" client:nil server:nil args:encoding10Args],
        [TestCase testCase:@"client/amd server with sliced format" client:nil server:nil args:slicedArgs],
        [TestCase testCase:@"client/amd server with 1.0 encoding" client:nil server:nil args:encoding10Args],
    ]],
    [TestSuite testSuite:@"Ice/dispatcher" testCases:clientServer],
    [TestSuite testSuite:@"Ice/stream" testCases:clientOnly],
    [TestSuite testSuite:@"Ice/hold" testCases:clientServer],
    [TestSuite testSuite:@"Ice/custom" testCases:@[
        [TestCase testCase:@"client/server" client:nil server:nil args:nil],
        [TestCase testCase:@"client/amd server" client:nil server:nil args:nil],
        [TestCase testCase:@"collocated" client:nil server:nil args:nil],
    ]],
    [TestSuite testSuite:@"Ice/retry" testCases:clientServer],
    [TestSuite testSuite:@"Ice/timeout" testCases:clientServer],
    [TestSuite testSuite:@"Ice/interceptor" testCases:clientOnly],
    [TestSuite testSuite:@"Ice/udp" testCases:clientServer],
    [TestSuite testSuite:@"Ice/defaultServant" testCases:clientOnly],
    [TestSuite testSuite:@"Ice/defaultValue" testCases:clientOnly],
    [TestSuite testSuite:@"Ice/servantLocator" testCases:@[
        [TestCase testCase:@"client/server" client:nil server:nil args:nil],
        [TestCase testCase:@"client/amd server" client:nil server:nil args:nil],
        [TestCase testCase:@"collocated" client:nil server:nil args:nil],
    ]],
    [TestSuite testSuite:@"Ice/invoke" testCases:clientServer],
    [TestSuite testSuite:@"Ice/hash" testCases:clientOnly],
    [TestSuite testSuite:@"Ice/admin" testCases:clientServer],
    [TestSuite testSuite:@"Ice/metrics" testCases:clientServer],
    [TestSuite testSuite:@"Ice/enums" testCases:@[
        [TestCase testCase:@"client/server with default encoding" client:nil server:nil args:slicedArgs],
        [TestCase testCase:@"client/server with 1.0 encoding" client:nil server:nil args:encoding10Args],
    ]],
    [TestSuite testSuite:@"Ice/services" testCases:clientOnly],
    [TestSuite testSuite:@"IceSSL/configuration" testCases:clientServer],
];

@implementation AppDelegate

@synthesize window;
@synthesize navigationController;
@synthesize testSuites;
@synthesize currentTestSuite;
@synthesize loop;

static NSString* currentTestSuiteKey = @"currentTestSuiteKey";
static NSString* protocolKey = @"protocolKey";

+(void)initialize
{
    NSDictionary* appDefaults = [NSDictionary dictionaryWithObjectsAndKeys:
                                 @"0", currentTestSuiteKey,
                                 @"tcp", protocolKey,
                                 nil];
    [[NSUserDefaults standardUserDefaults] registerDefaults:appDefaults];
}

-(id)init
{
    if(self = [super init])
    {
        testSuites = [allTests retain];
        for(TestSuite* testSuite in allTests)
        {
            for(TestCase* testCase in testSuite.testCases)
            {
                if(testCase.client == nil)
                {
                    if([testCase.name rangeOfString:@"client/server"].location == 0)
                    {
                        testCase.client = @"client.bundle";
                        testCase.server = @"server.bundle";
                    }
                    else if([testCase.name rangeOfString:@"client/amd server"].location == 0)
                    {
                        testCase.client = @"client.bundle";
                        testCase.server = @"serveramd.bundle";
                    }
                    else if([testCase.name isEqualToString:@"collocated"])
                    {
                        testCase.client = @"collocated.bundle";
                    }
                    else
                    {
                        testCase.client = @"client.bundle";
                    }
                }
            }
        }

        // Initialize the application defaults.
        currentTestSuite = [[NSUserDefaults standardUserDefaults] integerForKey:currentTestSuiteKey];
        if(currentTestSuite < 0 || currentTestSuite > testSuites.count)
        {
            currentTestSuite = 0;
        }

        protocol = @"tcp";
        NSString* confProt = [[NSUserDefaults standardUserDefaults] stringForKey:protocolKey];
        for(id p in protocols)
        {
            if([p isEqualToString:confProt])
            {
                protocol = confProt;
            }
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
    [testSuites release];
    [protocol release];
    [navigationController release];
    [window release];
    [super dealloc];
}

-(NSInteger)currentTestSuite
{
    return currentTestSuite;
}

-(void)setCurrentTestSuite:(NSInteger)test
{
    currentTestSuite = test;

    [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithInteger:currentTestSuite] forKey:currentTestSuiteKey];
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
        self.currentTestSuite = (currentTestSuite + 1) % testSuites.count;
        if(loop)
        {
            return YES;
        }
    }
    return NO;
}

@end
