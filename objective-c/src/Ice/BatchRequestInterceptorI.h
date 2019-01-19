//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/Initialize.h>

#include <Ice/BatchRequestInterceptor.h>

@interface ICEBatchRequestInterceptor : NSObject
+(Ice::BatchRequestInterceptor*)
batchRequestInterceptorWithBatchRequestInterceptor:(void(^)(id<ICEBatchRequest>, int, int))arg;
@end
