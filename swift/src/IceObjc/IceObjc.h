// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <Foundation/Foundation.h>

#if TARGET_OS_IPHONE
#   import <UIKit/UIKit.h>
#else
#   import <Cocoa/Cocoa.h>
#endif

FOUNDATION_EXPORT double IceObjcVersionNumber;
FOUNDATION_EXPORT const unsigned char IceObjcVersionString[];

#import <IceObjc/Communicator.h>
#import <IceObjc/ImplicitContext.h>
#import <IceObjc/LocalObject.h>
#import <IceObjc/OutputStream.h>
#import <IceObjc/Connection.h>
#import <IceObjc/InputStream.h>
#import <IceObjc/Logger.h>
#import <IceObjc/Properties.h>
#import <IceObjc/Endpoint.h>
#import <IceObjc/IceUtil.h>
#import <IceObjc/LocalException.h>
#import <IceObjc/ObjectPrx.h>
