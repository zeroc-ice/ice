// Copyright (c) ZeroC, Inc.
#import "Config.h"

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @interface ICELocalObject : NSObject
//
// We hold a weak reference to the (possible) Swift object which has a handle to
// this ICELocalObject. That way we can recover the Swift object later.
//
@property(weak, atomic, nullable) id swiftRef;
- (instancetype)init ICE_SWIFT_UNAVAILABLE("");
@end

#ifdef __cplusplus

@interface ICELocalObject ()
@property(nonatomic, readonly) std::shared_ptr<void> cppObject;
+ (nullable instancetype)getHandle:(std::shared_ptr<void>)cppObject NS_RETURNS_RETAINED;
@end

#endif

NS_ASSUME_NONNULL_END
