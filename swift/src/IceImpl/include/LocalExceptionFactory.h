// Copyright (c) ZeroC, Inc.
#import "Config.h"
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @protocol ICELocalExceptionFactory

// Local exceptions with the same fields share a factory method.

// DispatchException and the 3 UnknownXxxException. Don't use for the NotExist exceptions.
+ (NSError*)dispatchException:(uint8_t)replyStatus message:(NSString*)message file:(NSString*)file line:(int32_t)line;

// The 3 NoExist exceptions.
+ (NSError*)requestFailedException:(uint8_t)replyStatus
                              name:(NSString*)name
                          category:(NSString*)category
                             facet:(NSString*)facet
                         operation:(NSString*)operation
                              file:(NSString*)file
                              line:(int32_t)line;

// AlreadyRegisteredException + NotRegisteredException
+ (NSError*)registeredException:(NSString*)typeId
                   kindOfObject:(NSString*)kindOfObject
                       objectId:(NSString*)objectId
                        message:(NSString*)message
                           file:(NSString*)file
                           line:(int32_t)line;

// ConnectionAbortedException + ConnectionClosedException
+ (NSError*)connectionClosedException:(NSString*)typeId
                  closedByApplication:(BOOL)closedByApplication
                              message:(NSString*)message
                                 file:(NSString*)file
                                 line:(int32_t)line;

// All other local exceptions.
+ (NSError*)localException:(NSString*)typeId message:(NSString*)message file:(NSString*)file line:(int32_t)line;

// Other std::exception
+ (NSError*)cxxException:(NSString*)typeName message:(NSString*)message;
@end

NS_ASSUME_NONNULL_END
