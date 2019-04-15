//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
