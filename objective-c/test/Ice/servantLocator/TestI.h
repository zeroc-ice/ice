//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <ServantLocatorTest.h>

@interface TestServantLocatorTestIntfI : TestServantLocatorTestIntf<TestServantLocatorTestIntf>
@end

// TODO: simplify this cookie code

@protocol TestServantLocatorCookie <NSObject>
-(NSMutableString*) message;
@end

@interface TestServantLocatorCookie : ICELocalObject
+(id) cookie;
@end

@interface TestServantLocatorCookieI : TestServantLocatorCookie<TestServantLocatorCookie>
@end
