//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>

@interface InterceptorI : ICEDispatchInterceptor <ICEDispatchInterceptor>
{
    ICEObject* servant;
    NSString* lastOperation;
    BOOL lastStatus;
}

-(id) init:(ICEObject*)servant;
-(BOOL) getLastStatus;
-(NSString*) getLastOperation;
-(void) clear;

@end
