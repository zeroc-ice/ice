// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "Config.h"

NS_ASSUME_NONNULL_BEGIN

@interface ICELocalObject : NSObject
@property (weak, nonatomic, nullable) id swiftRef;
-(instancetype) init ICE_SWIFT_UNAVAILABLE("");
@end

#ifdef __cplusplus

@interface ICELocalObject ()
@property (nonatomic, readonly) void* _Nullable object;
-(instancetype) initWithLocalObject:(void*)object;
+(nullable instancetype) fromLocalObject:(void*)object NS_RETURNS_RETAINED;
@end

#endif

NS_ASSUME_NONNULL_END
