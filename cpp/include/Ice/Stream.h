// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STREAM_H
#define ICE_STREAM_H

#include <Ice/StreamF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Object.h>
#include <Ice/Exception.h>
#include <Ice/Proxy.h>
#include <Ice/SlicedDataF.h>
#include <IceUtil/Shared.h>

namespace Ice
{

enum StreamTraitType
{
    StreamTraitTypeBuiltin,
    StreamTraitTypeStruct,
    StreamTraitTypeStructClass,     // struct with cpp:class metadata
    StreamTraitTypeByteEnum,        // Enums with up to 127 enumerators
    StreamTraitTypeShortEnum,       // Enums with up to 32767 enumerators
    StreamTraitTypeIntEnum,         // Enums with more than 32767 enumerators
    StreamTraitTypeSequence,
#ifndef __BCPLUSPLUS__ // COMPILERFIX: See StreamTrait<vector<bool>> comment below
    StreamTraitTypeSequenceBool,
#endif
    StreamTraitTypeDictionary,
    StreamTraitTypeUserException,
    StreamTraitTypeUnknown
};

//
// Base trait template. This doesn't actually do anything -- we just
// use it as a template that we can specialize.
//
template<typename T>
struct StreamTrait
{
    static const StreamTraitType type = StreamTraitTypeUnknown;
    static const int minWireSize = 0;
};

//
// StreamTrait specialization for std::vector
//
template<typename T>
struct StreamTrait< ::std::vector<T> >
{
    static const StreamTraitType type = StreamTraitTypeSequence;
    static const int minWireSize = 1;
};

//
// StreamTrait specialization for std::vector<bool>. Sequences of bool
// are handled specifically because C++ optimizations for vector<bool>
// prevent us from reading vector of bools the same way as other
// sequences (see StreamReader<StreamTraitTypeSequenceBool>::read
// implementation below)
//
// COMPILERFIX: BCC2010 doesn't allow use of full specialization over
// partial specialization.
//
#ifndef __BCPLUSPLUS__
template<>
struct StreamTrait< ::std::vector<bool> >
{
    static const StreamTraitType type = StreamTraitTypeSequenceBool;
    static const int minWireSize = 1;
};
#endif

template<>
struct StreamTrait<UserException>
{
    static const StreamTraitType type = StreamTraitTypeUserException;
};

//
// StreamTrait specialization for std::map.
//
template<typename K, typename V>
struct StreamTrait< ::std::map<K, V> >
{
    static const StreamTraitType type = StreamTraitTypeDictionary;
    static const int minWireSize = 1;
};

//
// StreamTrait specialization for builtins (these are needed for sequence
// marshalling to figure out the minWireSize of each built-in).
//
template<>
struct StreamTrait< bool>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 1;
};

template<>
struct StreamTrait< Byte>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 1;
};

template<>
struct StreamTrait< Short>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 2;
};

template<>
struct StreamTrait< Int>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 4;
};

template<>
struct StreamTrait< Long>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 8;
};

template<>
struct StreamTrait< Float>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 4;
};

template<>
struct StreamTrait< Double>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 8;
};

template<>
struct StreamTrait< ::std::string>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 1;
};

template<>
struct StreamTrait< ::std::wstring>
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 1;
};

template<typename T>
struct StreamTrait< ::IceInternal::ProxyHandle<T> >
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 2;
};

template<typename T>
struct StreamTrait< ::IceInternal::Handle<T> >
{
    static const StreamTraitType type = StreamTraitTypeBuiltin;
    static const int minWireSize = 4;
};

//
// This is the non-specialized version of the writer. For each kind of
// user-defined type (enum, struct, etc), we specialize this template
// to call the correct member function that writes an instance of that
// type to the stream.
//
template<StreamTraitType st>
struct StreamWriter
{
    template<typename T>
    static void write(const OutputStreamPtr&, const T&)
    {
        // This asserts because we end up writing something for which
        // we never defined a trait.
        assert(false);
    }
};

//
// This is the non-specialized version of the reader. For each kind of
// user-defined type (enum, struct, etc), we specialize this template
// to call the correct member function that reads an instance of that
// type to the stream.
//
template<StreamTraitType st>
struct StreamReader
{
    template<typename T>
    static void read(const InputStreamPtr&, T&)
    {
        // This asserts because we end up reading something for which
        // we never define a trait.
        assert(false);
    }
};

class ICE_API ReadObjectCallback : public ::IceUtil::Shared
{
public:

    virtual void invoke(const ObjectPtr&) = 0;
};
typedef IceUtil::Handle<ReadObjectCallback> ReadObjectCallbackPtr;

template<typename T>
class ReadObjectCallbackI : public ReadObjectCallback
{

public:

    ReadObjectCallbackI(::IceInternal::Handle<T>& v) :
        _v(v)
    {
    }

    virtual void invoke(const ObjectPtr& p)
    {
        _v = ::IceInternal::Handle<T>::dynamicCast(p);
        if(p && !_v)
        {
            IceInternal::Ex::throwUOE(T::ice_staticId(), p->ice_id());
        }
    }

private:

    ::IceInternal::Handle<T>& _v;
};

class ICE_API UserExceptionReader : public UserException
{
public:

    UserExceptionReader(const CommunicatorPtr&);
    ~UserExceptionReader() throw();

    virtual void read(const InputStreamPtr&) const = 0;
    virtual bool usesClasses() const = 0;
    virtual void usesClasses(bool) = 0;

    virtual ::std::string ice_name() const = 0;
    virtual Exception* ice_clone() const = 0;
    virtual void ice_throw() const = 0;

    virtual void __write(IceInternal::BasicStream*) const;
    virtual void __read(IceInternal::BasicStream*);

    virtual bool __usesClasses() const;
    virtual void __usesClasses(bool);

protected:

    const CommunicatorPtr _communicator;
};

class ICE_API UserExceptionReaderFactory : public IceUtil::Shared
{
public:

    virtual void createAndThrow(const std::string&) const = 0;
};
typedef ::IceUtil::Handle<UserExceptionReaderFactory> UserExceptionReaderFactoryPtr;

class ICE_API InputStream : public ::IceUtil::Shared
{
public:

    virtual CommunicatorPtr communicator() const = 0;

    virtual void sliceObjects(bool) = 0;

    //
    // COMPILERFIX: BCC2010 doesn't allow use of full specialization over
    // partial specialization.
    //
#ifdef __BCPLUSPLUS__
    void
    read(::std::_Vb_reference<unsigned int, int> v)
    {
        v = readBool();
    }
#endif

    ICE_DEPRECATED_API virtual bool readBool() = 0;
    ICE_DEPRECATED_API virtual Byte readByte() = 0;
    ICE_DEPRECATED_API virtual Short readShort() = 0;
    ICE_DEPRECATED_API virtual Int readInt() = 0;
    ICE_DEPRECATED_API virtual Long readLong() = 0;
    ICE_DEPRECATED_API virtual Float readFloat() = 0;
    ICE_DEPRECATED_API virtual Double readDouble() = 0;
    ICE_DEPRECATED_API virtual ::std::string readString(bool = true) = 0;
    ICE_DEPRECATED_API virtual ::std::wstring readWstring() = 0;

    ICE_DEPRECATED_API virtual ::std::vector<bool> readBoolSeq() = 0;
    ICE_DEPRECATED_API virtual ::std::vector<Byte> readByteSeq() = 0;
    ICE_DEPRECATED_API virtual ::std::vector<Short> readShortSeq() = 0;
    ICE_DEPRECATED_API virtual ::std::vector<Int> readIntSeq() = 0;
    ICE_DEPRECATED_API virtual ::std::vector<Long> readLongSeq() = 0;
    ICE_DEPRECATED_API virtual ::std::vector<Float> readFloatSeq() = 0;
    ICE_DEPRECATED_API virtual ::std::vector<Double> readDoubleSeq() = 0;
    ICE_DEPRECATED_API virtual ::std::vector< ::std::string> readStringSeq(bool = true) = 0;
    ICE_DEPRECATED_API virtual ::std::vector< ::std::wstring> readWstringSeq() = 0;

    ICE_DEPRECATED_API virtual bool* readBoolSeq(::std::pair<const bool*, const bool*>&) = 0;
    ICE_DEPRECATED_API virtual void readByteSeq(::std::pair<const Byte*, const Byte*>&) = 0;
    ICE_DEPRECATED_API virtual Short* readShortSeq(::std::pair<const Short*, const Short*>&) = 0;
    ICE_DEPRECATED_API virtual Int* readIntSeq(::std::pair<const Int*, const Int*>&) = 0;
    ICE_DEPRECATED_API virtual Long* readLongSeq(::std::pair<const Long*, const Long*>&) = 0;
    ICE_DEPRECATED_API virtual Float* readFloatSeq(::std::pair<const Float*, const Float*>&) = 0;
    ICE_DEPRECATED_API virtual Double* readDoubleSeq(::std::pair<const Double*, const Double*>&) = 0;

    virtual Int readSize() = 0;
    virtual Int readAndCheckSeqSize(int) = 0;

    virtual ObjectPrx readProxy() = 0;
    virtual void readObject(const ReadObjectCallbackPtr&) = 0;

    virtual void throwException() = 0;
    virtual void throwException(const UserExceptionReaderFactoryPtr&) = 0;

    virtual void startObject() = 0;
    virtual SlicedDataPtr endObject(bool) = 0;

    virtual void startException() = 0;
    virtual SlicedDataPtr endException(bool) = 0;

    virtual std::string startSlice() = 0;
    virtual void endSlice() = 0;
    virtual void skipSlice() = 0;

    virtual Ice::EncodingVersion startEncapsulation() = 0;
    virtual void endEncapsulation() = 0;
    virtual Ice::EncodingVersion skipEncapsulation() = 0;

    virtual void readPendingObjects() = 0;

    virtual void rewind() = 0;

    virtual void read(bool&) = 0;
    virtual void read(Byte&) = 0;
    virtual void read(Short&) = 0;
    virtual void read(Int&) = 0;
    virtual void read(Long&) = 0;
    virtual void read(Float&) = 0;
    virtual void read(Double&) = 0;
    virtual void read(::std::string&, bool = true) = 0;
    virtual void read(::std::vector< ::std::string>&, bool) = 0; // Overload required for additional bool argument.
    virtual void read(::std::wstring&) = 0;

    virtual void read(::std::pair<const bool*, const bool*>&, ::IceUtil::ScopedArray<bool>&) = 0;
    virtual void read(::std::pair<const Byte*, const Byte*>&) = 0;
    virtual void read(::std::pair<const Short*, const Short*>&, ::IceUtil::ScopedArray<Short>&) = 0;
    virtual void read(::std::pair<const Int*, const Int*>&, ::IceUtil::ScopedArray<Int>&) = 0;
    virtual void read(::std::pair<const Long*, const Long*>&, ::IceUtil::ScopedArray<Long>&) = 0;
    virtual void read(::std::pair<const Float*, const Float*>&, ::IceUtil::ScopedArray<Float>&) = 0;
    virtual void read(::std::pair<const Double*, const Double*>&, ::IceUtil::ScopedArray<Double>&) = 0;

    template<typename T> inline void
    read(::IceInternal::ProxyHandle<T>& v)
    {
        ObjectPrx proxy = readProxy();
        if(!proxy)
        {
            v = 0;
        }
        else
        {
            v = new T;
            v->__copyFrom(proxy);
        }
    }

    template<typename T> inline void
    read(::IceInternal::Handle<T>& v)
    {
        ReadObjectCallbackPtr cb = new ReadObjectCallbackI<T>(v);
        readObject(cb);
    }

    template<typename T> inline void
    read(T& v)
    {
        StreamReader< StreamTrait<T>::type>::read(this, v);
    }

    virtual void closure(void*) = 0;
    virtual void* closure() const = 0;
};

class ICE_API OutputStream : public ::IceUtil::Shared
{
public:

    virtual CommunicatorPtr communicator() const = 0;

    ICE_DEPRECATED_API virtual void writeBool(bool) = 0;
    ICE_DEPRECATED_API virtual void writeByte(Byte) = 0;
    ICE_DEPRECATED_API virtual void writeShort(Short) = 0;
    ICE_DEPRECATED_API virtual void writeInt(Int) = 0;
    ICE_DEPRECATED_API virtual void writeLong(Long) = 0;
    ICE_DEPRECATED_API virtual void writeFloat(Float) = 0;
    ICE_DEPRECATED_API virtual void writeDouble(Double) = 0;
    ICE_DEPRECATED_API virtual void writeString(const ::std::string&, bool = true) = 0;
    ICE_DEPRECATED_API virtual void writeWstring(const ::std::wstring&)= 0;

    ICE_DEPRECATED_API virtual void writeBoolSeq(const ::std::vector<bool>&) = 0;
    ICE_DEPRECATED_API virtual void writeByteSeq(const ::std::vector<Byte>&) = 0;
    ICE_DEPRECATED_API virtual void writeShortSeq(const ::std::vector<Short>&) = 0;
    ICE_DEPRECATED_API virtual void writeIntSeq(const ::std::vector<Int>&) = 0;
    ICE_DEPRECATED_API virtual void writeLongSeq(const ::std::vector<Long>&) = 0;
    ICE_DEPRECATED_API virtual void writeFloatSeq(const ::std::vector<Float>&) = 0;
    ICE_DEPRECATED_API virtual void writeDoubleSeq(const ::std::vector<Double>&) = 0;
    ICE_DEPRECATED_API virtual void writeStringSeq(const ::std::vector< ::std::string>&, bool = true) = 0;
    ICE_DEPRECATED_API virtual void writeWstringSeq(const ::std::vector< ::std::wstring>&) = 0;

    ICE_DEPRECATED_API virtual void writeBoolSeq(const bool*, const bool*) = 0;
    ICE_DEPRECATED_API virtual void writeByteSeq(const Byte*, const Byte*) = 0;
    ICE_DEPRECATED_API virtual void writeShortSeq(const Short*, const Short*) = 0;
    ICE_DEPRECATED_API virtual void writeIntSeq(const Int*, const Int*) = 0;
    ICE_DEPRECATED_API virtual void writeLongSeq(const Long*, const Long*) = 0;
    ICE_DEPRECATED_API virtual void writeFloatSeq(const Float*, const Float*) = 0;
    ICE_DEPRECATED_API virtual void writeDoubleSeq(const Double*, const Double*) = 0;

    virtual void writeSize(Int) = 0;
    virtual void writeProxy(const ObjectPrx&) = 0;
    virtual void writeObject(const ObjectPtr&) = 0;
    virtual void writeException(const UserException&) = 0;

    virtual void format(FormatType) = 0;

    virtual void startObject(const SlicedDataPtr&) = 0;
    virtual void endObject() = 0;

    virtual void startException(const SlicedDataPtr&) = 0;
    virtual void endException() = 0;

    virtual void startSlice(const ::std::string&, bool) = 0;
    virtual void endSlice() = 0;

    virtual void startEncapsulation(const Ice::EncodingVersion&) = 0;
    virtual void startEncapsulation() = 0;
    virtual void endEncapsulation() = 0;

    virtual void writePendingObjects() = 0;

    virtual void finished(::std::vector<Byte>&) = 0;

    virtual void reset(bool) = 0;

    virtual void write(bool) = 0;
    virtual void write(Byte) = 0;
    virtual void write(Short) = 0;
    virtual void write(Int) = 0;
    virtual void write(Long) = 0;
    virtual void write(Float) = 0;
    virtual void write(Double) = 0;
    virtual void write(const ::std::string&, bool = true) = 0;
    virtual void write(const ::std::vector< ::std::string>&, bool) = 0; // Overload required for bool argument.
    virtual void write(const char*, bool = true) = 0;
    virtual void write(const ::std::wstring&) = 0;

    virtual void write(const bool*, const bool*) = 0;
    virtual void write(const Byte*, const Byte*) = 0;
    virtual void write(const Short*, const Short*) = 0;
    virtual void write(const Int*, const Int*) = 0;
    virtual void write(const Long*, const Long*) = 0;
    virtual void write(const Float*, const Float*) = 0;
    virtual void write(const Double*, const Double*) = 0;

    template<typename T> inline void
    write(const ::IceInternal::ProxyHandle<T>& v)
    {
        writeProxy(v);
    }

    template<typename T> inline void
    write(const ::IceInternal::Handle<T>& v)
    {
        writeObject(v);
    }

    template<typename T> inline void
    write(const T& v)
    {
        StreamWriter<StreamTrait<T>::type>::write(this, v);
    }
};

template<> // StreamWriter specialization for structs
struct StreamWriter<StreamTraitTypeStruct>
{
    template<typename T>
    static void write(const OutputStreamPtr& outS, const T& v)
    {
        v.ice_write(outS);
    }
};

template<> // StreamReader specialization for structs
struct StreamReader<StreamTraitTypeStruct>
{
    template<typename T>
    static void read(const InputStreamPtr& inS, T& v)
    {
        v.ice_read(inS);
    }
};

template<> // StreamWriter specialization for structs with cpp:class metadata
struct StreamWriter<StreamTraitTypeStructClass>
{
    template<typename T>
    static void write(const OutputStreamPtr& outS, const T& v)
    {
        v->ice_write(outS);
    }
};

template<> // StreamReader specialization for structs with cpp:class metadata
struct StreamReader<StreamTraitTypeStructClass>
{
    template<typename T>
    static void read(const InputStreamPtr& inS, const T& v)
    {
        v->ice_read(inS);
    }
};

template<> // StreamWriter specialization for byte enums
struct StreamWriter<StreamTraitTypeByteEnum>
{
    template<typename T>
    static void write(const OutputStreamPtr& outS, const T& v)
    {
        if(static_cast<int>(v) < 0 || static_cast<int>(v) >= StreamTrait<T>::enumLimit)
        {
            IceInternal::Ex::throwMarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        outS->write(static_cast<Byte>(v));
    }
};

template<> // StreamReader specialization for byte enums
struct StreamReader<StreamTraitTypeByteEnum>
{
    template<typename T>
    static void read(const InputStreamPtr& inS, T& v)
    {
        Byte val;
        inS->read(val);
        if(val > StreamTrait<T>::enumLimit)
        {
            IceInternal::Ex::throwMarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        v = static_cast<T>(val);
    }
};

template<> // StreamWriter specialization for short enums
struct StreamWriter<StreamTraitTypeShortEnum>
{
    template<typename T>
    static void write(const OutputStreamPtr& outS, const T& v)
    {
        if(static_cast<int>(v) < 0 || static_cast<int>(v) >= StreamTrait<T>::enumLimit)
        {
            IceInternal::Ex::throwMarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        outS->write(static_cast<Short>(v));
    }
};

template<> // StreamReader specialization for short enums
struct StreamReader<StreamTraitTypeShortEnum>
{
    template<typename T>
    static void read(const InputStreamPtr& inS, T& v)
    {
        Short val;
        inS->read(val);
        if(val < 0 || val > StreamTrait<T>::enumLimit)
        {
            IceInternal::Ex::throwMarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        v = static_cast<T>(val);
    }
};

template<> // StreamWriter specialization for int enums
struct StreamWriter<StreamTraitTypeIntEnum>
{
    template<typename T>
    static void write(const OutputStreamPtr& outS, const T& v)
    {
        if(static_cast<int>(v) < 0 || static_cast<int>(v) >= StreamTrait<T>::enumLimit)
        {
            IceInternal::Ex::throwMarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        outS->write(static_cast<Int>(v));
    }
};

template<> // StreamReader specialization for int enums
struct StreamReader<StreamTraitTypeIntEnum>
{
    template<typename T>
    static void read(const InputStreamPtr& inS, T& v)
    {
        Int val;
        inS->read(val);
        if(val < 0 || val > StreamTrait<T>::enumLimit)
        {
            IceInternal::Ex::throwMarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        v = static_cast<T>(val);
    }
};

template<> // StreamWriter specialization for sequences
struct StreamWriter<StreamTraitTypeSequence>
{
    template<typename T>
    static void write(const OutputStreamPtr& outS, const T& v)
    {
        outS->writeSize(static_cast<Int>(v.size()));
        for(typename T::const_iterator p = v.begin(); p != v.end(); ++p)
        {
            outS->write(*p);
        }
    }
};

template<> // StreamReader specialization for sequences
struct StreamReader<StreamTraitTypeSequence>
{
    template<typename T>
    static void read(const InputStreamPtr& inS, T& v)
    {
        Int sz = inS->readAndCheckSeqSize(StreamTrait<typename T::value_type>::minWireSize);
        v.resize(sz);
        for(typename T::iterator p = v.begin(); p != v.end(); ++p)
        {
            inS->read(*p);
        }
    }
};

#ifndef __BCPLUSPLUS__ // COMPILERFIX: See StreamTrait<vector<bool>> comment above
template<> // StreamWriter specialization for sequences of bool
struct StreamWriter<StreamTraitTypeSequenceBool>
{
    template<typename T>
    static void write(const OutputStreamPtr& outS, const T& v)
    {
        outS->writeSize(static_cast<Int>(v.size()));
        for(typename T::const_iterator p = v.begin(); p != v.end(); ++p)
        {
            outS->write(*p);
        }
    }
};

template<> // Reader specialization for sequences of bool
struct StreamReader<StreamTraitTypeSequenceBool>
{
    template<typename T>
    static void read(const InputStreamPtr& inS, T& v)
    {
        Int sz = inS->readAndCheckSeqSize(StreamTrait<bool>::minWireSize);
        v.resize(sz);
        for(typename T::iterator p = v.begin(); p != v.end(); ++p)
        {
            //
            // We can't just call inS->read(*p) here because *p is
            // a compiler dependent bit reference.
            //
            bool b;
            inS->read(b);
            *p = b;
        }
    }
};
#endif

template<> // StreamWriter specialization for dictionaries.
struct StreamWriter<StreamTraitTypeDictionary>
{
    template<typename T>
    static void write(const OutputStreamPtr& outS, const T& v)
    {
        outS->writeSize(static_cast<Int>(v.size()));
        typename T::const_iterator p;
        for(p = v.begin(); p != v.end(); ++p)
        {
            outS->write(p->first);
            outS->write(p->second);
        }
    }
};

template<> // StreamReader specialization for dictionaries.
struct StreamReader<StreamTraitTypeDictionary>
{
    template<typename T>
    static void read(const InputStreamPtr& inS, T& v)
    {
        Int sz = inS->readSize();
        while(sz--)
        {
            typename T::value_type p;
            inS->read(const_cast<typename T::key_type&>(p.first));
            typename T::iterator i = v.insert(v.end(), p);
            inS->read(i->second);
        }
    }
};

template<> // StreamWriter specialization for UserExceptions.
struct StreamWriter<StreamTraitTypeUserException>
{
    template<typename T>
    static void write(const OutputStreamPtr& outS, const T& v)
    {
        outS->writeException(v);
    }
};

class ICE_API ObjectReader : public Object
{
public:

    virtual void read(const InputStreamPtr&) = 0;

private:

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*);

    virtual void __write(const OutputStreamPtr&) const;
    virtual void __read(const InputStreamPtr&);
};
typedef ::IceInternal::Handle<ObjectReader> ObjectReaderPtr;

class ICE_API ObjectWriter : public Object
{
public:

    virtual void write(const OutputStreamPtr&) const = 0;

private:

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*);

    virtual void __write(const OutputStreamPtr&) const;
    virtual void __read(const InputStreamPtr&);
};
typedef ::IceInternal::Handle<ObjectWriter> ObjectWriterPtr;

class ICE_API UserExceptionWriter : public UserException
{
public:

    UserExceptionWriter(const CommunicatorPtr&);
    ~UserExceptionWriter() throw();

    virtual void write(const OutputStreamPtr&) const = 0;
    virtual bool usesClasses() const = 0;

    virtual ::std::string ice_name() const = 0;
    virtual Exception* ice_clone() const = 0;
    virtual void ice_throw() const = 0;

    virtual void __write(IceInternal::BasicStream*) const;
    virtual void __read(IceInternal::BasicStream*);

    virtual bool __usesClasses() const;

protected:

    const CommunicatorPtr _communicator;
};

}

#endif
