// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Config.h>
#import <objc/Ice/Current.h>

@protocol ICEImplicitContext<NSObject>
-(ICEMutableContext*) getContext;
-(void) setContext:(ICEContext*)context;
-(BOOL) containsKey:(NSString*)key;
-(NSString*) get:(NSString*)key;
-(NSString*) put:(NSString*)key value:(NSString*)value;
-(NSString*) remove:(NSString*)key;
@end
