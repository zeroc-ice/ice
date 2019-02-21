// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol ICEOutputStreamHelper
-(void) copy:(const void*)start count:(NSNumber*)count;
@end

NS_ASSUME_NONNULL_END
