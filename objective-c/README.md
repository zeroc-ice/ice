# Ice for Objective-C

[Getting started] | [Examples] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications, including RPC, pub/sub, server deployment, and more.

Ice for Objective-C is the Objective-C implementation of the Ice framework.

## Sample Code

```slice
// Slice definitions (Hello.ice)

module Demo
{
    interface Hello
    {
        void sayHello();
    }
}
```

```objective-c
// Client application (Client.m)
#import <objc/Ice.h>
#import <Hello.h>

int
main(int argc, char* argv[])
{
    int status = 0;
    @autoreleasepool
    {
        id<ICECommunicator> communicator = nil;
        @try
        {
            communicator = [ICEUtil createCommunicator:&argc argv:argv];
            if(argc > 1)
            {
                NSLog(@"%s: too many arguments", argv[0]);
                return 1;
            }
            id<DemoHelloPrx> hello = [DemoHelloPrx uncheckedCast:
                [communicator stringToProxy:@"hello:default -p 10000"]];
            [hello sayHello];
        }
        @catch(ICELocalException* ex)
        {
            NSLog(@"%@", ex);
            status = 1;
        }

        [communicator destroy];
    }
    return status;
}
```

```objective-c
// Server application (Server.m)
#import <objc/Ice.h>
#import <HelloI.h>

int
main(int argc, char* argv[])
{
    int status = 0;
    @autoreleasepool
    {
        id<ICECommunicator> communicator = nil;
        @try
        {
            communicator = [ICEUtil createCommunicator:&argc argv:argv];

            id<ICEObjectAdapter> adapter = [communicator
                createObjectAdapterWithEndpoints: @"Hello"
                endpoints:@"default -p 10000"];
            [adapter add:[HelloI hello] identity:[ICEUtil stringToIdentity:@"hello"]];
            [adapter activate];
            [communicator waitForShutdown];
        }
        @catch(ICELocalException* ex)
        {
            NSLog(@"%@", ex);
            status = 1;
        }
        [communicator destroy];
    }
    return status;
}
```

```objective-c
// Printer declaration (Printer.h)
#import <Hello.h>

@interface Printer : DemoHello<DemoHello>
@end
```

```objective-c
// Printer implementation (Printer.m)
#import <Printer.h>

#include <stdio.h>

@implementation Printer
-(void) sayHello:(ICECurrent*)current
{
    printf("Hello World!\n");
    fflush(stdout);
}
@end
```

[Getting started]: https://doc.zeroc.com/ice/3.7/hello-world-application/writing-an-ice-application-with-objective-c
[Examples]: https://github.com/zeroc-ice/ice-demos/tree/3.7/objective-c
[Documentation]: https://doc.zeroc.com/ice/3.7
[Building from source]: https://github.com/zeroc-ice/ice/blob/3.7/objective-c/BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
