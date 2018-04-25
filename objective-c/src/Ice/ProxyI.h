// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Proxy.h>

#include <Ice/Proxy.h>

@interface ICEAsyncResult : NSObject<ICEAsyncResult>
{
@private
    void* asyncResult_;
    NSString* operation_;
    id<ICEObjectPrx> proxy_;
}
-(ICEAsyncResult*)initWithAsyncResult:(const Ice::AsyncResultPtr&)arg operation:(NSString*)op proxy:(id<ICEObjectPrx>)p;
-(Ice::AsyncResult*) asyncResult;
+(ICEAsyncResult*)asyncResultWithAsyncResult:(const Ice::AsyncResultPtr&)arg;
+(ICEAsyncResult*)asyncResultWithAsyncResult:(const Ice::AsyncResultPtr&)arg operation:(NSString*)op proxy:(id<ICEObjectPrx>)p;
-(NSString*)operation;
@end

@interface ICEObjectPrx ()
-(ICEObjectPrx*)iceInitWithObjectPrx:(const Ice::ObjectPrx&)arg;
-(IceProxy::Ice::Object*) iceObjectPrx;
+(ICEObjectPrx*)iceObjectPrxWithObjectPrx:(const Ice::ObjectPrx&)arg;
@end
