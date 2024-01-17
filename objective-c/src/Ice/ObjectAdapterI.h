//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/ObjectAdapter.h>

#include <Ice/ObjectAdapter.h>

@interface ICEObjectAdapter : NSObject<ICEObjectAdapter>
{
    std::shared_ptr<Ice::ObjectAdapter> adapter_;
}
-(id) initWithCxxObject:(const std::shared_ptr<Ice::ObjectAdapter>&)arg;
-(std::shared_ptr<Ice::ObjectAdapter>) cxxObject;
+(id) objectAdapterWithCxxObject:(const std::shared_ptr<Ice::ObjectAdapter>&)arg;
+(id) objectAdapterWithCxxObjectNoAutoRelease:(const std::shared_ptr<Ice::ObjectAdapter>&)arg;
@end
