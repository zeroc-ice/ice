// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <DispatcherI.h>
#import <Util.h>
#import <ConnectionI.h>

#include <Block.h>

namespace
{

class DispatcherI : public Ice::Dispatcher
{
public:

// We must explicitely retain/release so that the garbage
// collector does not trash the dispatcher.
DispatcherI(void(^dispatcher)(id<ICEDispatcherCall>, id<ICEConnection>)) : _dispatcher(Block_copy(dispatcher))
{
}

virtual ~DispatcherI()
{
    Block_release(_dispatcher);
}

virtual void 
dispatch(const Ice::DispatcherCallPtr& call, const Ice::ConnectionPtr& connection)
{
    id<ICEConnection> con = [ICEConnection wrapperWithCxxObjectNoAutoRelease:connection.get()];
    id<ICEDispatcherCall> c = [[ICEDispatcherCall alloc] initWithCall:call.get()];
    @try
    {
        _dispatcher(c, con);
    }
    @finally
    {
        [con release];
        [c release];
    }
}

private:

void(^_dispatcher)(id<ICEDispatcherCall>, id<ICEConnection>);

};
typedef IceUtil::Handle<DispatcherI> DispatcherIPtr;

}

@implementation ICEDispatcher
+(Ice::Dispatcher*) dispatcherWithDispatcher:(void(^)(id<ICEDispatcherCall>, id<ICEConnection>))dispatcher
{
    return new DispatcherI(dispatcher);
}
@end

@implementation ICEDispatcherCall
-(id) initWithCall:(Ice::DispatcherCall*)call
{
    self = [super init];
    if(!self)
    {
        return nil;
    }

    cxxCall_ = call;
    cxxCall_->__incRef();
    return self;
}
-(void) dealloc
{
    cxxCall_->__decRef();
    cxxCall_ = 0;
    [super dealloc];
}
-(void) finalize
{
    cxxCall_->__decRef();
    cxxCall_ = 0;
    [super finalize];
}

-(void) run
{
    cppCall(^ { cxxCall_->run(); });
}
@end
