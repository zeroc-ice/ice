//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/ImplicitContext.h>

#include <Ice/ImplicitContext.h>

@interface ICEImplicitContext : NSObject<ICEImplicitContext>
{
@private

    Ice::ImplicitContext* implicitContext_;
}
-(id) init:(Ice::ImplicitContext*)implicitContext;
+(id) implicitContextWithImplicitContext:(Ice::ImplicitContext*)implicitContext;
@end
