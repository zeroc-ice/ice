//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `Properties.ice'

#import <objc/Ice/Config.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>
#import <objc/Ice/Exception.h>
#import <objc/Ice/BuiltinSequences.h>
#import <objc/Ice/PropertyDict.h>

#ifndef ICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

@protocol ICEProperties;

ICE_API @protocol ICEProperties <NSObject>
-(NSMutableString*) getProperty:(NSString*)key;
-(NSMutableString*) getPropertyWithDefault:(NSString*)key value:(NSString*)value;
-(ICEInt) getPropertyAsInt:(NSString*)key;
-(ICEInt) getPropertyAsIntWithDefault:(NSString*)key value:(ICEInt)value;
-(ICEMutableStringSeq*) getPropertyAsList:(NSString*)key;
-(ICEMutableStringSeq*) getPropertyAsListWithDefault:(NSString*)key value:(ICEStringSeq*)value;
-(ICEMutablePropertyDict*) getPropertiesForPrefix:(NSString*)prefix;
-(void) setProperty:(NSString*)key value:(NSString*)value;
-(ICEMutableStringSeq*) getCommandLineOptions;
-(ICEMutableStringSeq*) parseCommandLineOptions:(NSString*)prefix options:(ICEStringSeq*)options;
-(ICEMutableStringSeq*) parseIceCommandLineOptions:(ICEStringSeq*)options;
-(void) load:(NSString*)file;
-(id<ICEProperties>) clone;
@end
