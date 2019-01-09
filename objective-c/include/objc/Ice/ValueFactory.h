// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice/Config.h>
#import <objc/Ice/Object.h>

#if defined(__clang__) && __has_feature(objc_arc)
typedef ICEObject* (^ICEValueFactory)(NSString*) NS_RETURNS_RETAINED;
#else
typedef ICEObject* (^ICEValueFactory)(NSString*);
#endif

ICE_API @protocol ICEValueFactoryManager <NSObject>
-(void) add:(ICEValueFactory)factory sliceId:(NSString*)id_;
-(ICEValueFactory) find:(NSString*)id_;
@end
