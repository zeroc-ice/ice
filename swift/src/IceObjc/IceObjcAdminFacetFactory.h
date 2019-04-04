// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcLocalObject.h"

@class ICECommunicator;
@class ICEProcess;
@class ICEPropertiesAdmin;
@class ICEUnsupportedAdminFacet;
@protocol ICEBlobjectFacade;

NS_ASSUME_NONNULL_BEGIN

@protocol ICEAdminFacetFactory
+(id<ICEBlobjectFacade>) createProcess:(ICECommunicator*)communicator handle:(ICEProcess*)handle;
+(id<ICEBlobjectFacade>) createProperties:(ICECommunicator*)communicator handle:(ICEPropertiesAdmin*)handle;
+(id<ICEBlobjectFacade>) createUnsupported:(ICECommunicator*)communicator handle:(ICEUnsupportedAdminFacet*)handle;
@end

NS_ASSUME_NONNULL_END
