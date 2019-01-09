// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice/Config.h>

ICE_API @protocol ICEObjectFactory <NSObject>
-(ICEObject*) create:(NSString*)sliceId NS_RETURNS_RETAINED;
-(void) destroy;
@end
