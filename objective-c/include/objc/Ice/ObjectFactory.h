//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/Config.h>

ICE_API @protocol ICEObjectFactory <NSObject>
-(ICEValue*) create:(NSString*)sliceId NS_RETURNS_RETAINED;
-(void) destroy;
@end
