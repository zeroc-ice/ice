// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <BatchRequestInterceptorI.h>
#import <Util.h>
#import <ProxyI.h>

#include <Block.h>

@interface ICEBatchRequest : NSObject<ICEBatchRequest>
{
    const Ice::BatchRequest* cxxRequest_;
}
-(void) reset:(const Ice::BatchRequest*)call;
@end

namespace
{

class BatchRequestInterceptorI : public Ice::BatchRequestInterceptor
{
public:

// We must explicitely retain/release so that the garbage
// collector does not trash the dispatcher.
BatchRequestInterceptorI(void(^interceptor)(id<ICEBatchRequest>, int, int)) :
    _interceptor(Block_copy(interceptor)), _request([[ICEBatchRequest alloc] init])
{
}

virtual ~BatchRequestInterceptorI()
{
    Block_release(_interceptor);
    [_request release];
}

virtual void
enqueue(const Ice::BatchRequest& request, int count, int size)
{
    NSException* ex = nil;
    @autoreleasepool
    {
        @try
        {
            [_request reset:&request];
            _interceptor(_request, count, size);
        }
        @catch(id e)
        {
            ex = [e retain];
        }
    }
    if(ex != nil)
    {
        rethrowCxxException(ex, true); // True = release the exception.
    }
}

private:

void(^_interceptor)(id<ICEBatchRequest>, int, int);
ICEBatchRequest* _request;

};
typedef IceUtil::Handle<BatchRequestInterceptorI> BatchRequestInterceptorIPtr;

}

@implementation ICEBatchRequestInterceptor
+(Ice::BatchRequestInterceptor*)
batchRequestInterceptorWithBatchRequestInterceptor:(void(^)(id<ICEBatchRequest>, int, int))interceptor
{
    return new BatchRequestInterceptorI(interceptor);
}
@end

@implementation ICEBatchRequest
-(void) reset:(const Ice::BatchRequest*)request
{
    cxxRequest_ = request;
}
-(void) enqueue
{
    cppCall(^ { cxxRequest_->enqueue(); });
}
-(int) getSize
{
    return cxxRequest_->getSize();
}
-(NSString*) getOperation
{
    return [toNSString(cxxRequest_->getOperation()) autorelease];
}
-(id<ICEObjectPrx>) getProxy
{
    return [ICEObjectPrx iceObjectPrxWithObjectPrx:cxxRequest_->getProxy()];
}
@end
