// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "ViewController.h"
#import <Controller.h>
#import <TestCommon.h>

#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <dlfcn.h>

@interface MainHelper : NSThread
{
    id<ViewController> _controller;
    void* _func;
    NSArray* _args;
    BOOL _ready;
    BOOL _completed;
    int _status;
    NSMutableString* _out;
    NSCondition* _cond;
}
-(void) serverReady;
-(void) shutdown;
-(void) print:(NSString*)str;
-(void) completed:(int)status;
-(void) waitReady:(int)timeout;
-(int) waitSuccess:(int)timeout;
-(NSString*)getOutput;
@end

@interface ProcessI : TestCommonProcess<TestCommonProcess>
{
    id<ViewController> _controller;
    MainHelper* _helper;
}
-(void) waitReady:(int)timeout current:(ICECurrent*)current;
-(int) waitSuccess:(int)timeout current:(ICECurrent*)current;
-(NSString*) terminate:(ICECurrent*)current;
@end

@interface ProcessControllerI : TestCommonProcessController<TestCommonProcessController>
{
    id<ViewController> _controller;
    NSString* _ipv4;
    NSString* _ipv6;
}
-(id) init:(id<ViewController>) controller ipv4:(NSString*)ipv4 ipv6:(NSString*)ipv6;
-(id<TestCommonProcessPrx>) start:(NSString*)testsuite exe:(NSString*)exe args:(NSArray*)args current:(ICECurrent*)current;
-(NSString*) getHost:(NSString*)protocol ipv6:(BOOL)ipv6 current:(ICECurrent*)current;
@end

@implementation MainHelper
-(id) init:(id<ViewController>)controller func:(void*)func args:(NSArray*)args
{
    self = [super init];
    if(self == nil)
    {
        return nil;
    }
    _controller = ICE_RETAIN(controller);
    _func = func;
    _args = ICE_RETAIN(args);
    _ready = FALSE;
    _completed = FALSE;
    _status = 0;
    _out = ICE_RETAIN([NSMutableString string]);
    _cond = [NSCondition new];
    return self;
}
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_controller release];
    [_args release];
    [_cond release];
    [_out release];
    [super dealloc];
}
#endif
-(void) serverReady
{
    [_cond lock];
    @try
    {
        _ready = YES;
        [_cond signal];
    }
    @finally
    {
        [_cond unlock];
    }
}
-(void) shutdown
{
    [_cond lock];
    @try
    {
        if(_completed)
        {
            return;
        }
        serverStop();
    }
    @finally
    {
        [_cond unlock];
    }
}
-(void) print:(NSString*)msg
{
    [_out appendString:msg];
}
-(void) main
{
    int (*mainEntryPoint)(int, char**) = (int (*)(int, char**))_func;
    char** argv = malloc(sizeof(char*) * (_args.count + 1));
    int i = 0;
    for(NSString* arg in _args)
    {
        argv[i++] = (char*)[arg UTF8String];
    }
    argv[_args.count] = 0;
    @try
    {
        [self completed:mainEntryPoint((int)_args.count, argv)];
    }
    @catch(NSException* ex)
    {
        [self print:[NSString stringWithFormat:@"unexpected exception while running `%s':%@\n", argv[0], ex]];
        [self completed:EXIT_FAILURE];
    }
    free(argv);
}
-(void) completed:(int)status
{
    [_cond lock];
    @try
    {
        _completed = YES;
        _status = status;
        [_cond signal];
    }
    @finally
    {
        [_cond unlock];
    }
}
-(void) waitReady:(int)timeout
{
    [_cond lock];
    @try
    {
        while(!_ready && !_completed)
        {
            if(![_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:timeout]])
            {
               @throw [TestCommonProcessFailedException
                    processFailedException:@"timed out waiting for the process to be ready"];
            }
        }
        if(_completed && _status == EXIT_FAILURE)
        {
            @throw [TestCommonProcessFailedException processFailedException:_out];
        }
    }
    @finally
    {
        [_cond unlock];
    }
}
-(int) waitSuccess:(int)timeout
{
    [_cond lock];
    @try
    {
        while(!_completed)
        {
            if(timeout >= 0)
            {
                if(![_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:timeout]])
                {
                   @throw [TestCommonProcessFailedException
                        processFailedException:@"timed out waiting for the process to succeed"];
                }
            }
            else
            {
                [_cond wait];
            }
        }
    }
    @finally
    {
        [_cond unlock];
    }
    return _status;
}
-(NSString*) getOutput
{
    return _out;
}
@end

@implementation ProcessI
-(id) init:(id<ViewController>)controller helper:(MainHelper*)helper
{
    self = [super init];
    if(self == nil)
    {
        return nil;
    }
    _controller = ICE_RETAIN(controller);
    _helper = ICE_RETAIN(helper);
    return self;
}
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_controller release];
    [_helper release];
    [super dealloc];
}
#endif
-(void) waitReady:(int)timeout current:(ICECurrent*)current
{
    [_helper waitReady:timeout];
}
-(int) waitSuccess:(int)timeout current:(ICECurrent*)current
{
    return [_helper waitSuccess:timeout];
}
-(NSString*) terminate:(ICECurrent*)current
{
    [_helper shutdown];
    [current.adapter remove:current.id_];
    [_helper waitSuccess:-1];
    return [_helper getOutput];
}
@end

@implementation ProcessControllerI
-(id) init:(id<ViewController>)controller ipv4:(NSString*)ipv4 ipv6:(NSString*)ipv6
{
    self = [super init];
    if(self == nil)
    {
        return nil;
    }
    _controller = ICE_RETAIN(controller);
    _ipv4 = ICE_RETAIN(ipv4);
    _ipv6 = ICE_RETAIN(ipv6);
    return self;
}
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_controller release];
    [_ipv4 release];
    [_ipv6 release];
    [super dealloc];
}
#endif
-(id<TestCommonProcessPrx>) start:(NSString*)testSuite exe:(NSString*)exe args:(NSArray*)args current:(ICECurrent*)c
{
    [_controller println:[NSString stringWithFormat:@"starting %@ %@... ", testSuite, exe]];

    NSArray<NSString*>* components = [testSuite componentsSeparatedByString:@"/"];
    components = [components arrayByAddingObject:exe];
    NSMutableString* func = [NSMutableString string];
    [func appendString:[components objectAtIndex:1]];
    for(int i = 2; i < components.count; ++i)
    {
        NSString* comp = [components objectAtIndex:i];
        [func appendString:[[comp substringToIndex:1] capitalizedString]];
        [func appendString:[comp substringFromIndex:1]];
    }

    void* sym = dlsym(RTLD_SELF, [func UTF8String]);
    if(!sym)
    {
        @throw [TestCommonProcessFailedException processFailedException:
                    [NSString stringWithFormat:@"couldn't find %@", func]];
    }
    MainHelper* helper = ICE_AUTORELEASE([[MainHelper alloc] init:_controller func:sym args:args]);
    if([exe isEqualToString:@"client"] || [exe isEqualToString:@"collocated"])
    {
        TestCommonInit(helper, @selector(print:));
    }
    else
    {
        TestCommonInit(helper, @selector(print:));
        TestCommonTestInit(helper, @selector(serverReady), @"", NO, NO);
    }
    [helper start];
    id<ICEObjectPrx> prx = [c.adapter addWithUUID:ICE_AUTORELEASE([[ProcessI alloc] init:_controller helper:helper])];
    return [TestCommonProcessPrx uncheckedCast:prx];
}
-(NSString*) getHost:(NSString*)protocol ipv6:(BOOL)ipv6 current:(ICECurrent*)c
{
    return ICE_AUTORELEASE(ICE_RETAIN(ipv6 ? _ipv6 : _ipv4));
}
@end

@implementation ViewController
- (void) startController
{
    NSString* ipv4 = [interfacesIPv4 objectAtIndex:[interfaceIPv4 selectedRowInComponent:0]];
    NSString* ipv6 = [interfacesIPv6 objectAtIndex:[interfaceIPv6 selectedRowInComponent:0]];

    ICEInitializationData* initData = [ICEInitializationData initializationData];
    initData.properties = [ICEUtil createProperties];
    [initData.properties setProperty:@"Ice.ThreadPool.Server.SizeMax" value:@"10"];
    [initData.properties setProperty:@"Ice.Plugin.IceDiscovery" value:@"1"];
    [initData.properties setProperty:@"IceDiscovery.DomainId" value:@"TestController"];
    [initData.properties setProperty:@"IceDiscovery.Interface" value:ipv4];
    [initData.properties setProperty:@"Ice.Default.Host" value:ipv4];
    [initData.properties setProperty:@"ControllerAdapter.Endpoints" value:@"tcp"];
    //[initData.properties setProperty:@"Ice.Trace.Network", @"2");
    //[initData.properties setProperty:@"Ice.Trace.Protocol", @"2");
    [initData.properties setProperty:@"ControllerAdapter.AdapterId" value:[ICEUtil generateUUID]];

    communicator = ICE_RETAIN([ICEUtil createCommunicator:initData]);

    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"ControllerAdapter"];
    ICEIdentity* ident = [ICEIdentity identity];
#if TARGET_IPHONE_SIMULATOR != 0
    ident.category = @"iPhoneSimulator";
#else
    ident.category = @"iPhoneOS";
#endif
    ident.name = [[NSBundle mainBundle] bundleIdentifier];
    [adapter add:[[ProcessControllerI alloc] init:self ipv4:ipv4 ipv6:ipv6] identity:ident];
    [adapter activate];
}
- (void) stopController
{
    [communicator destroy];
    ICE_RELEASE(communicator);
    communicator = nil;
}
- (void)viewDidLoad
{
    [super viewDidLoad];
    ICEregisterIceDiscovery(NO);

    //
    // Search for local network interfaces
    //
    interfacesIPv4 = ICE_RETAIN([NSMutableArray array]);
    [interfacesIPv4 addObject:@"127.0.0.1"];
    interfacesIPv6 = ICE_RETAIN([NSMutableArray array]);
    [interfacesIPv6 addObject:@"::1"];
    struct ifaddrs* ifap;
    if(getifaddrs(&ifap) == 0)
    {
        struct ifaddrs* curr = ifap;
        while(curr != 0)
        {
            if(curr->ifa_addr && curr->ifa_flags & IFF_UP && !(curr->ifa_flags & IFF_LOOPBACK))
            {
                if(curr->ifa_addr->sa_family == AF_INET)
                {
                    char buf[INET_ADDRSTRLEN];
                    const struct sockaddr_in *addr = (const struct sockaddr_in*)curr->ifa_addr;
                    if(inet_ntop(AF_INET, &addr->sin_addr, buf, INET_ADDRSTRLEN))
                    {
                        [interfacesIPv4 addObject:[NSString stringWithUTF8String:buf]];
                    }
                }
                else if(curr->ifa_addr->sa_family == AF_INET6)
                {
                    char buf[INET6_ADDRSTRLEN];
                    const struct sockaddr_in6 *addr6 = (const struct sockaddr_in6*)curr->ifa_addr;
                    if(inet_ntop(AF_INET6, &addr6->sin6_addr, buf, INET6_ADDRSTRLEN))
                    {
                        [interfacesIPv6 addObject:[NSString stringWithUTF8String:buf]];
                    }
                }
            }
            curr = curr->ifa_next;
        }
        freeifaddrs(ifap);
    }

    // By default, use the loopback
    [interfaceIPv4 selectRow:0 inComponent:0 animated:NO];
    [interfaceIPv6 selectRow:0 inComponent:0 animated:NO];
    [self startController];
}

- (void) dealloc
{
    [self stopController];
#if defined(__clang__) && !__has_feature(objc_arc)
    [interfacesIPv4 release];
    [interfacesIPv6 release];
    [super dealloc];
#endif
}

-(void) write:(NSString*)msg
{
    [output insertText:msg];
    [output layoutIfNeeded];
    [output scrollRangeToVisible:NSMakeRange([output.text length] - 1, 1)];
}

#pragma mark ViewController

-(void) print:(NSString*)msg
{
    [self performSelectorOnMainThread:@selector(write:) withObject:msg waitUntilDone:NO];
}
-(void) println:(NSString*)msg
{
    [self print:[msg stringByAppendingString:@"\n"]];
}

#pragma mark UIPickerViewDelegate

- (NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
    if(pickerView == interfaceIPv4)
    {
        return [interfacesIPv4 objectAtIndex:row];
    }
    else
    {
        return [interfacesIPv6 objectAtIndex:row];
    }
}

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
    [self stopController];
    [self startController];
}

#pragma mark UIPickerViewDataSource

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
    return 1;
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
    if(pickerView == interfaceIPv4)
    {
        return interfacesIPv4.count;
    }
    else
    {
        return interfacesIPv6.count;
    }
}

@end
