// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "ViewController.h"

#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    //
    // Load the controller bundle.
    //
    NSString* bundlePath = [[NSBundle mainBundle] privateFrameworksPath];
#ifdef ICE_CPP11_MAPPING
    const char* bundle = "Cpp11ControllerBundle.bundle";
#else
    const char* bundle = "Cpp98ControllerBundle.bundle";
#endif
    bundlePath = [bundlePath stringByAppendingPathComponent:[NSString stringWithUTF8String:bundle]];

    NSURL* bundleURL = [NSURL fileURLWithPath:bundlePath];
    CFBundleRef handle = CFBundleCreate(NULL, (CFURLRef)bundleURL);
    if(!handle)
    {
        [self println:[NSString stringWithFormat:@"Could not find bundle %@", bundlePath]];
        return;
    }

    CFErrorRef error = nil;
    Boolean loaded = CFBundleLoadExecutableAndReturnError(handle, &error);
    if(error != nil || !loaded)
    {
        [self println:[(__bridge NSError *)error description]];
        return;
    }

    startController = CFBundleGetFunctionPointerForName(handle, CFSTR("startController"));
    if(startController == 0)
    {
        NSString* err = [NSString stringWithFormat:@"Could not get function pointer startController from bundle %@",
                         bundlePath];
        [self println:err];
        return;
    }

    stopController = CFBundleGetFunctionPointerForName(handle, CFSTR("stopController"));
    if(stopController == 0)
    {
        NSString* err = [NSString stringWithFormat:@"Could not get function pointer stopController from bundle %@",
                         bundlePath];
        [self println:err];
        return;
    }

    //
    // Search for local network interfaces
    //
    interfacesIPv4 = [NSMutableArray array];
    [interfacesIPv4 addObject:@"127.0.0.1"];
    interfacesIPv6 = [NSMutableArray array];
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
    (*startController)(self,
                       [interfacesIPv4 objectAtIndex:[interfaceIPv4 selectedRowInComponent:0]],
                       [interfacesIPv6 objectAtIndex:[interfaceIPv6 selectedRowInComponent:0]]);
}

- (void) dealloc
{
    (*stopController)(self);
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
    (*stopController)(self);
    (*startController)(self,
                       [interfacesIPv4 objectAtIndex:[interfaceIPv4 selectedRowInComponent:0]],
                       [interfacesIPv6 objectAtIndex:[interfaceIPv6 selectedRowInComponent:0]]);
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
