//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @protocol ICEOutputStreamHelper
-(void) copy:(void*)bytes
       count:(long)count;
@end

NS_ASSUME_NONNULL_END
