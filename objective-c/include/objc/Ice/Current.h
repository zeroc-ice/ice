//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `Current.ice'

#import <objc/Ice/Config.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>
#import <objc/Ice/Exception.h>
#import <objc/Ice/ObjectAdapterF.h>
#import <objc/Ice/ConnectionF.h>
#import <objc/Ice/Context.h>
#import <objc/Ice/Identity.h>
#import <objc/Ice/OperationMode.h>
#import <objc/Ice/Version.h>

#ifndef ICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

ICE_API @interface ICECurrent : NSObject <NSCopying>
{
    @private
        ICELocalObject<ICEObjectAdapter> *adapter;
        ICELocalObject<ICEConnection> *con;
        ICEIdentity *id_;
        NSString *facet;
        NSString *operation;
        ICEOperationMode mode;
        ICEContext *ctx;
        ICEInt requestId;
        ICEEncodingVersion *encoding;
}

@property(nonatomic, ICE_STRONG_ATTR) ICELocalObject<ICEObjectAdapter> *adapter;
@property(nonatomic, ICE_STRONG_ATTR) ICELocalObject<ICEConnection> *con;
@property(nonatomic, ICE_STRONG_ATTR) ICEIdentity *id_;
@property(nonatomic, ICE_STRONG_ATTR) NSString *facet;
@property(nonatomic, ICE_STRONG_ATTR) NSString *operation;
@property(nonatomic, assign) ICEOperationMode mode;
@property(nonatomic, ICE_STRONG_ATTR) ICEContext *ctx;
@property(nonatomic, assign) ICEInt requestId;
@property(nonatomic, ICE_STRONG_ATTR) ICEEncodingVersion *encoding;

-(id) init;
-(id) init:(ICELocalObject<ICEObjectAdapter> *)adapter con:(ICELocalObject<ICEConnection> *)con id:(ICEIdentity*)id_ facet:(NSString*)facet operation:(NSString*)operation mode:(ICEOperationMode)mode ctx:(ICEContext*)ctx requestId:(ICEInt)requestId encoding:(ICEEncodingVersion*)encoding;
+(id) current;
+(id) current:(ICELocalObject<ICEObjectAdapter> *)adapter con:(ICELocalObject<ICEConnection> *)con id:(ICEIdentity*)id_ facet:(NSString*)facet operation:(NSString*)operation mode:(ICEOperationMode)mode ctx:(ICEContext*)ctx requestId:(ICEInt)requestId encoding:(ICEEncodingVersion*)encoding;
// This class also overrides copyWithZone:
// This class also overrides dealloc
// This class also overrides hash, and isEqual:
@end
