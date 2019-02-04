//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/ObjectAdapter.h>

#import <objc/Ice/LocalObject.h>

#include <Ice/ObjectAdapter.h>

@class ICECommunicator;

@interface ICEObjectAdapter : ICELocalObject<ICEObjectAdapter>
-(Ice::ObjectAdapter*) adapter;
@end
