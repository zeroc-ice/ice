//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol ICEOutputStreamHelper
-(void) copy:(const void*)start count:(NSNumber*)count;
@end

NS_ASSUME_NONNULL_END
