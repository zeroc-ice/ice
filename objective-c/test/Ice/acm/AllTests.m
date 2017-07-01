// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <acm/TestI.h>

@interface LoggerI : ICELocalObject<ICELogger>
{
    NSCondition* _cond;
    BOOL _started;
    NSMutableArray* _messages;
}
-(void) start;
-(void) dump;
@end

@implementation LoggerI
-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    _cond = [[NSCondition alloc] init];
    _started = NO;
    _messages = [NSMutableArray array];
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_cond release];
    [super dealloc];
}
#endif
-(void) start
{
    [_cond lock];
    _started = YES;
    [self dump];
    [_cond unlock];
}
-(void) print:(NSString*)message
{
    [_cond lock];
    [_messages addObject:message];
    if(_started)
    {
        [self dump];
    }
    [_cond unlock];
}
-(void) trace:(NSString*)category message:(NSString*)message
{
    [_cond lock];
    [_messages addObject:message];
    if(_started)
    {
        [self dump];
    }
    [_cond unlock];
}
-(void) warning:(NSString*)message
{
    [_cond lock];
    [_messages addObject:message];
    if(_started)
    {
        [self dump];
    }
    [_cond unlock];
}
-(void) error:(NSString*)message
{
    [_cond lock];
    [_messages addObject:message];
    if(_started)
    {
        [self dump];
    }
    [_cond unlock];
}
-(NSMutableString*) getPrefix
{
    return [@"" mutableCopy];
}
-(id<ICELogger>) cloneWithPrefix:(NSString*)prefix
{
    return self;
}
-(void) dump
{
    for(int i = 0; i < _messages.count; ++i)
    {
        tprintf([_messages[i] UTF8String]);
    }
    [_messages removeAllObjects];
}
@end

@class TestCaseThread;

@interface TestCase : ICELocalObject
{
    NSCondition* _cond;

    NSString* _name;
    id<TestACMRemoteCommunicatorPrx> _com;
    NSString* _msg;
    LoggerI* _logger;
    TestCaseThread* _thread;

    id<ICECommunicator> _communicator;
    id<TestACMRemoteObjectAdapterPrx> _adapter;

    int _clientACMTimeout;
    int _clientACMClose;
    int _clientACMHeartbeat;
    int _serverACMTimeout;
    int _serverACMClose;
    int _serverACMHeartbeat;

    int _heartbeat;
    BOOL _closed;
}
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com;
-(id) initWithName:(NSString*)name com:(id<TestACMRemoteCommunicatorPrx>)com;

-(void) initialize;
-(void) start;
-(void) destroy;
-(void) join;

-(void) run;

+(NSString*) getName;
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy;

-(void) waitForClosed;
-(void) setClientACM:(int)timeout close:(int)close heartbeat:(int)heartbeat;
-(void) setServerACM:(int)timeout close:(int)close heartbeat:(int)heartbeat;
@end

@interface TestCaseThread : NSThread
{
    NSCondition* _cond;
    TestCase* _test;
    BOOL _called;
}
-(id) initWithTestCase:(TestCase*)test;
-(void) join;
-(void) run;
@end

@implementation TestCase
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com
{
    return ICE_AUTORELEASE([[self alloc] initWithName:[self getName] com:com]);
}
-(id) initWithName:(NSString*)name com:(id<TestACMRemoteCommunicatorPrx>)com
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    _name = name;
    _com = com;

    _cond = [[NSCondition alloc] init];
    _logger = [[LoggerI alloc] init];

    _clientACMTimeout = -1;
    _clientACMClose = -1;
    _clientACMHeartbeat = -1;
    _serverACMTimeout = -1;
    _serverACMClose = -1;
    _serverACMHeartbeat = -1;

    _heartbeat = 0;
    _closed = NO;

    return self;
}
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_cond release];
    [_logger release];
    [super dealloc];
}
#endif

-(void) initialize
{
    _adapter = ICE_RETAIN([_com createObjectAdapter:_serverACMTimeout close:_serverACMClose
                                          heartbeat:_serverACMHeartbeat]);

    ICEInitializationData* initData = [ICEInitializationData initializationData];
    initData.properties = [[[_com ice_getCommunicator] getProperties] clone];
    initData.logger = _logger;
    [initData.properties setProperty:@"Ice.ACM.Timeout" value:@"2"];
    if(_clientACMTimeout >= 0)
    {
        [initData.properties setProperty:@"Ice.ACM.Client.Timeout"
                                   value:[NSString stringWithFormat:@"%d", _clientACMTimeout]];
    }
    if(_clientACMClose >= 0)
    {
        [initData.properties setProperty:@"Ice.ACM.Client.Close"
                                   value:[NSString stringWithFormat:@"%d", _clientACMClose]];
    }
    if(_clientACMHeartbeat >= 0)
    {
        [initData.properties setProperty:@"Ice.ACM.Client.Heartbeat"
                                   value:[NSString stringWithFormat:@"%d", _clientACMHeartbeat]];
    }
    _communicator = ICE_RETAIN([ICEUtil createCommunicator:initData]);
}
-(void) start
{
    _thread = [[TestCaseThread alloc] initWithTestCase:self];
    [_thread start];
}
-(void) destroy
{
    [_adapter deactivate];
    ICE_RELEASE(_adapter);

    [_communicator destroy];
    ICE_RELEASE(_communicator);
}
-(void) join
{
    tprintf("testing %s... ", [_name UTF8String]);
    [_logger start];
    [_thread join];
    if([_msg length] == 0)
    {
        tprintf("ok\n");
    }
    else
    {
        tprintf("failed!\n%s", [_msg UTF8String]);
        test(NO);
    }
}

-(void) run
{
    id<TestACMTestIntfPrx> proxy =
        [TestACMTestIntfPrx uncheckedCast:[_communicator stringToProxy:[[_adapter getTestIntf] ice_toString]]];

    @try
    {
        [[proxy ice_getConnection] setCloseCallback:^(id<ICEConnection> connection)
        {
            [_cond lock];
            _closed = YES;
            [_cond signal];
            [_cond unlock];
        }];
        [[proxy ice_getConnection] setHeartbeatCallback:^(id<ICEConnection> connection)
        {
            [_cond lock];
            ++_heartbeat;
            [_cond unlock];
        }];

        [self runTestCase:_adapter proxy:proxy];
    }
    @catch(ICEException* ex)
    {
        _msg = [ex description];
    }
}

+(NSString*) getName
{
    NSAssert(NO, @"Subclasses need to overwrite this method");
    return nil; // To keep compiler happy
}

-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy
{
    NSAssert(NO, @"Subclasses need to overwrite this method");
}

-(void) waitForClosed
{
    [_cond lock];
    @try
    {
        NSDate* start = [NSDate date];
        while(!_closed)
        {
            [_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:2]];
            if(start.timeIntervalSinceNow > -1)
            {
                test(NO);
            }
        }
    }
    @finally
    {
        [_cond unlock];
    }

}
-(void) setClientACM:(int)timeout close:(int)close heartbeat:(int)heartbeat
{
    _clientACMTimeout = timeout;
    _clientACMClose = close;
    _clientACMHeartbeat = heartbeat;
}
-(void) setServerACM:(int)timeout close:(int)close heartbeat:(int)heartbeat
{
    _serverACMTimeout = timeout;
    _serverACMClose = close;
    _serverACMHeartbeat = heartbeat;
}
@end

@implementation TestCaseThread
-(id) initWithTestCase:(TestCase*)test
{
    self = [super initWithTarget:self selector:@selector(run) object:nil];
    if(!self)
    {
        return nil;
    }

    _test = test;
    _called = NO;
    _cond = [[NSCondition alloc] init];

    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_cond release];
    [super dealloc];
}
#endif

-(void) run
{
    [_cond lock];
    [_test run];
    _test = nil; // Break cyclic reference count
    _called = YES;
    [_cond signal];
    [_cond unlock];
}

-(void) join
{
    [_cond lock];
    while(!_called)
    {
        [_cond wait];
    }
    [_cond unlock];
}
@end

@interface InvocationHeartbeatTest : TestCase
+(NSString*) getName;
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy;
@end

@implementation InvocationHeartbeatTest
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com
{
    id tc = [super testCase:com];
    [tc setServerACM:1 close:-1 heartbeat:-1]; // Faster ACM to make sure we receive enough ACM heartbeats
    return tc;
}
+(NSString*) getName
{
      return @"invocation heartbeat";
}
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy
{
    [proxy sleep:4];

    [_cond lock];
    @try
    {
        test(_heartbeat >= 2);
    }
    @finally
    {
        [_cond unlock];
    }
}
@end

@interface InvocationHeartbeatOnHoldTest : TestCase
+(NSString*) getName;
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy;
@end

@implementation InvocationHeartbeatOnHoldTest
+(NSString*) getName
{
      return @"invocation with heartbeat on hold";
}
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy
{
    @try
    {
        // When the OA is put on hold, connections shouldn't
        // send heartbeats, the invocation should therefore
        // fail.
        [proxy sleepAndHold:10];
        test(NO);
    }
    @catch(ICEConnectionTimeoutException* ex)
    {
        [adapter activate];
        [proxy interruptSleep];

        [self waitForClosed];
    }
}
@end

@interface InvocationNoHeartbeatTest : TestCase
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com;
+(NSString*) getName;
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy;
@end

@implementation InvocationNoHeartbeatTest
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com
{
    id tc = [super testCase:com];
    [tc setServerACM:2 close:2 heartbeat:0]; // Disable heartbeat on invocations
    return tc;
}
+(NSString*) getName
{
      return @"invocation with no heartbeat";
}
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy
{
    @try
    {
        // Heartbeats are disabled on the server, the
        // invocation should fail since heartbeats are
        // expected.
        [proxy sleep:10];
        test(NO);
    }
    @catch(ICEConnectionTimeoutException* ex)
    {
        [proxy interruptSleep];

        [self waitForClosed];

        [_cond lock];
        @try
        {
            test(_heartbeat == 0);
        }
        @finally
        {
            [_cond unlock];
        }
    }
}
@end

@interface InvocationHeartbeatCloseOnIdleTest : TestCase
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com;
+(NSString*) getName;
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy;
@end

@implementation InvocationHeartbeatCloseOnIdleTest
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com
{
    id tc = [super testCase:com];
    [tc setClientACM:1 close:1 heartbeat:0]; // Only close on idle.
    [tc setServerACM:1 close:2 heartbeat:0]; // Disable heartbeat on invocations
    return tc;
}
+(NSString*) getName
{
      return @"invocation with no heartbeat and close on idle";
}
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy
{
    // No close on invocation, the call should succeed this time.
    [proxy sleep:3];

    [_cond lock];
    @try
    {
        test(_heartbeat == 0);
        test(!_closed);
    }
    @finally
    {
        [_cond unlock];
    }
}
@end

@interface CloseOnIdleTest : TestCase
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com;
+(NSString*) getName;
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy;
@end

@implementation CloseOnIdleTest
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com
{
    id tc = [super testCase:com];
    [tc setClientACM:1 close:1 heartbeat:0]; // Only close on idle.
    return tc;
}
+(NSString*) getName
{
      return @"close on idle";
}
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy
{
    [_cond lock];
    [_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:3]]; // Idle for 3 seconds
    [_cond unlock];

    [self waitForClosed];

    [_cond lock];
    @try
    {
        test(_heartbeat == 0);
    }
    @finally
    {
        [_cond unlock];
    }
}
@end

@interface CloseOnInvocationTest : TestCase
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com;
+(NSString*) getName;
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy;
@end

@implementation CloseOnInvocationTest
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com
{
    id tc = [super testCase:com];
    [tc setClientACM:1 close:2 heartbeat:0]; // Only close on invocation
    return tc;
}
+(NSString*) getName
{
      return @"close on invocation";
}
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy
{
    [_cond lock];
    [_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:3]]; // Idle for 3 seconds
    [_cond unlock];

    [_cond lock];
    @try
    {
        test(_heartbeat == 0);
        test(!_closed);
    }
    @finally
    {
        [_cond unlock];
    }
}
@end

@interface CloseOnIdleAndInvocationTest : TestCase
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com;
+(NSString*) getName;
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy;
@end

@implementation CloseOnIdleAndInvocationTest
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com
{
    id tc = [super testCase:com];
    [tc setClientACM:1 close:3 heartbeat:0]; // Only close on idle and invocation
    return tc;
}
+(NSString*) getName
{
      return @"close on idle and invocation";
}
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy
{
    //
    // Put the adapter on hold. The server will not respond to
    // the graceful close. This allows to test whether or not
    // the close is graceful or forceful.
    //
    [adapter hold];

    [_cond lock];
    [_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:3]]; // Idle for 3 seconds
    [_cond unlock];

    [_cond lock];
    @try
    {
        test(_heartbeat == 0);
        test(!_closed); // Not closed yet because of graceful close.
    }
    @finally
    {
        [_cond unlock];
    }

    [adapter activate];
    [_cond lock];
    [_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
    [_cond unlock];

    [self waitForClosed];
}
@end

@interface ForcefulCloseOnIdleAndInvocationTest : TestCase
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com;
+(NSString*) getName;
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy;
@end

@implementation ForcefulCloseOnIdleAndInvocationTest
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com
{
    id tc = [super testCase:com];
    [tc setClientACM:1 close:4 heartbeat:0]; // Only close on idle and invocation
    return tc;
}
+(NSString*) getName
{
      return @"forceful close on idle and invocation";
}
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy
{
    [adapter hold];

    [_cond lock];
    [_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:3]]; // Idle for 3 seconds
    [_cond unlock];

    [self waitForClosed];

    [_cond lock];
    @try
    {
        test(_heartbeat == 0);
    }
    @finally
    {
        [_cond unlock];
    }
}
@end

@interface HeartbeatOnIdleTest : TestCase
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com;
+(NSString*) getName;
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy;
@end

@implementation HeartbeatOnIdleTest
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com
{
    id tc = [super testCase:com];
    [tc setServerACM:1 close:-1 heartbeat:2]; // Enable server heartbeats.
    return tc;
}
+(NSString*) getName
{
      return @"heartbeat on idle";
}
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy
{
    [_cond lock];
    [_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:3]];
    [_cond unlock];

    [_cond lock];
    @try
    {
        test(_heartbeat >= 3);
    }
    @finally
    {
        [_cond unlock];
    }
}
@end

@interface HeartbeatAlwaysTest : TestCase
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com;
+(NSString*) getName;
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy;
@end

@implementation HeartbeatAlwaysTest
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com
{
    id tc = [super testCase:com];
    [tc setServerACM:1 close:-1 heartbeat:3]; // Enable server heartbeats.
    return tc;
}
+(NSString*) getName
{
      return @"heartbeat always";
}
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy
{
    for(int i = 0; i < 10; ++i)
    {
        [proxy ice_ping];

        [_cond lock];
        [_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.3]];
        [_cond unlock];
    }

    [_cond lock];
    @try
    {
        test(_heartbeat >= 3);
    }
    @finally
    {
        [_cond unlock];
    }
}
@end

@interface HeartbeatManualTest : TestCase
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com;
+(NSString*) getName;
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy;
@end

@implementation HeartbeatManualTest
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com
{
    id tc = [super testCase:com];
    //
    // Disable heartbeats.
    //
    [tc setClientACM:10 close:-1 heartbeat:0];
    [tc setServerACM:10 close:-1 heartbeat:0];
    return tc;
}
+(NSString*) getName
{
      return @"manual heartbeats";
}
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy
{
    [proxy startHeartbeatCount];
    id<ICEConnection> con = [proxy ice_getConnection];
    [con heartbeat];
    [con heartbeat];
    [con heartbeat];
    [con heartbeat];
    [con heartbeat];
    [proxy waitForHeartbeatCount:5];
}
@end

@interface SetACMTest : TestCase
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com;
+(NSString*) getName;
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy;
@end

@implementation SetACMTest
+(id) testCase:(id<TestACMRemoteCommunicatorPrx>)com
{
    id tc = [super testCase:com];
    [tc setClientACM:15 close:4 heartbeat:0];
    return tc;
}
+(NSString*) getName
{
      return @"setACM/getACM";
}
-(void) runTestCase:(id<TestACMRemoteObjectAdapterPrx>)adapter proxy:(id<TestACMTestIntfPrx>)proxy
{
    ICEACM* acm = [[proxy ice_getCachedConnection] getACM];
    test(acm.timeout == 15);
    test(acm.close == ICECloseOnIdleForceful);
    test(acm.heartbeat == ICEHeartbeatOff);

    [[proxy ice_getCachedConnection] setACM:ICENone close:ICENone heartbeat:ICENone];
    acm = [[proxy ice_getCachedConnection] getACM];
    test(acm.timeout == 15);
    test(acm.close == ICECloseOnIdleForceful);
    test(acm.heartbeat == ICEHeartbeatOff);

    id timeout = @1;
    id close = @(ICECloseOnInvocationAndIdle);
    id heartbeat = @(ICEHeartbeatAlways);
    [[proxy ice_getCachedConnection] setACM:timeout close:close heartbeat:heartbeat];
    acm = [[proxy ice_getCachedConnection] getACM];
    test(acm.timeout == 1);
    test(acm.close == ICECloseOnInvocationAndIdle);
    test(acm.heartbeat == ICEHeartbeatAlways);

    [proxy startHeartbeatCount];
    [proxy waitForHeartbeatCount:2];
}
@end

void
acmAllTests(id<ICECommunicator> communicator)
{
    id<TestACMRemoteCommunicatorPrx> com =
        [TestACMRemoteCommunicatorPrx uncheckedCast:[communicator stringToProxy:@"communicator:default -p 12010"]];

    NSMutableArray* tests = [NSMutableArray array];

    [tests addObject:[InvocationHeartbeatTest testCase:com]];
    [tests addObject:[InvocationHeartbeatOnHoldTest testCase:com]];
    [tests addObject:[InvocationNoHeartbeatTest testCase:com]];
    [tests addObject:[InvocationHeartbeatCloseOnIdleTest testCase:com]];

    [tests addObject:[CloseOnIdleTest testCase:com]];
    [tests addObject:[CloseOnInvocationTest testCase:com]];
    [tests addObject:[CloseOnIdleAndInvocationTest testCase:com]];
    [tests addObject:[ForcefulCloseOnIdleAndInvocationTest testCase:com]];

    [tests addObject:[HeartbeatOnIdleTest testCase:com]];
    [tests addObject:[HeartbeatAlwaysTest testCase:com]];
    [tests addObject:[HeartbeatManualTest testCase:com]];
    [tests addObject:[SetACMTest testCase:com]];

    for(int i = 0; i < tests.count; ++i)
    {
        [tests[i] initialize];
    }
    for(int i = 0; i < tests.count; ++i)
    {
        [tests[i] start];
    }
    for(int i = 0; i < tests.count; ++i)
    {
        [tests[i] join];
    }
    for(int i = 0; i < tests.count; ++i)
    {
        [tests[i] destroy];
    }

    tprintf("shutting down... ");
    [com shutdown];
    tprintf("ok\n");
}
