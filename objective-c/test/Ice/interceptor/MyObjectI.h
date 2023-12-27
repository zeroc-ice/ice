//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <InterceptorTest.h>

@interface TestInterceptorMyObjectI : TestInterceptorMyObject<TestInterceptorMyObject>
@end

@interface TestInterceptorRetryException : ICELocalException
-(NSString *) ice_id;
+(id) retryException:(const char*)file line:(int)line;
@end
