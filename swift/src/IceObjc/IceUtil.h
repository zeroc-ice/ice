// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "Communicator.h"
#import "Connection.h"
#import "Endpoint.h"
#import "LocalException.h"
#import "Logger.h"
#import "Properties.h"

NS_ASSUME_NONNULL_BEGIN

//
// Utility methods
//
@interface ICEUtil: NSObject
@property (class, nonatomic, readonly) Class<ICELocalExceptionFactory> localExceptionFactory;
@property (class, nonatomic, readonly) Class<ICEConnectionInfoFactory> connectionInfoFactory;
@property (class, nonatomic, readonly) Class<ICEEndpointInfoFactory> endpointInfoFactory;

//This method should only be called once to guarenteed thread safety
+(BOOL) registerFactories:(Class<ICELocalExceptionFactory>)localException
           connectionInfo:(Class<ICEConnectionInfoFactory>)connectionInfo
             endpointInfo:(Class<ICEEndpointInfoFactory>)endpointInfo
NS_SWIFT_NAME(registerFactories(localException:connectionInfo:endpointInfo:));

+(nullable ICECommunicator*) initialize:(NSArray*)swiftArgs
                             properties:(ICEProperties* _Null_unspecified)properties
                                 logger:(id<ICELoggerProtocol>)logger
                                  error:(NSError* _Nullable * _Nullable)error;

+(nullable ICEProperties*) createProperties:(NSArray* _Nullable)swiftArgs
                                   defaults:(ICEProperties* _Nullable)defaults
                                    remArgs:(NSArray* _Null_unspecified * _Null_unspecified)remArgs
                                      error:(NSError* _Nullable * _Nullable)error;
+(BOOL) stringToIdentity:(NSString*)str
                    name:(NSString* __strong _Nonnull *  _Nonnull)name
                category:(NSString* __strong  _Nonnull *  _Nonnull)category
                   error:(NSError* _Nullable * _Nullable)error NS_SWIFT_NAME(stringToIdentity(str:name:category:));

+(nullable NSString*) identityToString:(NSString*)name
                              category:(NSString*)category
                                 error:(NSError* _Nullable * _Nullable)error NS_SWIFT_NAME(identityToString(name:category:));

+(void) currentEncoding:(UInt8*)major minor:(UInt8*)minor NS_SWIFT_NAME(currentEncoding(major:minor:));

@end

NS_ASSUME_NONNULL_END
