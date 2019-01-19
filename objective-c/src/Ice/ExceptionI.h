//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/Exception.h>

#include <Ice/Exception.h>

@interface ICELocalException ()
-(id) initWithLocalException:(const Ice::LocalException&)ex;
-(void) rethrowCxx;
+(id) localExceptionWithLocalException:(const Ice::LocalException&)ex;
@end
