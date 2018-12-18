// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <RetryTest.h>

@interface TestRetryRetryI : TestRetryRetry
{
    int _counter;
}
-(void) op:(BOOL)kill current:(ICECurrent *)current;
-(void) shutdown:(ICECurrent *)current;
@end
