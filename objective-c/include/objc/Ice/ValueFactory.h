// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Config.h>
#import <objc/Ice/Object.h>

typedef ICEObject* (^ICEValueFactory)(NSString*);

ICE_API @protocol ICEValueFactoryManager <NSObject>
-(void) add:(ICEValueFactory)factory sliceId:(NSString*)id_;
-(ICEValueFactory) find:(NSString*)id_;
@end
