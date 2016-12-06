// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <StreamI.h>
#import <CommunicatorI.h>
#import <ProxyI.h>
#import <Util.h>
#import <ObjectI.h>
#import <SlicedDataI.h>
#import <VersionI.h>
#import <LocalObjectI.h>

#import <objc/Ice/LocalException.h>

#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/SlicedData.h>

#import <objc/runtime.h>

ICE_API id ICENone = nil;

namespace IceObjC
{

class ValueWrapper : public Ice::Object
{
public:

    // We must explicitely CFRetain/CFRelease so that the garbage
    // collector does not trash the _obj.
    ValueWrapper(ICEObject* obj) : _obj(obj)
    {
        CFRetain(_obj);
    }

    virtual ~ValueWrapper()
    {
        CFRelease(_obj);
    }

    virtual void
    _iceWrite(Ice::OutputStream* stream) const
    {
        @try
        {
            [_obj iceWrite:static_cast<ICEOutputStream*>(stream->getClosure())];
        }
        @catch(id ex)
        {
            rethrowCxxException(ex);
        }
    }

    virtual void
    _iceRead(Ice::InputStream* stream)
    {
        @try
        {
            [_obj iceRead:static_cast<ICEInputStream*>(stream->getClosure())];
        }
        @catch(id ex)
        {
            rethrowCxxException(ex);
        }
    }

    virtual void ice_preMarshal()
    {
        [_obj ice_preMarshal];
    }

    virtual void ice_postUnmarshal()
    {
        [_obj ice_postUnmarshal];
    }

    ICEObject*
    getValue()
    {
        return _obj;
    }

private:

    ICEObject* _obj;
};
typedef IceUtil::Handle<ValueWrapper> ValueWrapperPtr;

class ReadValueBase
{
public:

    ReadValueBase(Class expectedType) : _expectedType(expectedType)
    {
    }

    virtual
    ~ReadValueBase()
    {
    }

    virtual void invoke(const Ice::ValuePtr&) = 0;
    void checkType(ICEObject*);

private:

    Class _expectedType;
};

void
patchFunc(void* obj, const Ice::ValuePtr& value)
{
    ReadValueBase* reader = static_cast<ReadValueBase*>(obj);
    reader->invoke(value);
    delete reader;
}

void
ReadValueBase::checkType(ICEObject* o)
{
    if(o != nil && ![o isKindOfClass:_expectedType])
    {
        NSString* actualType = [o ice_id];
        NSString* expectedType = [_expectedType ice_staticId];
        NSString* reason = [NSString stringWithFormat:@"expected element of type `%@' but received `%@'",
                                     expectedType, actualType];

        @throw [ICEUnexpectedObjectException unexpectedObjectException:__FILE__
                                             line:__LINE__
                                             reason:reason
                                             type:actualType
                                             expectedType:expectedType];
    }
}

class ReadValue : public ReadValueBase
{
public:

    ReadValue(ICEObject** addr, Class expectedType, bool autorelease) :
        ReadValueBase(expectedType), _addr(addr), _autorelease(autorelease)
    {
    }

    virtual void
    invoke(const Ice::ValuePtr& obj)
    {
        @try
        {
            if(obj)
            {
                ICEObject* o = ValueWrapperPtr::dynamicCast(obj)->getValue();
                checkType(o);
                *_addr = [o retain];
                if(_autorelease)
                {
                    [*_addr autorelease];
                }
            }
            else
            {
                *_addr = nil;
            }
        }
        @catch(id ex)
        {
            rethrowCxxException(ex);
        }
    }

private:

    ICEObject** _addr;
    bool _autorelease;
};

class ReadValueAtIndex : public ReadValueBase
{
public:

    ReadValueAtIndex(NSMutableArray* array, ICEInt index, Class expectedType) :
        ReadValueBase(expectedType), _array(array), _index(index)
    {
    }

    virtual void
    invoke(const Ice::ValuePtr& obj)
    {
        @try
        {
            if(obj)
            {
                ICEObject* o = ValueWrapperPtr::dynamicCast(obj)->getValue();
                checkType(o);
                [_array replaceObjectAtIndex:_index withObject:o];
            }
        }
        @catch(id ex)
        {
            rethrowCxxException(ex);
        }
    }

private:

    NSMutableArray* _array;
    ICEInt _index;
};

class ReadValueForKey : public ReadValueBase
{
public:

    // We must explicitely CFRetain/CFRelease so that the garbage
    // collector does not trash the _key.
    ReadValueForKey(NSMutableDictionary* dict, id key, Class expectedType) :
        ReadValueBase(expectedType), _dict(dict), _key(key)
    {
        CFRetain(_key);
    }

    virtual ~ReadValueForKey()
    {
        CFRelease(_key);
    }

    virtual void
    invoke(const Ice::ValuePtr& obj)
    {
        @try
        {
            if(obj)
            {
                ICEObject* o = ValueWrapperPtr::dynamicCast(obj)->getValue();
                checkType(o);
                [_dict setObject:o forKey:_key];
            }
            else
            {
                [_dict setObject:[NSNull null] forKey:_key];
            }
        }
        @catch(id ex)
        {
            rethrowCxxException(ex);
        }
    }

private:

    NSMutableDictionary* _dict;
    id _key;
};

class ExceptionWrapper : public Ice::UserException
{
public:

    ExceptionWrapper(ICEUserException* ex) : _ex(ex)
    {
    }

    virtual bool
    _usesClasses() const
    {
        return [_ex iceUsesClasses];
    }

    virtual std::string
    ice_id() const
    {
        return [[_ex ice_id] UTF8String];
    }

    virtual Ice::UserException*
    ice_clone() const
    {
        return new ExceptionWrapper(*this);
    }

    virtual void
    ice_throw() const
    {
        throw *this;
    }

    virtual void
    _write(Ice::OutputStream* s) const
    {
        [_ex iceWrite:static_cast<ICEOutputStream*>(s->getClosure())];
    }

    virtual void
    _read(Ice::InputStream* s)
    {
        [_ex iceRead:static_cast<ICEInputStream*>(s->getClosure())];
    }

    ICEUserException*
    getException() const
    {
        return _ex;
    }

protected:

    virtual void _writeImpl(Ice::OutputStream*) const {}
    virtual void _readImpl(Ice::InputStream*) {}

private:

    ICEUserException* _ex;
};

class UserExceptionFactoryI : public Ice::UserExceptionFactory
{
public:

    UserExceptionFactoryI(NSDictionary* prefixTable) : _prefixTable(prefixTable)
    {
    }

    virtual void createAndThrow(const std::string& typeId)
    {
        ICEUserException* ex = nil;
        Class c = objc_lookUpClass(toObjCSliceId(typeId, _prefixTable).c_str());
        if(c != nil)
        {
            ex = [[c alloc] init];
            throw ExceptionWrapper(ex);
        }
    }

private:

    NSDictionary* _prefixTable;
};

}

@implementation ICEInternalPrefixTable
@end

@interface ICEInternalNone : NSObject
@end

@implementation ICEInternalNone
+(void)load
{
    ICENone = [[ICEInternalNone alloc] init];
}
-(id) copyWithZone:(NSZone *)zone
{
    return self;
}
-(id) retain
{
    return self;
}
-(NSUInteger) retainCount
{
    return NSUIntegerMax;
}
-(oneway void) release
{
}
-(id) autorelease
{
    return self;
}
@end

@implementation ICEInputStream
-(id) initWithCxxCommunicator:(Ice::Communicator*)com data:(const std::pair<const Byte*, const Byte*>&)data
{
    self = [super init];
    if(!self)
    {
        return nil;
    }

    Ice::InputStream(com, data).swap(stream_);
    is_ = &stream_;
    is_->setClosure(self);
    data_ = nil;
    prefixTable_ = [[[ICECommunicator localObjectWithCxxObject:com] getPrefixTable] retain];
    return self;
}

-(id) initWithCommunicator:(id<ICECommunicator>)communicator data:(NSData*)data encoding:(ICEEncodingVersion*)e
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    ICECommunicator* com = (ICECommunicator*)communicator;
    std::pair<const Ice::Byte*, const Ice::Byte*> p((Ice::Byte*)[data bytes], (Ice::Byte*)[data bytes] + [data length]);
    if(e != nil)
    {
        Ice::InputStream([com communicator], [e encodingVersion], p).swap(stream_);
    }
    else
    {
        Ice::InputStream([com communicator], p).swap(stream_);
    }
    is_ = &stream_;
    is_->setClosure(self);
    data_ = [data retain];
    prefixTable_ = [[com getPrefixTable] retain];
    return self;
}

-(void) dealloc
{
    [data_ release];
    [prefixTable_ release];
    [super dealloc];
}

+(Ice::Object*)createObjectReader:(ICEObject*)obj
{
    return new IceObjC::ValueWrapper(obj);
}

-(Ice::InputStream*) is
{
    return is_;
}

// @protocol ICEInputStream methods

-(void) setSliceValues:(BOOL)b
{
    NSException* nsex = nil;
    try
    {
        is_->setSliceValues(b);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(BOOL)readBool
{
    NSException* nsex = nil;
    try
    {
        bool value;
        is_->read(value);
        return value;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return NO; // Keep the compiler happy.
}

-(NSMutableData*) readBoolSeq
{
    return [[self newBoolSeq] autorelease];
}

-(NSMutableData*) newBoolSeq
{
    NSException* nsex = nil;
    try
    {
        std::pair<const bool*, const bool*> seq;
        IceUtil::ScopedArray<bool> result;
        is_->read(seq, result);
        return [[NSMutableData alloc] initWithBytes:seq.first length:(seq.second - seq.first) * sizeof(BOOL)];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(ICEByte) readByte
{
    NSException* nsex = nil;
    try
    {
        Ice::Byte value;
        is_->read(value);
        return value;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return 0; // Keep the compiler happy.
}

-(NSMutableData*) readByteSeq
{
    return [[self newByteSeq] autorelease];
}

-(NSMutableData*) newByteSeq
{
    NSException* nsex = nil;
    try
    {
        std::pair<const Ice::Byte*, const Ice::Byte*> seq;
        is_->read(seq);
        return [[NSMutableData alloc] initWithBytes:seq.first length:(seq.second - seq.first)];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}

-(NSData*) readByteSeqNoCopy
{
    NSException* nsex = nil;
    try
    {
        std::pair<const Ice::Byte*, const Ice::Byte*> seq;
        is_->read(seq);
        return [NSData dataWithBytesNoCopy:const_cast<Ice::Byte*>(seq.first)
                       length:(seq.second - seq.first) freeWhenDone:NO];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(ICEShort) readShort
{
    NSException* nsex = nil;
    try
    {
        Ice::Short value;
        is_->read(value);
        return value;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return 0; // Keep the compiler happy.
}

-(NSMutableData*) readShortSeq
{
    return [[self newShortSeq] autorelease];
}

-(NSMutableData*) newShortSeq
{
    NSException* nsex = nil;
    try
    {
        std::pair<const Ice::Short*, const Ice::Short*> seq;
        IceUtil::ScopedArray<Ice::Short> result;
        is_->read(seq, result);
        return [[NSMutableData alloc] initWithBytes:seq.first length:(seq.second - seq.first) * sizeof(ICEShort)];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(ICEInt) readInt
{
    NSException* nsex = nil;
    try
    {
        Ice::Int value;
        is_->read(value);
        return value;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return 0; // Keep the compiler happy.
}

-(NSMutableData*) readIntSeq
{
    return [[self newIntSeq] autorelease];
}

-(NSMutableData*) newIntSeq
{
    NSException* nsex = nil;
    try
    {
        std::pair<const Ice::Int*, const Ice::Int*> seq;
        IceUtil::ScopedArray<Ice::Int> result;
        is_->read(seq, result);
        return [[NSMutableData alloc] initWithBytes:seq.first length:(seq.second - seq.first) * sizeof(ICEInt)];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(ICELong) readLong
{
    NSException* nsex = nil;
    try
    {
        Ice::Long value;
        is_->read(value);
        return value;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return 0; // Keep the compiler happy.
}

-(NSMutableData*) readLongSeq
{
    return [[self newLongSeq] autorelease];
}

-(NSMutableData*) newLongSeq
{
    NSException* nsex = nil;
    try
    {
        std::pair<const Ice::Long*, const Ice::Long*> seq;
        IceUtil::ScopedArray<Ice::Long> result;
        is_->read(seq, result);
        return [[NSMutableData alloc] initWithBytes:seq.first length:(seq.second - seq.first) * sizeof(ICELong)];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(ICEFloat) readFloat
{
    NSException* nsex = nil;
    try
    {
        Ice::Float value;
        is_->read(value);
        return value;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return 0.f; // Keep the compiler happy.
}

-(NSMutableData*) readFloatSeq
{
    return [[self newFloatSeq] autorelease];
}

-(NSMutableData*) newFloatSeq
{
    NSException* nsex = nil;
    try
    {
        std::pair<const Ice::Float*, const Ice::Float*> seq;
        IceUtil::ScopedArray<Ice::Float> result;
        is_->read(seq, result);
        return [[NSMutableData alloc] initWithBytes:seq.first length:(seq.second - seq.first) * sizeof(ICEFloat)];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(ICEDouble) readDouble
{
    NSException* nsex = nil;
    try
    {
        Ice::Double value;
        is_->read(value);
        return value;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return 0.0; // Keep the compiler happy.
}

-(NSMutableData*) readDoubleSeq
{
    return [[self newDoubleSeq] autorelease];
}

-(NSMutableData*) newDoubleSeq
{
    NSException* nsex = nil;
    try
    {
        std::pair<const Ice::Double*, const Ice::Double*> seq;
        IceUtil::ScopedArray<Ice::Double> result;
        is_->read(seq, result);
        return [[NSMutableData alloc] initWithBytes:seq.first length:(seq.second - seq.first) * sizeof(ICEDouble)];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(NSMutableString*)readString
{
    return [[self newString] autorelease];
}

-(NSMutableString*)newString
{
    NSException* nsex = nil;
    try
    {
        std::string value;
        is_->read(value);
        return toNSMutableString(value);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(NSMutableArray*) readStringSeq
{
    return [[self newStringSeq] autorelease];
}

-(NSMutableArray*) newStringSeq
{
    NSException* nsex = nil;
    try
    {
        Ice::StringSeq value;
        is_->read(value);
        return toNSArray(value);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(ICEInt) readEnumerator:(ICEInt)min max:(ICEInt)max
{
    NSException* nsex = nil;
    ICEInt val = 0; // Keep the compiler happy.
    try
    {
        if(is_->getEncoding() == Ice::Encoding_1_0)
        {
            if(max <= 0x7f)
            {
                val = [self readByte];
            }
            else if(max <= 0x7fff)
            {
                val = [self readShort];
            }
            else
            {
                val = [self readInt];
            }
        }
        else
        {
            val = [self readSize];
        }
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    if(val > max || val < min)
    {
        @throw [ICEMarshalException marshalException:__FILE__ line:__LINE__ reason:@"enumerator out of range"];
    }
    return val;
}

//
// Size of an enum. Slice enum underling type is specified as ICEInt so
// all Slice enums have the same size as an ICEInt.
//
#define ENUM_SIZE sizeof(ICEInt)

-(NSMutableData*) readEnumSeq:(ICEInt)min max:(ICEInt)max
{
    return [[self newEnumSeq:(ICEInt)min max:(ICEInt)max] autorelease];
}

-(NSMutableData*) newEnumSeq:(ICEInt)min max:(ICEInt)max
{
    NSException* nsex = nil;
    NSMutableData* ret = 0;
    try
    {
        int count = is_->readSize();
        if((ret = [[NSMutableData alloc] initWithLength:(count * ENUM_SIZE)]) == 0)
        {
            return ret;
        }

        ICEInt *v = (ICEInt *)[ret bytes];
        while(count-- > 0)
        {
            *v++ = [self readEnumerator:min max:max];
        }
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return ret;
}

-(ICEInt) readSize
{
    NSException* nsex = nil;
    try
    {
        return is_->readSize();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return 0; // Keep the compiler happy.
}

-(ICEInt) readAndCheckSeqSize:(ICEInt)minSize
{
    NSException* nsex = nil;
    try
    {
        return is_->readAndCheckSeqSize(minSize);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return 0; // Keep the compiler happy.
}

-(id<ICEObjectPrx>) readProxy:(Class)type
{
    return [[self newProxy:type] autorelease];
}

-(id<ICEObjectPrx>) newProxy:(Class)type
{
    NSException* nsex = nil;
    try
    {
        Ice::ObjectPrx p;
        is_->read(p);
        return p ? [[type alloc] iceInitWithObjectPrx:p] : nil;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(void) readValue:(ICEObject**)object
{
    [self readValue:object expectedType:[ICEObject class]];
}
-(void) readValue:(ICEObject**)object expectedType:(Class)type
{
    NSException* nsex = nil;
    try
    {
        is_->read(IceObjC::patchFunc, new IceObjC::ReadValue(object, type, true));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}
-(void) newValue:(ICEObject*ICE_STRONG_QUALIFIER*)object
{
    [self newValue:object expectedType:[ICEObject class]];
}
-(void) newValue:(ICEObject*ICE_STRONG_QUALIFIER*)object expectedType:(Class)type
{
    NSException* nsex = nil;
    try
    {
        is_->read(IceObjC::patchFunc, new IceObjC::ReadValue(object, type, false));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(NSMutableArray*) readValueSeq:(Class)type
{
    return [[self newValueSeq:(Class)type] autorelease];
}

-(NSMutableArray*) newValueSeq:(Class)type
{
    ICEInt sz = [self readAndCheckSeqSize:1];
    NSMutableArray* arr = [[NSMutableArray alloc] initWithCapacity:sz];
    NSException* nsex = nil;
    try
    {
        int i;
        id null = [NSNull null];
        for(i = 0; i < sz; i++)
        {
            [arr addObject:null];
            is_->read(IceObjC::patchFunc, new IceObjC::ReadValueAtIndex(arr, i, type));
        }
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        [arr release];
        @throw nsex;
    }
    return arr;
}

-(NSMutableDictionary*) readValueDict:(Class)keyHelper expectedType:(Class)type
{
    return [[self newValueDict:(Class)keyHelper expectedType:(Class)type] autorelease];
}

-(NSMutableDictionary*) newValueDict:(Class)keyHelper expectedType:(Class)type
{
    ICEInt sz = [self readAndCheckSeqSize:[keyHelper minWireSize] + 1];
    NSMutableDictionary* dictionary = [[NSMutableDictionary alloc] initWithCapacity:sz];
    id key = nil;
    for(int i = 0; i < sz; ++i)
    {
        @try
        {
            key = [keyHelper readRetained:self];
        }
        @catch(id ex)
        {
            [dictionary release];
            @throw ex;
        }

        NSException* nsex = nil;
        try
        {
            is_->read(IceObjC::patchFunc, new IceObjC::ReadValueForKey(dictionary, key, type));
        }
        catch(const std::exception& ex)
        {
            nsex = toObjCException(ex);
        }
        if(nsex != nil)
        {
            [key release];
            [dictionary release];
            @throw nsex;
        }
        [key release];
    }
    return dictionary;
}

-(NSMutableArray*) readSequence:(Class)helper
{
    return [[self newSequence:(Class)helper] autorelease];
}

-(NSMutableArray*) newSequence:(Class)helper
{
    ICEInt sz = [self readAndCheckSeqSize:[helper minWireSize]];
    NSMutableArray* arr = [[NSMutableArray alloc] initWithCapacity:sz];
    id obj = nil;
    @try
    {
        while(sz-- > 0)
        {
            obj = [helper readRetained:self];
            if(obj == nil)
            {
                [arr addObject:[NSNull null]];
            }
            else
            {
                [arr addObject:obj];
                [obj release];
            }
        }
    }
    @catch(id ex)
    {
        [arr release];
        [obj release];
        @throw ex;
    }
    return arr;
}

-(NSMutableDictionary*) readDictionary:(ICEKeyValueTypeHelper)helper
{
    return [[self newDictionary:(ICEKeyValueTypeHelper)helper] autorelease];
}

-(NSMutableDictionary*) newDictionary:(ICEKeyValueTypeHelper)helper
{
    ICEInt sz = [self readAndCheckSeqSize:[helper.key minWireSize] + [helper.value minWireSize]];
    NSMutableDictionary* dictionary = [[NSMutableDictionary alloc] initWithCapacity:sz];
    id key = nil;
    id value = nil;
    @try
    {
        while(sz-- > 0)
        {
            key = [helper.key readRetained:self];
            value = [helper.value readRetained:self];
            if(value == nil)
            {
                [dictionary setObject:[NSNull null] forKey:key];
            }
            else
            {
                [dictionary setObject:value forKey:key];
                [value release];
            }
            [key release];
        }
    }
    @catch(id ex)
    {
        [dictionary release];
        [key release];
        [value release];
        @throw ex;
    }
    return dictionary;
}

-(BOOL) readOptional:(ICEInt)tag format:(ICEOptionalFormat)format
{
    NSException* nsex = nil;
    try
    {
        return is_->readOptional(tag, static_cast<Ice::OptionalFormat>(format));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return NO; // Keep the compiler happy.
}

-(void) throwException
{
    ICEUserException* ex = nil;
    NSException* nsex = nil;
    try
    {
        is_->throwException(new IceObjC::UserExceptionFactoryI(prefixTable_));
    }
    catch(const IceObjC::ExceptionWrapper& e)
    {
        ex = e.getException();
        @throw [ex autorelease]; // NOTE: exceptions are always auto-released, no need for the caller to do it.
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
        @throw nsex;
    }
}

-(void) startValue
{
    NSException* nsex = nil;
    try
    {
        is_->startValue();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(id<ICESlicedData>) endValue:(BOOL)preserve
{
    NSException* nsex = nil;
    try
    {
        Ice::SlicedDataPtr slicedData = is_->endValue(preserve);
        return slicedData ? [[ICESlicedData alloc] initWithSlicedData:slicedData.get()] : nil;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return nil; // Make compiler happy
}

-(void) startException
{
    NSException* nsex = nil;
    try
    {
        is_->startException();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(id<ICESlicedData>) endException:(BOOL)preserve
{
    NSException* nsex = nil;
    try
    {
        Ice::SlicedDataPtr slicedData = is_->endException(preserve);
        return slicedData ? [[ICESlicedData alloc] initWithSlicedData:slicedData.get()] : nil;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return nil; // Make compiler happy
}

-(void) startSlice
{
    NSException* nsex = nil;
    try
    {
        is_->startSlice();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) endSlice
{
    NSException* nsex = nil;
    try
    {
        is_->endSlice();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) skipSlice
{
    NSException* nsex = nil;
    try
    {
        is_->skipSlice();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(ICEEncodingVersion*) startEncapsulation
{
    NSException* nsex = nil;
    try
    {
        return [ICEEncodingVersion encodingVersionWithEncodingVersion:is_->startEncapsulation()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return nil; // Keep the compiler happy.
}

-(void) endEncapsulation
{
    NSException* nsex = nil;
    try
    {
        is_->endEncapsulation();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(ICEEncodingVersion*) skipEmptyEncapsulation
{
    NSException* nsex = nil;
    try
    {
        return [ICEEncodingVersion encodingVersionWithEncodingVersion:is_->skipEmptyEncapsulation()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return nil; // Keep the compiler happy.
}

-(ICEEncodingVersion*) skipEncapsulation
{
    NSException* nsex = nil;
    try
    {
        return [ICEEncodingVersion encodingVersionWithEncodingVersion:is_->skipEncapsulation()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return nil; // Keep the compiler happy.
}

-(ICEEncodingVersion*) getEncoding
{
    return [ICEEncodingVersion encodingVersionWithEncodingVersion:is_->getEncoding()];
}

-(void) readPendingValues
{
    NSException* nsex = nil;
    try
    {
        is_->readPendingValues();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) rewind
{
    NSException* nsex = nil;
    try
    {
        is_->pos(0);
        is_->clear();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}


-(void) skip:(ICEInt)sz
{
    NSException* nsex = nil;
    try
    {
        is_->skip(sz);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) skipSize
{
    NSException* nsex = nil;
    try
    {
        is_->skipSize();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

@end

@implementation ICEOutputStream
-(id) initWithCxxCommunicator:(Ice::Communicator*)com
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    Ice::OutputStream(com).swap(stream_);
    os_ = &stream_;
    os_->setClosure(self);
    objectWriters_ = 0;
    return self;
}

-(id) initWithCxxStream:(Ice::OutputStream*)stream
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    os_ = stream;
    os_->setClosure(self);
    objectWriters_ = 0;
    return self;
}

-(id) initWithCommunicator:(id<ICECommunicator>)communicator encoding:(ICEEncodingVersion*)e
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    ICECommunicator* com = (ICECommunicator*)communicator;
    if(e != nil)
    {
        Ice::OutputStream([com communicator], [e encodingVersion]).swap(stream_);
    }
    else
    {
        Ice::OutputStream([com communicator]).swap(stream_);
    }
    os_ = &stream_;
    os_->setClosure(self);
    objectWriters_ = 0;
    return self;
}

-(Ice::OutputStream*) os
{
    return os_;
}

-(void) dealloc
{
    if(objectWriters_)
    {
        delete objectWriters_;
    }
    [super dealloc];
}

// @protocol ICEOutputStream methods

-(void)writeBool:(BOOL)v
{
    NSException* nsex = nil;
    try
    {
        os_->write(static_cast<bool>(v));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeBoolSeq:(NSData*)v
{
    NSException* nsex = nil;
    try
    {
        v == nil ? os_->writeSize(0) : os_->write((bool*)[v bytes], (bool*)[v bytes] + [v length] / sizeof(BOOL));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeByte:(ICEByte)v
{
    NSException* nsex = nil;
    try
    {
        os_->write(static_cast<Ice::Byte>(v));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeByteSeq:(NSData*)v
{
    NSException* nsex = nil;
    try
    {
        v == nil ? os_->writeSize(0) : os_->write((ICEByte*)[v bytes], (ICEByte*)[v bytes] + [v length]);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeShort:(ICEShort)v
{
    NSException* nsex = nil;
    try
    {
        os_->write(static_cast<Ice::Short>(v));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeShortSeq:(NSData*)v
{
    NSException* nsex = nil;
    try
    {
        v == nil ? os_->writeSize(0)
                 : os_->write((ICEShort*)[v bytes], (ICEShort*)[v bytes] + [v length] / sizeof(ICEShort));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeInt:(ICEInt)v
{
    NSException* nsex = nil;
    try
    {
        os_->write(static_cast<Ice::Int>(v));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeIntSeq:(NSData*)v
{
    NSException* nsex = nil;
    try
    {
        v == nil ? os_->writeSize(0)
                 : os_->write((ICEInt*)[v bytes], (ICEInt*)[v bytes] + [v length] / sizeof(ICEInt));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeLong:(ICELong)v
{
    NSException* nsex = nil;
    try
    {
        os_->write(static_cast<Ice::Long>(v));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeLongSeq:(NSData*)v
{
    NSException* nsex = nil;
    try
    {
        v == nil ? os_->writeSize(0)
                 : os_->write((ICELong*)[v bytes], (ICELong*)[v bytes] + [v length] / sizeof(ICELong));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeFloat:(ICEFloat)v
{
    NSException* nsex = nil;
    try
    {
        os_->write(static_cast<Ice::Float>(v));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeFloatSeq:(NSData*)v
{
    NSException* nsex = nil;
    try
    {
        v == nil ? os_->writeSize(0)
                 : os_->write((ICEFloat*)[v bytes], (ICEFloat*)[v bytes] + [v length] / sizeof(ICEFloat));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeDouble:(ICEDouble)v
{
    NSException* nsex = nil;
    try
    {
        os_->write(static_cast<Ice::Double>(v));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeDoubleSeq:(NSData*)v
{
    NSException* nsex = nil;
    try
    {
        v == nil ? os_->writeSize(0)
                 : os_->write((ICEDouble*)[v bytes],
                                      (ICEDouble*)[v bytes] + [v length] / sizeof(ICEDouble));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeString:(NSString*)v
{
    NSException* nsex = nil;
    try
    {
        os_->write(fromNSString(v));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeStringSeq:(NSArray*)v
{
    NSException* nsex = nil;
    try
    {
        std::vector<std::string> s;
        os_->write(fromNSArray(v, s));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void)writeSequence:(NSArray*)arr helper:(Class)helper
{
    if(arr == nil)
    {
        [self writeSize:0];
        return;
    }

    [self writeSize:[arr count]];
    for(id i in arr)
    {
        [helper write:(i == [NSNull null] ? nil : i) stream:self];
    }
}

-(void) writeDictionary:(NSDictionary*)dictionary helper:(ICEKeyValueTypeHelper)helper
{
    if(dictionary == nil)
    {
        [self writeSize:0];
        return;
    }

    [self writeSize:[dictionary count]];
    NSEnumerator* e = [dictionary keyEnumerator];
    id key;
    while((key = [e nextObject]))
    {
        if(key == [NSNull null])
        {
            @throw [ICEMarshalException marshalException:__FILE__ line:__LINE__ reason:@"illegal NSNull value"];
        }
        [helper.key write:key stream:self];
        NSObject *obj = [dictionary objectForKey:key];
        [helper.value write:(obj == [NSNull null] ? nil : obj) stream:self];
    }
}

-(BOOL) writeOptional:(ICEInt)tag format:(ICEOptionalFormat)format
{
    NSException* nsex = nil;
    try
    {
        return os_->writeOptional(tag, static_cast<Ice::OptionalFormat>(format));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return NO; // Keep the compiler happy.
}

-(void) writeEnumerator:(ICEInt)v min:(int)min max:(int)max
{
    if(v > max || v < min)
    {
        @throw [ICEMarshalException marshalException:__FILE__ line:__LINE__ reason:@"enumerator out of range"];
    }
    NSException* nsex = nil;
    try
    {
        if(os_->getEncoding() == Ice::Encoding_1_0)
        {
            if(max <= 0x7f)
            {
                os_->write(static_cast<Ice::Byte>(v));
            }
            else if(max <= 0x7fff)
            {
                os_->write(static_cast<Ice::Short>(v));
            }
            else
            {
                os_->write(static_cast<Ice::Int>(v));
            }
        }
        else
        {
            os_->writeSize(static_cast<Ice::Int>(v));
        }
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

//
// The C standard does not fix the size of an enum. The compiler is free
// to choose an enum size that depends on the number of enumerators, and
// the choice may vary depending on the processor. This means that we don't
// know what the size of an enum is until run time, so the marshaling
// has to be generic and copy with enums that could be 8, 16, or 32 bits wide.
//
-(void) writeEnumSeq:(NSData*)v min:(ICEInt)min max:(ICEInt)max
{
    NSException* nsex = nil;
    try
    {
        int count = v == nil ? 0 : [v length] / ENUM_SIZE;
        [self writeSize:count];
        if(count == 0)
        {
            return;
        }

        const ICEInt* p = (const ICEInt*)[v bytes];
        while(count-- > 0)
        {
            [self writeEnumerator:*p++ min:min max:max];
        }
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeSize:(ICEInt)v
{
    NSException* nsex = nil;
    try
    {
        os_->writeSize(v);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}


-(void) writeProxy:(id<ICEObjectPrx>)v
{
    NSException* nsex = nil;
    try
    {
        os_->write(Ice::ObjectPrx([(ICEObjectPrx*)v iceObjectPrx]));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeValue:(ICEObject*)v
{
    NSException* nsex = nil;
    try
    {
        if(v == nil)
        {
            os_->write(Ice::ValuePtr());
        }
        else
        {
            os_->write(Ice::ValuePtr([self addObject:v]));
        }
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) writeValueSeq:(NSArray*)arr
{
    if(arr == nil)
    {
        [self writeSize:0];
        return;
    }

    [self writeSize:[arr count]];
    for(id i in arr)
    {
        [self writeValue:(i == [NSNull null] ? nil : i)];
    }
}

-(void) writeValueDict:(NSDictionary*)dictionary helper:(Class)helper
{
    if(dictionary == nil)
    {
        [self writeSize:0];
        return;
    }

    [self writeSize:[dictionary count]];
    NSEnumerator* e = [dictionary keyEnumerator];
    id key;
    while((key = [e nextObject]))
    {
        if(key == [NSNull null])
        {
            @throw [ICEMarshalException marshalException:__FILE__ line:__LINE__ reason:@"illegal NSNull value"];
        }
        [helper write:key stream:self];
        id obj = [dictionary objectForKey:key];
        [self writeValue:(obj == [NSNull null] ? nil : obj)];
    }
}

-(void) writeException:(ICEUserException*)ex
{
    IceObjC::ExceptionWrapper writer(ex);
    os_->writeException(writer);
}

-(void) startValue:(id<ICESlicedData>)slicedData
{
    NSException* nsex = nil;
    try
    {
        if(slicedData != nil)
        {
            os_->startValue([self writeSlicedData:slicedData]);
        }
        else
        {
            os_->startValue(0);
        }
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) endValue
{
    NSException* nsex = nil;
    try
    {
        os_->endValue();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) startException:(id<ICESlicedData>)slicedData
{
    NSException* nsex = nil;
    try
    {
        if(slicedData != nil)
        {
            os_->startException([self writeSlicedData:slicedData]);
        }
        else
        {
            os_->startException(0);
        }
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) endException
{
    NSException* nsex = nil;
    try
    {
        os_->endException();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) startSlice:(NSString*)typeId compactId:(ICEInt)compactId lastSlice:(BOOL)lastSlice
{
    NSException* nsex = nil;
    try
    {
        os_->startSlice([typeId UTF8String], compactId, static_cast<bool>(lastSlice));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) endSlice
{
    NSException* nsex = nil;
    try
    {
        os_->endSlice();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) startEncapsulation
{
    NSException* nsex = nil;
    try
    {
        os_->startEncapsulation();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) startEncapsulation:(ICEEncodingVersion*)encoding format:(ICEFormatType)format
{
    NSException* nsex = nil;
    try
    {
        os_->startEncapsulation([encoding encodingVersion], (Ice::FormatType)format);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) endEncapsulation
{
    NSException* nsex = nil;
    try
    {
        os_->endEncapsulation();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(ICEEncodingVersion*) getEncoding
{
    return [ICEEncodingVersion encodingVersionWithEncodingVersion:os_->getEncoding()];
}

-(void) writePendingValues
{
    NSException* nsex = nil;
    try
    {
        os_->writePendingValues();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(NSMutableData*) finished
{
    NSException* nsex = nil;
    try
    {
        std::vector<Ice::Byte> buf;
        os_->finished(buf);
        return [NSMutableData dataWithBytes:&buf[0] length:buf.size()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(NSData*) finishedNoCopy
{
    NSException* nsex = nil;
    try
    {
        std::pair<const Ice::Byte*, const Ice::Byte*> b = os_->finished();
        return [NSData dataWithBytesNoCopy:const_cast<Ice::Byte*>(b.first) length:(b.second - b.first) freeWhenDone:NO];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(void) reset:(BOOL)clearBuffer
{
    NSException* nsex = nil;
    try
    {
        os_->clear();
        os_->resize(0);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(Ice::Object*) addObject:(ICEObject*)object
{
    //
    // Ice::ValueWrapper is a subclass of Ice::Object that wraps an Objective-C object for marshaling.
    // It is possible that this Objective-C object has already been marshaled, therefore we first must
    // check the object map to see if this object is present. If so, we use the existing ValueWrapper,
    // otherwise we create a new one.
    //
    if(!objectWriters_)
    {
        objectWriters_ = new std::map<ICEObject*, Ice::ValuePtr>();
    }
    std::map<ICEObject*, Ice::ValuePtr>::const_iterator p = objectWriters_->find(object);
    if(p != objectWriters_->end())
    {
        return p->second.get();
    }
    else
    {
        IceObjC::ValueWrapperPtr writer = new IceObjC::ValueWrapper(object);
        objectWriters_->insert(std::make_pair(object, writer));
        return writer.get();
    }
}

-(Ice::SlicedData*) writeSlicedData:(id<ICESlicedData>)sd
{
    NSAssert([sd isKindOfClass:[ICESlicedData class]], @"invalid sliced data object");
    Ice::SlicedData* origSlicedData = [((ICESlicedData*)sd) slicedData];
    Ice::SliceInfoSeq slices;
    for(Ice::SliceInfoSeq::const_iterator p = origSlicedData->slices.begin(); p != origSlicedData->slices.end(); ++p)
    {
        Ice::SliceInfoPtr info = new Ice::SliceInfo;
        info->typeId = (*p)->typeId;
        info->compactId = (*p)->compactId;
        info->bytes = (*p)->bytes;
        info->hasOptionalMembers = (*p)->hasOptionalMembers;
        info->isLastSlice = (*p)->isLastSlice;

        for(std::vector<Ice::ValuePtr>::const_iterator q = (*p)->instances.begin(); q != (*p)->instances.end(); ++q)
        {
            if(*q)
            {
                assert(IceObjC::ValueWrapperPtr::dynamicCast(*q));
                info->instances.push_back([self addObject:IceObjC::ValueWrapperPtr::dynamicCast(*q)->getValue()]);
            }
            else
            {
                info->instances.push_back(0);
            }
        }
        slices.push_back(info);
    }
    return new Ice::SlicedData(slices);
}

@end

@implementation ICEStreamHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    NSAssert(NO, @"requires override");
    return nil;
}
+(id) read:(id<ICEInputStream>)stream
{
    return [[self readRetained:stream] autorelease];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    NSAssert(NO, @"requires override");
}
+(id) readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    NSAssert(NO, @"requires override");
    return nil;
}
+(id) readOptional:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    return [[self readOptionalRetained:stream tag:tag] autorelease];
    return nil;
}
+(void) writeOptional:(id)obj stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    NSAssert(NO, @"requires override");
}
+(ICEInt) minWireSize
{
    NSAssert(NO, @"requires override");
    return 0;
}
@end

@implementation ICEBoolHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [[NSNumber alloc] initWithBool:[stream readBool]];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    if(obj == nil)
    {
        @throw [ICEMarshalException marshalException:__FILE__ line:__LINE__ reason:@"illegal NSNull value"];
    }
    [stream writeBool:[obj boolValue]];
}
+(id)readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatF1])
    {
        return [[NSNumber alloc] initWithBool:[stream readBool]];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    BOOL value;
    if([ICEOptionalGetter getBool:v value:&value] && [stream writeOptional:tag format:ICEOptionalFormatF1])
    {
        [stream writeBool:value];
    }
}
+(ICEInt) minWireSize
{
    return 1;
}
@end

@implementation ICEByteHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [[NSNumber alloc] initWithUnsignedChar:[stream readByte]];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    if(obj == nil)
    {
        @throw [ICEMarshalException marshalException:__FILE__ line:__LINE__ reason:@"illegal NSNull value"];
    }
    [stream writeByte:[obj unsignedCharValue]];
}
+(id)readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatF1])
    {
        return [[NSNumber alloc] initWithUnsignedChar:[stream readByte]];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    ICEByte value;
    if([ICEOptionalGetter getByte:v value:&value] && [stream writeOptional:tag format:ICEOptionalFormatF1])
    {
        [stream writeByte:value];
    }
}
+(ICEInt) minWireSize
{
    return 1;
}
@end

@implementation ICEShortHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [[NSNumber alloc] initWithShort:[stream readShort]];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    if(obj == nil)
    {
        @throw [ICEMarshalException marshalException:__FILE__ line:__LINE__ reason:@"illegal NSNull value"];
    }
    [stream writeShort:[obj shortValue]];
}
+(id)readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatF2])
    {
        return [[NSNumber alloc] initWithShort:[stream readShort]];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    ICEShort value;
    if([ICEOptionalGetter getShort:v value:&value] && [stream writeOptional:tag format:ICEOptionalFormatF2])
    {
        [stream writeShort:value];
    }
}
+(ICEInt) minWireSize
{
    return 2;
}
@end

@implementation ICEIntHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [[NSNumber alloc] initWithInt:[stream readInt]];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    if(obj == nil)
    {
        @throw [ICEMarshalException marshalException:__FILE__ line:__LINE__ reason:@"illegal NSNull value"];
    }
    [stream writeInt:[obj intValue]];
}
+(id)readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatF4])
    {
        return [[NSNumber alloc] initWithInt:[stream readInt]];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    ICEInt value;
    if([ICEOptionalGetter getInt:v value:&value] && [stream writeOptional:tag format:ICEOptionalFormatF4])
    {
        [stream writeInt:value];
    }
}
+(ICEInt) minWireSize
{
    return 4;
}
@end

@implementation ICELongHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [[NSNumber alloc] initWithLong:[stream readLong]];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    if(obj == nil)
    {
        @throw [ICEMarshalException marshalException:__FILE__ line:__LINE__ reason:@"illegal NSNull value"];
    }
    [stream writeLong:[obj longValue]];
}
+(id)readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatF8])
    {
        return [[NSNumber alloc] initWithLong:[stream readLong]];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    ICELong value;
    if([ICEOptionalGetter getLong:v value:&value] && [stream writeOptional:tag format:ICEOptionalFormatF8])
    {
        [stream writeLong:value];
    }
}
+(ICEInt) minWireSize
{
    return 8;
}
@end

@implementation ICEFloatHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [[NSNumber alloc] initWithFloat:[stream readFloat]];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    if(obj == nil)
    {
        @throw [ICEMarshalException marshalException:__FILE__ line:__LINE__ reason:@"illegal NSNull value"];
    }
    [stream writeFloat:[obj floatValue]];
}
+(id)readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatF4])
    {
        return [[NSNumber alloc] initWithFloat:[stream readFloat]];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    ICEFloat value;
    if([ICEOptionalGetter getFloat:v value:&value] && [stream writeOptional:tag format:ICEOptionalFormatF4])
    {
        [stream writeFloat:value];
    }
}
+(ICEInt) minWireSize
{
    return 4;
}
@end

@implementation ICEDoubleHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [[NSNumber alloc] initWithDouble:[stream readDouble]];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    if(obj == nil)
    {
        @throw [ICEMarshalException marshalException:__FILE__ line:__LINE__ reason:@"illegal NSNull value"];
    }
    [stream writeDouble:[obj doubleValue]];
}
+(id)readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatF8])
    {
        return [[NSNumber alloc] initWithDouble:[stream readDouble]];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    ICEDouble value;
    if([ICEOptionalGetter getDouble:v value:&value] && [stream writeOptional:tag format:ICEOptionalFormatF8])
    {
        [stream writeDouble:value];
    }
}
+(ICEInt) minWireSize
{
    return 8;
}
@end

@implementation ICEStringHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [stream newString];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    if(obj == [NSNull null])
    {
        obj = nil;
    }
    [stream writeString:obj];
}
+(id)readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatVSize])
    {
        return [stream newString];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    NSString* value;
    if([ICEOptionalGetter get:v value:&value type:[NSString class]] &&
       [stream writeOptional:tag format:ICEOptionalFormatVSize])
    {
        [stream writeString:v];
    }
}
+(ICEInt) minWireSize
{
    return 1;
}
@end

@implementation ICEEnumHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [[NSNumber alloc] initWithInt:[stream readEnumerator:[self getMinValue] max:[self getMaxValue]]];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    if(obj == nil)
    {
        @throw [ICEMarshalException marshalException:__FILE__ line:__LINE__ reason:@"illegal NSNull value"];
    }
    [stream writeEnumerator:[obj intValue] min:[self getMinValue] max:[self getMaxValue]];
}
+(id)readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatSize])
    {
        return [[NSNumber alloc] initWithInt:[stream readEnumerator:[self getMinValue] max:[self getMaxValue]]];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    ICEInt value;
    if([ICEOptionalGetter getInt:v value:&value] && [stream writeOptional:tag format:ICEOptionalFormatSize])
    {
        [stream writeEnumerator:value min:[self getMinValue] max:[self getMaxValue]];
    }
}
+(ICEInt) getMinValue
{
    NSAssert(NO, @"ICEEnumHelper getMinValue requires override");
    return 0;
}
+(ICEInt) getMaxValue
{
    NSAssert(NO, @"ICEEnumHelper getMaxValue requires override");
    return 0;
}
+(ICEInt) minWireSize
{
    return 1;
}
@end

@implementation ICEObjectHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    //
    // This will only work with the 1.1 encoding. With the 1.0 encoding
    // objects are read when readPendingObjects is called.
    //
    ICEObject* obj;
    [stream newValue:&obj];
    return (id)obj;
}
+(void)readRetained:(ICEObject**)v stream:(id<ICEInputStream>)stream
{
    [stream newValue:v];
}
+(void)read:(ICEObject**)v stream:(id<ICEInputStream>)stream
{
    [stream readValue:v];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    [stream writeValue:obj];
}
+(id)readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatClass])
    {
        return [self readRetained:stream];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    ICEObject* value;
    if([ICEOptionalGetter get:v value:&value type:[ICEObject class]] &&
       [stream writeOptional:tag format:ICEOptionalFormatClass])
    {
        [self write:value stream:stream];
    }
}
+(void)readOptionalRetained:(id *)v stream:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatClass])
    {
        [self readRetained:(ICEObject**)v stream:stream];
    }
    else
    {
        *v = ICENone;
    }
}
+(void)readOptional:(id *)v stream:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatClass])
    {
        [self read:(ICEObject**)v stream:stream];
    }
    else
    {
        *v = ICENone;
    }
}
+(ICEInt) minWireSize
{
    return 1;
}
@end

@implementation ICEProxyHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [stream newProxy:[ICEObjectPrx class]];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    [stream writeProxy:obj];
}
+(id)readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatFSize])
    {
        return [ICEVarLengthOptionalHelper readRetained:stream helper:self];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    ICEObjectPrx* value;
    if([ICEOptionalGetter get:v value:&value type:[ICEObjectPrx class]] &&
       [stream writeOptional:tag format:ICEOptionalFormatFSize])
    {
        [ICEVarLengthOptionalHelper write:v stream:stream helper:self];
    }
}
+(ICEInt) minWireSize
{
    return 2;
}
@end

@implementation ICEStructHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [self readRetained:stream value:nil];
}
+(id) readRetained:(id<ICEInputStream>)stream value:(id)p
{
    if(p == nil)
    {
        p = [[self getType] new];
    }
    else
    {
        [p retain];
    }

    @try
    {
        [p iceRead:stream];
    }
    @catch(NSException *ex)
    {
        [p release];
        @throw ex;
    }
    return p;
}
+(id) read:(id<ICEInputStream>)stream value:(id)p
{
    return [[self readRetained:stream value:p] autorelease];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    if(obj == nil)
    {
        obj = [[self getType] new];
        @try
        {
            [obj iceWrite:stream];
        }
        @finally
        {
            [obj release];
        }
    }
    else
    {
        [obj iceWrite:stream];
    }
}
+(id) readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    Class helper = [self getOptionalHelper];
    if([stream readOptional:tag format:[helper optionalFormat]])
    {
        return [helper readRetained:stream helper:self];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)s tag:(ICEInt)tag
{
    Class helper = [self getOptionalHelper];
    NSObject* a;
    if([ICEOptionalGetter get:v value:&a type:[self getType]] && [s writeOptional:tag format:[helper optionalFormat]])
    {
        [helper write:a stream:s helper:self];
    }
}
+(Class) getType
{
    NSAssert(NO, @"getType requires override");
    return nil;
}
+(Class) getOptionalHelper
{
    NSAssert(NO, @"getOptionalHelper requires override");
    return nil;
}
+(ICEInt) minWireSize
{
    NSAssert(NO, @"minWireSize requires override");
    return 0;
}
@end

@implementation ICEArraySequenceHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [stream newSequence:[self getElementHelper]];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    [stream writeSequence:obj helper:[self getElementHelper]];
}
+(id) readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    Class helper = [self getOptionalHelper];
    if([stream readOptional:tag format:[helper optionalFormat]])
    {
        return [helper readRetained:stream helper:self];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)s tag:(ICEInt)tag
{
    Class helper = [self getOptionalHelper];
    NSArray* a;
    if([ICEOptionalGetter get:v value:&a type:[NSArray class]] && [s writeOptional:tag format:[helper optionalFormat]])
    {
        [helper write:a stream:s helper:self];
    }
}
+(Class) getElementHelper
{
    NSAssert(NO, @"getElementHelper requires override");
    return nil;
}
+(Class) getOptionalHelper
{
    NSAssert(NO, @"getOptionalHelper requires override");
    return nil;
}
+(ICEInt) minWireSize
{
    return 1;
}
+(ICEInt) count:(id)obj
{
    return [obj count];
}
@end

@implementation ICEDataSequenceHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    NSAssert(NO, @"readRetained requires override");
    return nil;
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    NSAssert(NO, @"write requires override");
}
+(id) readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatVSize])
    {
        return [ICEFixedSequenceOptionalHelper readRetained:stream helper:self];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    NSData* a;
    if([ICEOptionalGetter get:v value:&a type:[NSData class]] &&
       [stream writeOptional:tag format:ICEOptionalFormatVSize])
    {
        [ICEFixedSequenceOptionalHelper write:a stream:stream helper:self];
    }
}
+(ICEInt) minWireSize
{
    return 1;
}
+(ICEInt) count:(id)obj
{
    return [obj length] / [[self getElementHelper] minWireSize];
}
+(Class) getElementHelper
{
    return [ICEShortHelper class];
}
@end

@implementation ICEBoolSequenceHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [stream newBoolSeq];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    [stream writeBoolSeq:obj];
}
+(id) readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatVSize])
    {
        return [stream newBoolSeq];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    NSData* a;
    if([ICEOptionalGetter get:v value:&a type:[NSData class]] &&
       [stream writeOptional:tag format:ICEOptionalFormatVSize])
    {
        [stream writeBoolSeq:a];
    }
}
+(Class) getElementHelper
{
    return [ICEBoolHelper class];
}
@end

@implementation ICEByteSequenceHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [stream newByteSeq];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    [stream writeByteSeq:obj];
}
+(id) readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    if([stream readOptional:tag format:ICEOptionalFormatVSize])
    {
        return [stream newByteSeq];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)stream tag:(ICEInt)tag
{
    NSData* a;
    if([ICEOptionalGetter get:v value:&a type:[NSData class]] &&
       [stream writeOptional:tag format:ICEOptionalFormatVSize])
    {
        [stream writeByteSeq:a];
    }
}
+(Class) getElementHelper
{
    return [ICEIntHelper class];
}
@end

@implementation ICEShortSequenceHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [stream newShortSeq];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    [stream writeShortSeq:obj];
}
+(Class) getElementHelper
{
    return [ICEShortHelper class];
}
@end

@implementation ICEIntSequenceHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [stream newIntSeq];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    [stream writeIntSeq:obj];
}
+(Class) getElementHelper
{
    return [ICEIntHelper class];
}
@end

@implementation ICELongSequenceHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [stream newLongSeq];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    [stream writeLongSeq:obj];
}
+(Class) getElementHelper
{
    return [ICELongHelper class];
}
@end

@implementation ICEFloatSequenceHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [stream newFloatSeq];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    [stream writeFloatSeq:obj];
}
+(Class) getElementHelper
{
    return [ICEFloatHelper class];
}
@end

@implementation ICEDoubleSequenceHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [stream newDoubleSeq];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    [stream writeDoubleSeq:obj];
}
+(Class) getElementHelper
{
    return [ICEDoubleHelper class];
}
@end

@implementation ICEEnumSequenceHelper
+(ICEInt) count:(id)obj
{
    return [obj length] / ENUM_SIZE;
}
+(Class) getElementHelper
{
    return [ICEEnumHelper class];
}
@end

@implementation ICEStringSequenceHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [stream newStringSeq];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    [stream writeStringSeq:obj];
}
+(Class) getElementHelper
{
    return [ICEStringHelper class];
}
+(Class) getOptionalHelper
{
    return [ICEVarLengthOptionalHelper class];
}
@end

@implementation ICEObjectSequenceHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [stream newValueSeq:[ICEObject class]];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    [stream writeValueSeq:obj];
}
+(Class) getElementHelper
{
    return [ICEObjectHelper class];
}
+(Class) getOptionalHelper
{
    return [ICEVarLengthOptionalHelper class];
}
@end

@implementation ICEProxySequenceHelper
+(Class) getElementHelper
{
    return [ICEProxyHelper class];
}
+(Class) getOptionalHelper
{
    return [ICEVarLengthOptionalHelper class];
}
@end

@implementation ICEDictionaryHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    return [stream newDictionary:[self getKeyValueHelper]];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    [stream writeDictionary:obj helper:[self getKeyValueHelper]];
}
+(id) readOptionalRetained:(id<ICEInputStream>)stream tag:(ICEInt)tag
{
    Class helper = [self getOptionalHelper];
    if([stream readOptional:tag format:[helper optionalFormat]])
    {
        return [helper readRetained:stream helper:self];
    }
    return ICENone;
}
+(void) writeOptional:(id)v stream:(id<ICEOutputStream>)s tag:(ICEInt)tag
{
    Class helper = [self getOptionalHelper];
    NSDictionary* a;
    if([ICEOptionalGetter get:v value:&a type:[NSDictionary class]] &&
       [s writeOptional:tag format:[helper optionalFormat]])
    {
        [helper write:a stream:s helper:self];
    }
}
+(Class) getOptionalHelper
{
    NSAssert(NO, @"getOptionalHelper requires override");
    return nil;
}
+(ICEKeyValueTypeHelper) getKeyValueHelper
{
    NSAssert(NO, @"ICEDictionaryHelper getKeyValueHelper requires override");
    ICEKeyValueTypeHelper dummy;
    return dummy; // Keep compiler quiet
}
+(ICEInt) minWireSize
{
    return 1;
}
+(ICEInt) count:(id)obj
{
    return [obj count];
}
@end

@implementation ICEObjectDictionaryHelper
+(id) readRetained:(id<ICEInputStream>)stream
{
    NSAssert(NO, @"ICEObjectDictionaryHelper readRetained requires override");
    return nil;
}

+(void) write:(id)obj stream:(id<ICEOutputStream>)stream
{
    NSAssert(NO, @"ICEObjectDictionaryHelper write requires override");
}

+(Class) getOptionalHelper
{
    return [ICEVarLengthOptionalHelper class];
}
@end

@implementation ICEVarLengthOptionalHelper
+(id) readRetained:(id<ICEInputStream>)stream helper:(Class)helper
{
    [stream skip:4];
    return [helper readRetained:stream];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream helper:(Class)helper
{
    Ice::OutputStream* os = [(ICEOutputStream*)stream os];
    os->write(static_cast<Ice::Int>(0));
    Ice::OutputStream::size_type p = os->pos();
    [helper write:obj stream:stream];
    os->rewrite(static_cast<Ice::Int>(os->pos() - p), p - 4);
}
+(ICEOptionalFormat) optionalFormat
{
    return ICEOptionalFormatFSize;
}
@end

@implementation ICEFixedLengthOptionalHelper
+(id) readRetained:(id<ICEInputStream>)stream helper:(Class)helper
{
    [stream skipSize];
    return [helper readRetained:stream];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream helper:(Class)helper
{
    [stream writeSize:[helper minWireSize]];
    [helper write:obj stream:stream];
}
+(ICEOptionalFormat) optionalFormat
{
    return ICEOptionalFormatVSize;
}
@end

@implementation ICEFixedSequenceOptionalHelper
+(id) readRetained:(id<ICEInputStream>)stream helper:(Class)helper
{
    [stream skipSize];
    return [helper readRetained:stream];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream helper:(Class)helper
{
    //
    // The container size is the number of elements * the size of
    // an element and the size-encoded number of elements (1 or
    // 5 depending on the number of elements).
    //
    ICEInt n = [helper count:obj];
    ICEInt sz = [[helper getElementHelper] minWireSize];
    [stream writeSize:sz * n + (n < 255 ? 1 : 5)];
    [helper write:obj stream:stream];
}
+(ICEOptionalFormat) optionalFormat
{
    return ICEOptionalFormatVSize;
}
@end

@implementation ICEFixedSize1SequenceOptionalHelper
+(id) readRetained:(id<ICEInputStream>)stream helper:(Class)helper
{
    return [helper readRetained:stream];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream helper:(Class)helper
{
    [helper write:obj stream:stream];
}
+(ICEOptionalFormat) optionalFormat
{
    return ICEOptionalFormatVSize;
}
@end

@implementation ICEFixedDictionaryOptionalHelper
+(id) readRetained:(id<ICEInputStream>)stream helper:(Class)helper
{
    [stream skipSize];
    return [helper readRetained:stream];
}
+(void) write:(id)obj stream:(id<ICEOutputStream>)stream helper:(Class)helper
{
    //
    // The container size is the number of elements * the size of
    // an element and the size-encoded number of elements (1 or
    // 5 depending on the number of elements).
    //
    ICEInt n = [helper count:obj];
    ICEKeyValueTypeHelper h = [helper getKeyValueHelper];
    ICEInt sz = [h.key minWireSize] + [h.value minWireSize];
    [stream writeSize:sz * n + (n < 255 ? 1 : 5)];
    [helper write:obj stream:stream];
}
+(ICEOptionalFormat) optionalFormat
{
    return ICEOptionalFormatVSize;
}
@end

@implementation ICEOptionalGetter
+(BOOL) get:(id)value type:(Class)cl
{
    if(value == ICENone)
    {
        return NO;
    }
    else
    {
        if(value != nil && ![value isKindOfClass:cl])
        {
            @throw [NSException exceptionWithName:NSInvalidArgumentException reason:@"unexpected type" userInfo:cl];
        }
        return YES;
    }
}
+(BOOL) get:(id)value value:(id*)v type:(Class)cl
{
    if([self get:value type:cl])
    {
        *v = value;
        return YES;
    }
    return NO;
}
+(BOOL) getRetained:(id)value value:(id*)v type:(Class)cl
{
    BOOL r = [self get:value value:v type:cl];
    [*v retain];
    return r;
}
+(BOOL) getByte:(id)value value:(ICEByte*)v
{
    if([self get:value type:[NSNumber class]])
    {
        *v = [value unsignedCharValue];
        return YES;
    }
    return NO;
}
+(BOOL) getBool:(id)value value:(BOOL*)v
{
    if([self get:value type:[NSNumber class]])
    {
        *v = [value boolValue];
        return YES;
    }
    return NO;
}
+(BOOL) getShort:(id)value value:(ICEShort*)v
{
    if([self get:value type:[NSNumber class]])
    {
        *v = [value shortValue];
        return YES;
    }
    return NO;
}
+(BOOL) getInt:(id)value value:(ICEInt*)v
{
    if([self get:value type:[NSNumber class]])
    {
        *v = [value intValue];
        return YES;
    }
    return NO;
}
+(BOOL) getLong:(id)value value:(ICELong*)v
{
    if([self get:value type:[NSNumber class]])
    {
        *v = [value longValue];
        return YES;
    }
    return NO;
}
+(BOOL) getFloat:(id)value value:(ICEFloat*)v
{
    if([self get:value type:[NSNumber class]])
    {
        *v = [value floatValue];
        return YES;
    }
    return NO;
}
+(BOOL) getDouble:(id)value value:(ICEDouble*)v
{
    if([self get:value type:[NSNumber class]])
    {
        *v = [value doubleValue];
        return YES;
    }
    return NO;
}
@end
