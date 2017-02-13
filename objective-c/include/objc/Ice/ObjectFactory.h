// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Config.h>

ICE_API @protocol ICEObjectFactory <NSObject>
-(ICEObject*) create:(NSString*)sliceId NS_RETURNS_RETAINED;
-(void) destroy;
@end

