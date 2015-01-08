// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Config.h>
#import <objc/Ice/Stream.h>

//
// Forward declarations
//
@protocol ICEObjectAdapter;
@protocol ICEConnection;
@class ICEIdentity;
@class ICEEncodingVersion;

typedef NSDictionary ICEContext;
typedef NSMutableDictionary ICEMutableContext;

typedef enum
{
    ICENormal,
    ICENonmutating,
    ICEIdempotent
} ICEOperationMode;

@interface ICECurrent : NSObject <NSCopying>
{
    @private
        id<ICEObjectAdapter> adapter;
        id<ICEConnection> con;
        ICEIdentity *id_;
        NSString *facet;
        NSString *operation;
        ICEOperationMode mode;
        NSDictionary *ctx;
        ICEInt requestId;
        ICEEncodingVersion *encoding;
}

@property(nonatomic, retain) id<ICEObjectAdapter> adapter;
@property(nonatomic, retain) id<ICEConnection> con;
@property(nonatomic, retain) ICEIdentity *id_;
@property(nonatomic, retain) NSString *facet;
@property(nonatomic, retain) NSString *operation;
@property(nonatomic, assign) ICEOperationMode mode;
@property(nonatomic, retain) NSDictionary *ctx;
@property(nonatomic, assign) ICEInt requestId;
@property(nonatomic, assign) ICEEncodingVersion *encoding;

-(id) init:(id<ICEObjectAdapter>)adapter 
       con:(id<ICEConnection>)con_ 
       id_:(ICEIdentity *)id_ 
     facet:(NSString *)facet 
 operation:(NSString *)operation 
      mode:(ICEOperationMode)mode 
       ctx:(NSDictionary *)ctx 
 requestId:(ICEInt)requestId 
  encoding:(ICEEncodingVersion*)encoding;

+(id) current:(id<ICEObjectAdapter>)adapter 
          con:(id<ICEConnection>)con_ 
          id_:(ICEIdentity *)id_ 
        facet:(NSString *)facet 
    operation:(NSString *)operation 
         mode:(ICEOperationMode)mode 
          ctx:(NSDictionary *)ctx 
    requestId:(ICEInt)requestId
     encoding:(ICEEncodingVersion*)encoding;

+(id) current;
// This class also overrides copyWithZone:, hash, isEqual:, and dealloc.
@end

@interface ICEContextHelper : ICEDictionaryHelper
@end

@interface ICEOperationModeHelper : ICEEnumHelper
+(ICEInt) getMinValue;
+(ICEInt) getMaxValue;
@end
