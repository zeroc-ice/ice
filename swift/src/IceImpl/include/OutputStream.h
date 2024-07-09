// Copyright (c) ZeroC, Inc.
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @protocol ICEOutputStreamHelper
- (void)copy:(const void*)bytes count:(long)count;
@end

NS_ASSUME_NONNULL_END
