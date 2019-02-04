//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/Initialize.h>
#import <objc/Ice/Connection.h>

#import <objc/Ice/LocalObject.h>

#include <Ice/Dispatcher.h>

@interface ICEDispatcher : NSObject
+(Ice::Dispatcher*)dispatcherWithDispatcher:(void(^)(id<ICEDispatcherCall>, id<ICEConnection>))arg;
@end

@interface ICEDispatcherCall : NSObject<ICEDispatcherCall>
{
    Ice::DispatcherCall* cxxCall_;
}
-(id) initWithCall:(Ice::DispatcherCall*)call;
@end
