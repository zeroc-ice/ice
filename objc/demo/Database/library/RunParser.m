// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <objc/Glacier2.h>
#import <Library.h>
#import <Session.h>
#import <Glacier2Session.h>
#import <Parser.h>
#import <stdio.h>
#import <string.h>

#import <Foundation/NSThread.h>
#import <Foundation/NSLock.h>

@interface SessionRefreshThread : NSThread
{
@private
    id<ICELogger> logger;
    id session;
    NSCondition* cond;

    long timeout;
}

-(id)initWithLogger:(id<ICELogger>) logger timeout:(long)timeout session:(id)session;
+(id)sessionRefreshThreadWithLogger:(id<ICELogger>)logger timeout:(long)timeout session:(id)session;
@end

@interface SessionRefreshThread()
@property (nonatomic, retain) id<ICELogger> logger;
@property (nonatomic, retain) id session;
@property (nonatomic, retain) NSCondition* cond;
@end

@implementation SessionRefreshThread

@synthesize logger;
@synthesize session;
@synthesize cond;

-(id)initWithLogger:(id<ICELogger>)l timeout:(long)t session:(id)s
{
    if((self = [super init]))
    {
        self.logger = l;
        self.session = s;
        self.cond = [[NSCondition alloc] init];
        timeout = t;
    }
    return self;
} 

+(id)sessionRefreshThreadWithLogger:(id<ICELogger>)logger timeout:(long)timeout session:(id)session
{
    return [[SessionRefreshThread alloc] initWithLogger:logger timeout:timeout session:session];
}

-(void)main
{
    [cond lock];
    @try
    {
        while(!self.isCancelled)
        {
            [cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:timeout]];
            if(!self.isCancelled)
            {
                @try
                {
                    [session refresh];
                }
                @catch(ICELocalException* ex)
                {
                    NSString* warning = [NSString stringWithFormat:@"SessionRefreshThread: %@", [ex description]];
                    [logger warning:warning];
                    [super cancel];
                }
            }
        }
    }
    @finally
    {
        [cond unlock];
    }
}

-(void)cancel
{
    [super cancel];
    [cond lock];
    @try
    {
        [cond signal];
    }
    @finally
    {
        [cond unlock];
    }
}

@end

int
runParser(int argc, char* argv[], id<ICECommunicator> communicator)
{
    id<GLACIER2RouterPrx> router = [GLACIER2RouterPrx uncheckedCast:[communicator getDefaultRouter]];
    ICELong timeout;
    id<DemoLibraryPrx> library;
    id session;
    if(router != nil)
    {
        printf("This demo accepts any user-id / password combination.\n");

        while(true)
        {
            char id[1024];
            printf("user id: ");
            fflush(stdout);
            fgets(id, sizeof(id), stdin);
            int len = strlen(id);
            if(id[len-1] == '\n')
            {
                id[len-1] = '\0';
            }
            printf("password: ");
            fflush(stdout);
            char pw[1024];
            fgets(pw, sizeof(pw), stdin);
            len = strlen(pw);
            if(pw[len-1] == '\n')
            {
                pw[len-1] = '\0';
            }

            @try
            {
                session = [DemoGlacier2SessionPrx uncheckedCast:
                            [router createSession:[NSString stringWithCString:id encoding:NSUTF8StringEncoding]
                            password:[NSString stringWithCString:pw encoding:NSUTF8StringEncoding]]];
                break;
            }
            @catch(GLACIER2PermissionDeniedException* ex)
            {
                printf("permission denied:\n%s\n", [ex.reason UTF8String]);
            }
            @catch(GLACIER2CannotCreateSessionException* ex)
            {
                printf("cannot create session:\n%s\n", [ex.reason UTF8String]);
            }
        }
	timeout = [router getSessionTimeout]/2;
        library = [session getLibrary];
    }
    else
    {
        id<DemoSessionFactoryPrx> factory = [DemoSessionFactoryPrx checkedCast:[
                communicator propertyToProxy:@"SessionFactory.Proxy"] ];
        if(factory == nil)
        {
            fprintf(stderr, "%s: invalid object reference", argv[0]);
            return 1;
        }

        session = [factory create];
        timeout = [factory getSessionTimeout]/2;
        library = [session getLibrary];
    }

    SessionRefreshThread* refresh = [SessionRefreshThread sessionRefreshThreadWithLogger:[communicator getLogger]
                                                          timeout:timeout/2 session:session];
    [refresh start];

    Parser* parser = [Parser parserWithLibrary:library];

    int rc = [parser parse];

    [refresh cancel];

    // No join.
    while(!refresh.isFinished)
    {
        [NSThread sleepForTimeInterval:0.1];
    }

    if(router)
    {
	@try
	{
	    [router destroySession];
	}
	@catch(GLACIER2SessionNotExistException* ex)
	{
	    NSLog(@"%@\n", [ex description]);
	}
	@catch(ICEConnectionLostException* ex)
	{
	    //
	    // Expected: the router closed the connection.
	    //
	}
    }
    else
    {
	[session destroy];
    }

    return rc;
}
