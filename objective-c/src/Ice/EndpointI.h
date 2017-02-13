// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in
// the ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Endpoint.h>

#import <objc/Ice/LocalObject.h>

#include <Ice/Endpoint.h>

@interface ICEEndpoint : ICELocalObject<ICEEndpoint>
-(Ice::Endpoint*) endpoint;
@end

@interface ICEEndpointInfo (ICEInternal)
-(id) initWithEndpointInfo:(Ice::EndpointInfo*)endpointInfo;
@end

@interface ICEIPEndpointInfo (ICEInternal)
-(id) initWithIPEndpointInfo:(Ice::IPEndpointInfo*)ipEndpointInfo;
@end

@interface ICETCPEndpointInfo (ICEInternal)
-(id) initWithTCPEndpointInfo:(Ice::TCPEndpointInfo*)tcpEndpointInfo;
@end

@interface ICEUDPEndpointInfo (ICEInternal)
-(id) initWithUDPEndpointInfo:(Ice::UDPEndpointInfo*)udpEndpointInfo;
@end

@interface ICEWSEndpointInfo (ICEInternal)
-(id) initWithWSEndpointInfo:(Ice::WSEndpointInfo*)wsEndpointInfo;
@end

@interface ICEOpaqueEndpointInfo (ICEInternal)
-(id) initWithOpaqueEndpointInfo:(Ice::OpaqueEndpointInfo*)opaqueEndpointInfo;
@end

