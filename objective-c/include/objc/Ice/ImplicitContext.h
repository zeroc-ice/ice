//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `ImplicitContext.ice'

#import <objc/Ice/Config.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>
#import <objc/Ice/Exception.h>
#import <objc/Ice/LocalException.h>
#import <objc/Ice/Current.h>

#ifndef ICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

@protocol ICEImplicitContext;

ICE_API @protocol ICEImplicitContext <NSObject>
-(ICEMutableContext*) getContext;
-(void) setContext:(ICEContext*)newContext;
-(BOOL) containsKey:(NSString*)key;
-(NSMutableString*) get:(NSString*)key;
-(NSMutableString*) put:(NSString*)key value:(NSString*)value;
-(NSMutableString*) remove:(NSString*)key;
@end
