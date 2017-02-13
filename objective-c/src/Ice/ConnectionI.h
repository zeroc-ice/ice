// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Connection.h>
#import <objc/Ice/LocalObject.h>

#include <Ice/Connection.h>

namespace IceObjC
{

ICE_API void registerConnectionInfoClass(Class cl);

}

@interface ICEConnectionInfo (ICEInternal)
-(id) initWithConnectionInfo:(Ice::ConnectionInfo*)connectionInfo;
@end

@interface ICEIPConnectionInfo (ICEInternal)
-(id) initWithIPConnectionInfo:(Ice::IPConnectionInfo*)ipConnectionInfo;
@end

@interface ICETCPConnectionInfo (ICEInternal)
-(id) initWithTCPConnectionInfo:(Ice::TCPConnectionInfo*)tcpConnectionInfo;
@end

@interface ICEUDPConnectionInfo (ICEInternal)
-(id) initWithUDPConnectionInfo:(Ice::UDPConnectionInfo*)udpConnectionInfo;
@end

@interface ICEWSConnectionInfo (ICEInternal)
-(id) initWithWSConnectionInfo:(Ice::WSConnectionInfo*)wsConnectionInfo;
@end

@interface ICEConnection : ICELocalObject<ICEConnection>
@end
