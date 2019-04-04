// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcLocalObject.h"

NS_ASSUME_NONNULL_BEGIN

@interface ICEUnsupportedAdminFacet: ICELocalObject
@end

#ifdef __cplusplus

@interface ICEUnsupportedAdminFacet()
@property (nonatomic, readonly) std::shared_ptr<Ice::Object>facet;
-(instancetype) initWithCppAdminFacet:(std::shared_ptr<Ice::Object>)facet;
@end

#endif

NS_ASSUME_NONNULL_END
