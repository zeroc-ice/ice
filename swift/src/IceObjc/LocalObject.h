//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Config.h"

NS_ASSUME_NONNULL_BEGIN

ICEOBJC_API @interface ICELocalObject : NSObject
//
// We hold a weak referece to the (possile) Swift object which has a handle to
// this ICELocalObject. That way we can recover the Swift object later.
//
@property (weak, atomic, nullable) id swiftRef;
-(instancetype) init ICE_SWIFT_UNAVAILABLE("");
@end

#ifdef __cplusplus

@interface ICELocalObject ()
@property (nonatomic, readonly) std::shared_ptr<void> cppObject;
+(nullable instancetype) getHandle:(std::shared_ptr<void>)cppObject NS_RETURNS_RETAINED;
@end

#endif

NS_ASSUME_NONNULL_END
