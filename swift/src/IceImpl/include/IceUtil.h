// Copyright (c) ZeroC, Inc.
#import "AdminFacetFactory.h"
#import "Communicator.h"
#import "Connection.h"
#import "Endpoint.h"
#import "LocalExceptionFactory.h"
#import "Logger.h"
#import "Properties.h"

NS_ASSUME_NONNULL_BEGIN

//
// Utility methods
//
ICEIMPL_API @interface ICEUtil : NSObject
@property(class, nonatomic, readonly) Class<ICELocalExceptionFactory> localExceptionFactory;
@property(class, nonatomic, readonly) Class<ICEConnectionInfoFactory> connectionInfoFactory;
@property(class, nonatomic, readonly) Class<ICEEndpointInfoFactory> endpointInfoFactory;
@property(class, nonatomic, readonly) Class<ICEAdminFacetFactory> adminFacetFactory;

// This method should only be called once to guarantee thread safety
+ (BOOL)registerFactories:(Class<ICELocalExceptionFactory>)exception
           connectionInfo:(Class<ICEConnectionInfoFactory>)connectionInfo
             endpointInfo:(Class<ICEEndpointInfoFactory>)endpointInfo
               adminFacet:(Class<ICEAdminFacetFactory>)adminFacet
    NS_SWIFT_NAME(registerFactories(exception:connectionInfo:endpointInfo:adminFacet:));

+ (nullable ICECommunicator*)initialize:(ICEProperties*)properties
                                 logger:(id<ICELoggerProtocol> _Nullable)logger
                                  error:(NSError* _Nullable* _Nullable)error;

+ (ICEProperties*)createProperties;

+ (nullable ICEProperties*)createProperties:(NSArray* _Nullable)swiftArgs
                                   defaults:(ICEProperties* _Nullable)defaults
                                    remArgs:(NSArray* _Null_unspecified* _Null_unspecified)remArgs
                                      error:(NSError* _Nullable* _Nullable)error;
+ (BOOL)stringToIdentity:(NSString*)str
                    name:(NSString* __strong _Nonnull* _Nonnull)name
                category:(NSString* __strong _Nonnull* _Nonnull)category
                   error:(NSError* _Nullable* _Nullable)error NS_SWIFT_NAME(stringToIdentity(str:name:category:));

+ (NSString*)identityToString:(NSString*)name
                     category:(NSString*)category
                         mode:(uint8_t)mode NS_SWIFT_NAME(identityToString(name:category:mode:));
@end

NS_ASSUME_NONNULL_END
