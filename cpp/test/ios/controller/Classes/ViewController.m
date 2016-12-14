// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "ViewController.h"

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
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
    
    void (*startController)(id<ViewController>) = CFBundleGetFunctionPointerForName(handle, CFSTR("startController"));
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

    
    (*startController)(self);
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
-(void) print:(NSString*)msg
{
    [self performSelectorOnMainThread:@selector(write:) withObject:msg waitUntilDone:NO];
}
-(void) println:(NSString*)msg
{
    [self print:[msg stringByAppendingString:@"\n"]];
}
@end
