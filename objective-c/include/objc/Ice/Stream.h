// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Config.h>
#import <objc/Ice/Format.h> // for ICEFormatType

#import <Foundation/NSData.h>

//
// Forward declarations
//
@class ICEObject;
@protocol ICEObjectPrx;
@protocol ICESlicedData;
@class ICEUserException;
@class ICEEncodingVersion;

ICE_API @interface ICEInternalPrefixTable : NSObject
@end

typedef enum
{
    ICEOptionalFormatF1 = 0,
    ICEOptionalFormatF2 = 1,
    ICEOptionalFormatF4 = 2,
    ICEOptionalFormatF8 = 3,
    ICEOptionalFormatSize = 4,
    ICEOptionalFormatVSize = 5,
    ICEOptionalFormatFSize = 6,
    ICEOptionalFormatClass = 7
} ICEOptionalFormat;

ICE_API @protocol ICEReadObjectCallback <NSObject>
-(void)invoke:(ICEObject*)obj;
@end

//
// ICENone singleton to specify not set optionals.
//
extern id ICENone;

typedef struct
{
    Class key;
    Class value;
} ICEKeyValueTypeHelper;

ICE_API @protocol ICEInputStream <NSObject>

-(void) setSliceValues:(BOOL)b;

-(BOOL) readBool;
-(NSMutableData*) newBoolSeq;
-(NSMutableData*) readBoolSeq;

-(ICEByte) readByte;
-(NSMutableData*) newByteSeq;
-(NSMutableData*) readByteSeq;
-(NSData*) readByteSeqNoCopy;

-(ICEShort) readShort;
-(NSMutableData*) newShortSeq;
-(NSMutableData*) readShortSeq;

-(ICEInt) readInt;
-(NSMutableData*) newIntSeq;
-(NSMutableData*) readIntSeq;

-(ICELong) readLong;
-(NSMutableData*) newLongSeq;
-(NSMutableData*) readLongSeq;

-(ICEFloat) readFloat;
-(NSMutableData*) newFloatSeq;
-(NSMutableData*) readFloatSeq;

-(ICEDouble) readDouble;
-(NSMutableData*) newDoubleSeq;
-(NSMutableData*) readDoubleSeq;

-(NSMutableString*) newString;
-(NSMutableString*) readString;
-(NSMutableArray*) newStringSeq;
-(NSMutableArray*) readStringSeq;

-(ICEInt) readEnumerator:(ICEInt)min max:(ICEInt)max;
-(NSMutableData*) newEnumSeq:(ICEInt)min max:(ICEInt)max;
-(NSMutableData*) readEnumSeq:(ICEInt)min max:(ICEInt)max;

-(id<ICEObjectPrx>) newProxy:(Class)c;
-(id<ICEObjectPrx>) readProxy:(Class)c;

-(void) newValue:(ICEObject*ICE_STRONG_QUALIFIER*)object;
-(void) newValue:(ICEObject*ICE_STRONG_QUALIFIER*)object expectedType:(Class)type;
-(void) readValue:(ICEObject**)object;
-(void) readValue:(ICEObject**)object expectedType:(Class)type;
-(NSMutableArray*) newValueSeq:(Class)expectedType;
-(NSMutableArray*) readValueSeq:(Class)expectedType;
-(NSMutableDictionary*) newValueDict:(Class)keyType expectedType:(Class)type;
-(NSMutableDictionary*) readValueDict:(Class)keyType expectedType:(Class)type;

-(NSMutableArray*) newSequence:(Class)type;
-(NSMutableArray*) readSequence:(Class)type;
-(NSMutableDictionary*) newDictionary:(ICEKeyValueTypeHelper)type;
-(NSMutableDictionary*) readDictionary:(ICEKeyValueTypeHelper)type;

-(BOOL) readOptional:(ICEInt)tag format:(ICEOptionalFormat)format;

-(ICEInt) readSize;
-(ICEInt) readAndCheckSeqSize:(ICEInt)minSize;

-(void) throwException;

-(void) startValue;
-(id<ICESlicedData>) endValue:(BOOL)preserve NS_RETURNS_RETAINED;

-(void) startException;
-(id<ICESlicedData>) endException:(BOOL)preserve NS_RETURNS_RETAINED;

-(void) startSlice;
-(void) endSlice;
-(void) skipSlice;

-(ICEEncodingVersion*) startEncapsulation;
-(void) endEncapsulation;
-(ICEEncodingVersion*) skipEmptyEncapsulation;
-(ICEEncodingVersion*) skipEncapsulation;

-(ICEEncodingVersion*) getEncoding;

-(void) readPendingValues;

-(void) rewind;

-(void) skip:(ICEInt)sz;
-(void) skipSize;
@end

ICE_API @protocol ICEOutputStream <NSObject>

-(void) writeBool:(BOOL)v;
-(void) writeBoolSeq:(NSData*)v;

-(void) writeByte:(ICEByte)v;
-(void) writeByteSeq:(NSData*)v;

-(void) writeShort:(ICEShort)v;
-(void) writeShortSeq:(NSData*)v;

-(void) writeInt:(ICEInt)v;
-(void) writeIntSeq:(NSData*)v;

-(void) writeLong:(ICELong)v;
-(void) writeLongSeq:(NSData*)v;

-(void) writeFloat:(ICEFloat)v;
-(void) writeFloatSeq:(NSData*)v;

-(void) writeDouble:(ICEDouble)v;
-(void) writeDoubleSeq:(NSData*)v;

-(void) writeString:(NSString*)v;
-(void) writeStringSeq:(NSArray*)v;

-(void) writeEnumerator:(ICEInt)v min:(ICEInt)min max:(ICEInt)max;
-(void) writeEnumSeq:(NSData*)v min:(ICEInt)min max:(ICEInt)max;

-(void) writeProxy:(id<ICEObjectPrx>)v;

-(void) writeValue:(ICEObject*)v;
-(void) writeValueSeq:(NSArray*)v;
-(void) writeValueDict:(NSDictionary*)v helper:(Class)helper;

-(void) writeSequence:(NSArray*)arr helper:(Class)helper;
-(void) writeDictionary:(NSDictionary*)dictionary helper:(ICEKeyValueTypeHelper)helper;

-(BOOL) writeOptional:(ICEInt)tag format:(ICEOptionalFormat)format;

-(void) writeSize:(ICEInt)v;

-(void) writeException:(ICEUserException*)v;

-(void) startValue:(id<ICESlicedData>)slicedData;
-(void) endValue;

-(void) startException:(id<ICESlicedData>)slicedData;
-(void) endException;

-(void) startSlice:(NSString*)typeId compactId:(ICEInt)compactId lastSlice:(BOOL)lastSlice;
-(void) endSlice;

-(void) startEncapsulation;
-(void) startEncapsulation:(ICEEncodingVersion*)encoding format:(ICEFormatType)format;
-(void) endEncapsulation;

-(ICEEncodingVersion*) getEncoding;

-(void) writePendingValues;

-(NSMutableData*) finished;
-(NSData*) finishedNoCopy;

-(void) reset:(BOOL)clearBuffer;
@end

//
// Helper protocol implemented by helpers for marshaling/un-marshaling
// Slice types.
//
ICE_API @protocol ICEStreamHelper
+(id) readRetained:(id<ICEInputStream>)stream NS_RETURNS_RETAINED;
+(id) read:(id<ICEInputStream>)stream;
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream;
+(id) readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag;
+(id) readOptional:(id<ICEInputStream>)stream tag:(ICEInt)tag;
+(void) writeOptional:(id)obj stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag;
+(ICEInt) minWireSize;
@end

ICE_API @interface ICEStreamHelper : NSObject<ICEStreamHelper>
@end

//
// Helper classes for streaming Slice types
//
ICE_API @interface ICEBoolHelper : ICEStreamHelper
@end

ICE_API @interface ICEByteHelper : ICEStreamHelper
@end

ICE_API @interface ICEShortHelper : ICEStreamHelper
@end

ICE_API @interface ICEIntHelper : ICEStreamHelper
@end

ICE_API @interface ICELongHelper : ICEStreamHelper
@end

ICE_API @interface ICEFloatHelper : ICEStreamHelper
@end

ICE_API @interface ICEDoubleHelper : ICEStreamHelper
@end

ICE_API @interface ICEStringHelper : ICEStreamHelper
@end

ICE_API @interface ICEObjectHelper : ICEStreamHelper
+(void)read:(ICEObject**)v stream:(id<ICEInputStream>)stream;
+(void)readOptional:(id*)v stream:(id<ICEInputStream>)stream tag:(ICEInt)tag;
@end

ICE_API @interface ICEProxyHelper : ICEStreamHelper
@end

ICE_API @interface ICEEnumHelper : ICEStreamHelper
+(ICEInt) getMinValue;
+(ICEInt) getMaxValue;
@end

ICE_API @interface ICEStructHelper : ICEStreamHelper
+(void) readRetained:(id<ICEInputStream>)stream value:(id ICE_STRONG_QUALIFIER*)v;
+(void) read:(id<ICEInputStream>)stream value:(id*)v;
+(Class) getOptionalHelper;
@end

ICE_API @protocol ICESequenceStreamHelper<ICEStreamHelper>
+(Class) getElementHelper;
+(ICEInt) count:(id)obj;
@end

ICE_API @interface ICEArraySequenceHelper : ICEStreamHelper<ICESequenceStreamHelper>
+(Class) getOptionalHelper;
@end

ICE_API @interface ICEDataSequenceHelper : ICEStreamHelper<ICESequenceStreamHelper>
@end

ICE_API @interface ICEBoolSequenceHelper : ICEDataSequenceHelper
@end

ICE_API @interface ICEByteSequenceHelper : ICEDataSequenceHelper
@end

ICE_API @interface ICEShortSequenceHelper : ICEDataSequenceHelper
@end

ICE_API @interface ICEIntSequenceHelper : ICEDataSequenceHelper
@end

ICE_API @interface ICELongSequenceHelper : ICEDataSequenceHelper
@end

ICE_API @interface ICEFloatSequenceHelper : ICEDataSequenceHelper
@end

ICE_API @interface ICEDoubleSequenceHelper : ICEDataSequenceHelper
@end

ICE_API @interface ICEEnumSequenceHelper : ICEDataSequenceHelper
@end

ICE_API @interface ICEStringSequenceHelper : ICEArraySequenceHelper
@end

ICE_API @interface ICEObjectSequenceHelper : ICEArraySequenceHelper
@end

ICE_API @interface ICEProxySequenceHelper : ICEArraySequenceHelper
@end

ICE_API @protocol ICEDictionaryStreamHelper<ICEStreamHelper>
+(ICEInt) count:(id)obj;
@end

ICE_API @interface ICEDictionaryHelper : ICEStreamHelper<ICEDictionaryStreamHelper>
+(Class) getOptionalHelper;
@end

ICE_API @interface ICEObjectDictionaryHelper : ICEDictionaryHelper
@end

//
// Helper for optionals
//
ICE_API @protocol ICEOptionalStreamHelper
+(id) readRetained:(id<ICEInputStream>)stream helper:(Class)helper NS_RETURNS_RETAINED;
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream helper:(Class)helper;
+(ICEOptionalFormat) optionalFormat;
@end

ICE_API @interface ICEFixedLengthOptionalHelper : NSObject<ICEOptionalStreamHelper>
@end

ICE_API @interface ICEVarLengthOptionalHelper : NSObject<ICEOptionalStreamHelper>
@end

ICE_API @interface ICEFixedSequenceOptionalHelper : NSObject<ICEOptionalStreamHelper>
@end

ICE_API @interface ICEFixedSize1SequenceOptionalHelper : NSObject<ICEOptionalStreamHelper>
@end

ICE_API @interface ICEFixedDictionaryOptionalHelper : NSObject<ICEOptionalStreamHelper>
@end

ICE_API @interface CompactIdMapHelper : NSObject
+(void) initialize;
+(void) registerClass:(NSString*)type value:(ICEInt)value;
@end

ICE_API @interface ICEOptionalGetter : NSObject
+(BOOL) get:(id)value value:(id ICE_STRONG_QUALIFIER*)v type:(Class)cl;
+(BOOL) getRetained:(id)value value:(id ICE_STRONG_QUALIFIER*)v type:(Class)cl;
+(BOOL) getByte:(id)value value:(ICEByte*)v;
+(BOOL) getBool:(id)value value:(BOOL*)v;
+(BOOL) getShort:(id)value value:(ICEShort*)v;
+(BOOL) getInt:(id)value value:(ICEInt*)v;
+(BOOL) getLong:(id)value value:(ICELong*)v;
+(BOOL) getFloat:(id)value value:(ICEFloat*)v;
+(BOOL) getDouble:(id)value value:(ICEDouble*)v;
@end
