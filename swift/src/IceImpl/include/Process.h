// Copyright (c) ZeroC, Inc.
#import "Config.h"

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @interface ICEProcess : NSObject
- (void)shutdown;
- (void)writeMessage:(NSString*)message fd:(int32_t)fd;
@end

#ifdef __cplusplus

@interface ICEProcess ()
- (nonnull instancetype)initWithCppProcess:(Ice::ProcessPtr)cppProcess;
@end

#endif

NS_ASSUME_NONNULL_END
