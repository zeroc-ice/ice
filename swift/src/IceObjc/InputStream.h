// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "Config.h"

NS_ASSUME_NONNULL_BEGIN

@interface ICEInputStream : NSObject
-(instancetype) init ICE_SWIFT_UNAVAILABLE("ICEInputStream cannot be initialized from Swift");
-(void*) data;
-(size_t) size;
@end

#ifdef __cplusplus

@interface ICEInputStream()
@property (nonatomic, readonly) std::vector<Ice::Byte> bytes;
-(instancetype) initWithBytes:(std::vector<Ice::Byte>)bytes;
@end

#endif

NS_ASSUME_NONNULL_END
