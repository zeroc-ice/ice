// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in
// the ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Endpoint.h>

#import <Wrapper.h>

#include <IceCpp/Endpoint.h>
#include <IceSSLCpp/EndpointInfo.h>

@interface ICEEndpoint : ICEInternalWrapper<ICEEndpoint>
-(Ice::Endpoint*) endpoint;
@end


@interface ICEEndpointInfo ()
-(id) initWithEndpointInfo:(Ice::EndpointInfo*)endpointInfo;
+(id) enpointInfoWithEndpointInfo:(Ice::EndpointInfo*)endpointInfo;
@end

@interface ICEIPEndpointInfo ()
-(id) initWithIPEndpointInfo:(Ice::IPEndpointInfo*)ipEndpointInfo;
@end

@interface ICETCPEndpointInfo ()
-(id) initWithTCPEndpointInfo:(Ice::TCPEndpointInfo*)tcpEndpointInfo;
@end

@interface ICEUDPEndpointInfo ()
-(id) initWithUDPEndpointInfo:(Ice::UDPEndpointInfo*)udpEndpointInfo;
@end

@interface ICEWSEndpointInfo ()
-(id) initWithWSEndpointInfo:(Ice::WSEndpointInfo*)wsEndpointInfo;
@end

@interface ICEOpaqueEndpointInfo ()
-(id) initWithOpaqueEndpointInfo:(Ice::OpaqueEndpointInfo*)opaqueEndpointInfo;
@end

@interface ICESSLEndpointInfo ()
-(id) initWithSSLEndpointInfo:(IceSSL::EndpointInfo*)sslEndpointInfo;
@end
