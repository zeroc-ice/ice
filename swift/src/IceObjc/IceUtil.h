//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Communicator.h"
#import "Connection.h"
#import "Endpoint.h"
#import "Exception.h"
#import "Logger.h"
#import "Properties.h"
#import "AdminFacetFactory.h"

NS_ASSUME_NONNULL_BEGIN

//
// Utility methods
//
@interface ICEUtil: NSObject
@property (class, nonatomic, readonly) Class<ICEExceptionFactory> exceptionFactory;
@property (class, nonatomic, readonly) Class<ICEConnectionInfoFactory> connectionInfoFactory;
@property (class, nonatomic, readonly) Class<ICEEndpointInfoFactory> endpointInfoFactory;
@property (class, nonatomic, readonly) Class<ICEAdminFacetFactory> adminFacetFactory;

//This method should only be called once to guarenteed thread safety
+(BOOL) registerFactories:(Class<ICEExceptionFactory>)exception
           connectionInfo:(Class<ICEConnectionInfoFactory>)connectionInfo
             endpointInfo:(Class<ICEEndpointInfoFactory>)endpointInfo
               adminFacet:(Class<ICEAdminFacetFactory>)adminFacet
NS_SWIFT_NAME(registerFactories(exception:connectionInfo:endpointInfo:adminFacet:));

+(nullable ICECommunicator*) initialize:(NSArray*)swiftArgs
                             properties:(ICEProperties*)properties
                         withConfigFile:(BOOL)withConfiFile
                                remArgs:(NSArray* _Null_unspecified * _Null_unspecified)remArgs
                                  error:(NSError* _Nullable * _Nullable)error;

+(ICEProperties*) createProperties;

+(nullable ICEProperties*) createProperties:(NSArray* _Nullable)swiftArgs
                                   defaults:(ICEProperties* _Nullable)defaults
                                    remArgs:(NSArray* _Null_unspecified * _Null_unspecified)remArgs
                                      error:(NSError* _Nullable * _Nullable)error;
+(BOOL) stringToIdentity:(NSString*)str
                    name:(NSString* __strong _Nonnull *  _Nonnull)name
                category:(NSString* __strong  _Nonnull *  _Nonnull)category
                   error:(NSError* _Nullable * _Nullable)error NS_SWIFT_NAME(stringToIdentity(str:name:category:));

+(NSString*) identityToString:(NSString*)name
                              category:(NSString*)category
                                  mode:(uint8_t)mode
    NS_SWIFT_NAME(identityToString(name:category:mode:));

+(NSString*) encodingVersionToString:(UInt8)major
                               minor:(UInt8)minor NS_SWIFT_NAME(encodingVersionToString(major:minor:));

+(nullable NSString*) escapeString:(NSString*)string
                           special:(NSString*)special
                              communicator:(ICECommunicator*)communicator
                             error:(NSError* _Nullable * _Nullable)error
    NS_SWIFT_NAME(escapeString(string:special:communicator:));

+(NSString*) errorToString:(int32_t)error NS_SWIFT_NAME(errorToString(_:));
+(NSString*) errorToStringDNS:(int32_t)error NS_SWIFT_NAME(errorToStringDNS(_:));
@end

NS_ASSUME_NONNULL_END
