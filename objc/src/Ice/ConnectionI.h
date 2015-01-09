// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Connection.h>

#import <Wrapper.h>

#include <IceCpp/Connection.h>
#include <IceSSLCpp/ConnectionInfo.h>

@interface ICEConnectionInfo ()
-(id) initWithConnectionInfo:(Ice::ConnectionInfo*)connectionInfo;
+(id) connectionInfoWithConnectionInfo:(Ice::ConnectionInfo*)connectionInfo;
@end

@interface ICEIPConnectionInfo ()
-(id) initWithIPConnectionInfo:(Ice::IPConnectionInfo*)ipConnectionInfo;
@end

@interface ICETCPConnectionInfo ()
-(id) initWithTCPConnectionInfo:(Ice::TCPConnectionInfo*)tcpConnectionInfo;
@end

@interface ICEUDPConnectionInfo ()
-(id) initWithUDPConnectionInfo:(Ice::UDPConnectionInfo*)udpConnectionInfo;
@end

@interface ICEWSConnectionInfo ()
-(id) initWithWSConnectionInfo:(Ice::WSConnectionInfo*)wsConnectionInfo;
@end

@interface ICESSLConnectionInfo ()
-(id) initWithSSLConnectionInfo:(IceSSL::ConnectionInfo*)sslConnectionInfo;
@end

@interface ICEConnection : ICEInternalWrapper<ICEConnection>
@end
