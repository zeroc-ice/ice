//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
