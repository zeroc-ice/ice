//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "LocalObject.h"

@class ICECommunicator;
@class ICEProcess;
@class ICEPropertiesAdmin;
@class ICEUnsupportedAdminFacet;
@protocol ICEBlobjectFacade;

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @protocol ICEAdminFacetFactory
+(id<ICEBlobjectFacade>) createProcess:(ICECommunicator*)communicator handle:(ICEProcess*)handle;
+(id<ICEBlobjectFacade>) createProperties:(ICECommunicator*)communicator handle:(ICEPropertiesAdmin*)handle;
+(id<ICEBlobjectFacade>) createUnsupported:(ICECommunicator*)communicator handle:(ICEUnsupportedAdminFacet*)handle;
@end

NS_ASSUME_NONNULL_END
