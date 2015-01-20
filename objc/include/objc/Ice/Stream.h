// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
@protocol ICECommunicator;
@protocol ICESlicedData;
@class ICEUserException;
@class ICEEncodingVersion;

@interface ICEInternalPrefixTable : NSObject
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

@protocol ICEReadObjectCallback <NSObject>
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

@protocol ICEInputStream <NSObject>

-(id<ICECommunicator>) communicator;

-(void) sliceObjects:(BOOL)b;

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

-(void) newObject:(ICEObject*ICE_STRONG_QUALIFIER*)object;
-(void) newObject:(ICEObject*ICE_STRONG_QUALIFIER*)object expectedType:(Class)type;
-(void) readObject:(ICEObject**)object;
-(void) readObject:(ICEObject**)object expectedType:(Class)type;
-(NSMutableArray*) newObjectSeq:(Class)expectedType;
-(NSMutableArray*) readObjectSeq:(Class)expectedType;
-(NSMutableDictionary*) newObjectDict:(Class)keyType expectedType:(Class)type;
-(NSMutableDictionary*) readObjectDict:(Class)keyType expectedType:(Class)type;

-(NSMutableArray*) newSequence:(Class)type;
-(NSMutableArray*) readSequence:(Class)type;
-(NSMutableDictionary*) newDictionary:(ICEKeyValueTypeHelper)type;
-(NSMutableDictionary*) readDictionary:(ICEKeyValueTypeHelper)type;

-(BOOL) readOptional:(ICEInt)tag format:(ICEOptionalFormat)format; 

-(ICEInt) readSize;
-(ICEInt) readAndCheckSeqSize:(ICEInt)minSize;

-(void) throwException;

-(void) startObject;
-(id<ICESlicedData>) endObject:(BOOL)preserve NS_RETURNS_RETAINED;

-(void) startException;
-(id<ICESlicedData>) endException:(BOOL)preserve NS_RETURNS_RETAINED;

-(void) startSlice;
-(void) endSlice;
-(void) skipSlice;

-(ICEEncodingVersion*) startEncapsulation;
-(void) endEncapsulation;
-(ICEEncodingVersion*) skipEncapsulation;

-(ICEEncodingVersion*) getEncoding;

-(void) readPendingObjects;

-(void) rewind;

-(void) skip:(ICEInt)sz;
-(void) skipSize;
@end

@protocol ICEOutputStream <NSObject>

-(id<ICECommunicator>) communicator;

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

-(void) writeObject:(ICEObject*)v;
-(void) writeObjectSeq:(NSArray*)v;
-(void) writeObjectDict:(NSDictionary*)v helper:(Class)helper;

-(void) writeSequence:(NSArray*)arr helper:(Class)helper;
-(void) writeDictionary:(NSDictionary*)dictionary helper:(ICEKeyValueTypeHelper)helper;

-(BOOL) writeOptional:(ICEInt)tag format:(ICEOptionalFormat)format; 

-(void) writeSize:(ICEInt)v;

-(void) writeException:(ICEUserException*)v;

-(void) startObject:(id<ICESlicedData>)slicedData;
-(void) endObject;

-(void) startException:(id<ICESlicedData>)slicedData;
-(void) endException;

-(void) startSlice:(NSString*)typeId compactId:(ICEInt)compactId lastSlice:(BOOL)lastSlice;
-(void) endSlice;

-(void) startEncapsulation;
-(void) startEncapsulation:(ICEEncodingVersion*)encoding format:(ICEFormatType)format;
-(void) endEncapsulation;

-(ICEEncodingVersion*) getEncoding;

-(void) writePendingObjects;

-(NSMutableData*) finished;
-(NSData*) finishedNoCopy;

-(void) reset:(BOOL)clearBuffer;
@end

//
// Helper protocol implemented by helpers for marshaling/un-marshaling
// Slice types.
//
@protocol ICEStreamHelper
+(id) readRetained:(id<ICEInputStream>)stream NS_RETURNS_RETAINED;
+(id) read:(id<ICEInputStream>)stream;
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream;
+(id) readOptRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag;
+(id) readOpt:(id<ICEInputStream>)stream tag:(ICEInt)tag;
+(void) writeOpt:(id)obj stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag;
+(ICEInt) minWireSize;
@end

@interface ICEStreamHelper : NSObject<ICEStreamHelper>
@end

//
// Helper classes for streaming Slice types
//
@interface ICEBoolHelper : ICEStreamHelper
@end

@interface ICEByteHelper : ICEStreamHelper
@end

@interface ICEShortHelper : ICEStreamHelper
@end

@interface ICEIntHelper : ICEStreamHelper
@end

@interface ICELongHelper : ICEStreamHelper
@end

@interface ICEFloatHelper : ICEStreamHelper
@end

@interface ICEDoubleHelper : ICEStreamHelper
@end

@interface ICEStringHelper : ICEStreamHelper
@end

@interface ICEObjectHelper : ICEStreamHelper
+(void)read:(ICEObject**)v stream:(id<ICEInputStream>)stream;
+(void)readOpt:(id*)v stream:(id<ICEInputStream>)stream tag:(ICEInt)tag;
@end

@interface ICEProxyHelper : ICEStreamHelper
@end

@interface ICEEnumHelper : ICEStreamHelper
+(ICEInt) getMinValue;
+(ICEInt) getMaxValue;
@end

@interface ICEStructHelper : ICEStreamHelper
+(Class) getOptionalHelper;
@end

@protocol ICESequenceStreamHelper<ICEStreamHelper>
+(Class) getElementHelper;
+(ICEInt) count:(id)obj;
@end

@interface ICEArraySequenceHelper : ICEStreamHelper<ICESequenceStreamHelper>
+(Class) getOptionalHelper;
@end

@interface ICEDataSequenceHelper : ICEStreamHelper<ICESequenceStreamHelper>
@end

@interface ICEBoolSequenceHelper : ICEDataSequenceHelper
@end

@interface ICEByteSequenceHelper : ICEDataSequenceHelper
@end

@interface ICEShortSequenceHelper : ICEDataSequenceHelper
@end

@interface ICEIntSequenceHelper : ICEDataSequenceHelper
@end

@interface ICELongSequenceHelper : ICEDataSequenceHelper
@end

@interface ICEFloatSequenceHelper : ICEDataSequenceHelper
@end

@interface ICEDoubleSequenceHelper : ICEDataSequenceHelper
@end

@interface ICEEnumSequenceHelper : ICEDataSequenceHelper
@end

@interface ICEStringSequenceHelper : ICEArraySequenceHelper
@end

@interface ICEObjectSequenceHelper : ICEArraySequenceHelper
@end

@interface ICEProxySequenceHelper : ICEArraySequenceHelper
@end

@protocol ICEDictionaryStreamHelper<ICEStreamHelper>
+(ICEInt) count:(id)obj;
@end

@interface ICEDictionaryHelper : ICEStreamHelper<ICEDictionaryStreamHelper>
+(Class) getOptionalHelper;
@end

@interface ICEObjectDictionaryHelper : ICEDictionaryHelper
@end

//
// Helper for optionals
//
@protocol ICEOptionalStreamHelper
+(id) readRetained:(id<ICEInputStream>)stream helper:(Class)helper NS_RETURNS_RETAINED;
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream helper:(Class)helper;
+(ICEOptionalFormat) optionalFormat;
@end

@interface ICEFixedLengthOptionalHelper : NSObject<ICEOptionalStreamHelper>
@end

@interface ICEVarLengthOptionalHelper : NSObject<ICEOptionalStreamHelper>
@end

@interface ICEFixedSequenceOptionalHelper : NSObject<ICEOptionalStreamHelper>
@end

@interface ICEFixedSize1SequenceOptionalHelper : NSObject<ICEOptionalStreamHelper>
@end

@interface ICEFixedDictionaryOptionalHelper : NSObject<ICEOptionalStreamHelper>
@end
 
@interface CompactIdMapHelper : NSObject
+(void) initialize;
+(void) registerClass:(NSString*)type value:(ICEInt)value;
@end

@interface ICEOptionalGetter : NSObject
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
