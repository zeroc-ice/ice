// Copyright (c) ZeroC, Inc.
#import "LocalObject.h"

@class ICECommunicator;
@class ICEProcess;
@class ICEPropertiesAdmin;
@protocol ICEDispatchAdapter;

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @protocol ICEAdminFacetFactory
+ (id<ICEDispatchAdapter>)createProcess:(ICECommunicator*)communicator handle:(ICEProcess*)handle;
+ (id<ICEDispatchAdapter>)createProperties:(ICECommunicator*)communicator handle:(ICEPropertiesAdmin*)handle;
+ (id<ICEDispatchAdapter>)createUnsupported:(ICECommunicator*)communicator;
@end

NS_ASSUME_NONNULL_END
