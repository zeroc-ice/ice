// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OBJC_H
#define ICE_OBJC_H

#define ICE_CPP11_MAPPING

#include <Ice/Ice.h>
#include <IceSSL/IceSSL.h>

#import "Ice-Bridging-Header.h"

NS_ASSUME_NONNULL_BEGIN

@interface ICELocalObject ()
@property (nonatomic, readonly) void* _Nullable object;
-(instancetype) initWithLocalObject:(void*)object;
+(nullable instancetype) fromLocalObject:(void*)object NS_RETURNS_RETAINED;
@end

@interface ICECommunicator()
@property (nonatomic, readonly) std::shared_ptr<Ice::Communicator> communicator;
-(instancetype) initWithCppCommunicator:(std::shared_ptr<Ice::Communicator>)communicator;
@end

@interface ICEObjectPrx()
@property (nonatomic, readonly) std::shared_ptr<Ice::ObjectPrx> objectPrx;
-(nullable instancetype) initWithCppObjectPrx:(std::shared_ptr<Ice::ObjectPrx>)prx;
@end

@interface ICEProperties()
@property (nonatomic, readonly) std::shared_ptr<Ice::Properties> properties;
-(instancetype) initWithCppProperties:(std::shared_ptr<Ice::Properties>)Properties;
@end

@interface ICEInputStream()
@property (nonatomic, readonly) std::vector<Ice::Byte> bytes;
-(instancetype) initWithBytes:(std::vector<Ice::Byte>)bytes;
@end

@interface ICEConnection()
@property (nonatomic, readonly) std::shared_ptr<Ice::Connection> connection;
-(instancetype) initWithCppConnection:(std::shared_ptr<Ice::Connection>) connection;
@end

@interface ICEEndpoint()
@property (nonatomic, readonly) std::shared_ptr<Ice::Endpoint> endpoint;
-(instancetype) initWithCppEndpoint:(std::shared_ptr<Ice::Endpoint>)endpoint;
+(nullable ICEEndpointInfo*) createEndpointInfo:(std::shared_ptr<Ice::EndpointInfo>)infoPtr NS_RETURNS_RETAINED;
@end

@interface ICEEndpointInfo()
@property (nonatomic, readonly) std::shared_ptr<Ice::EndpointInfo> info;
-(instancetype) initWithCppEndpointInfo:(std::shared_ptr<Ice::EndpointInfo>) info;
@end

@interface ICELogger()
@property (nonatomic, readonly) std::shared_ptr<Ice::Logger> logger;
-(instancetype) initWithCppLogger:(std::shared_ptr<Ice::Logger>)logger;
@end

@interface ICEImplicitContext()
@property (nonatomic, readonly) std::shared_ptr<Ice::ImplicitContext> implicitContext;
-(instancetype) initWithCppImplicitContext:(std::shared_ptr<Ice::ImplicitContext>)implicitContext;
@end

NS_ASSUME_NONNULL_END

#endif
