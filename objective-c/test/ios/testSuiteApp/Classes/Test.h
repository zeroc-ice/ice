// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <UIKit/UIKit.h>


@interface Test : NSObject
{
@private
    int (*server)(int,char**);
    int (*client)(int,char**);
    NSString* name;
    BOOL sslSupport;
    BOOL wsSupport;
    BOOL runWithSlicedFormat;
    BOOL runWith10Encoding;
}

+(id) testWithName:(const NSString*)name
              server:(int (*)(int, char**))server
              client:(int (*)(int, char**))client
          sslSupport:(BOOL)sslSupport
           wsSupport:(BOOL)wsSupport
 runWithSlicedFormat:(BOOL)runWithSlicedFormat
   runWith10Encoding:(BOOL)runWith10Encoding;

-(BOOL)hasServer;
-(BOOL)isProtocolSupported:(NSString*)protocol;
-(int)server;
-(int)client;

@property (readonly) NSString* name;
@property (readonly) BOOL sslSupport;
@property (readonly) BOOL wsSupport;
@property (readonly) BOOL runWithSlicedFormat;
@property (readonly) BOOL runWith10Encoding;

@end
