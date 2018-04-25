// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <ProxyTest.h>

@interface TestProxyMyDerivedClassI : TestProxyMyDerivedClass<TestProxyMyDerivedClass>
{
@private
    ICEContext *_ctx;
}
-(id<ICEObjectPrx>) echo:(id<ICEObjectPrx>)proxy current:(ICECurrent*)current;
-(void) shutdown:(ICECurrent*)current;
-(ICEContext*) getContext:(ICECurrent*)current;
-(BOOL) ice_isA:(NSString*)typeId current:(ICECurrent*)current;
@end
