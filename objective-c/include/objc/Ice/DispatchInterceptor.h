//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/Object.h>

ICE_API @protocol ICEDispatchInterceptor <ICEObject>
-(void) dispatch:(id<ICERequest>)request;
@end

ICE_API @interface ICEDispatchInterceptor : ICEServant
@end

ICE_API @interface ICEMainThreadDispatch : ICEDispatchInterceptor<ICEDispatchInterceptor>
{
    ICEObject* servant;
}

-(id)init:(ICEObject*)servant;
+(id)mainThreadDispatch:(ICEObject*)servant;
@end
