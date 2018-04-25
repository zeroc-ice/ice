// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/ObjectAdapter.h>

#import <objc/Ice/LocalObject.h>

#include <Ice/ObjectAdapter.h>

@class ICECommunicator;

@interface ICEObjectAdapter : ICELocalObject<ICEObjectAdapter>
-(Ice::ObjectAdapter*) adapter;
@end
