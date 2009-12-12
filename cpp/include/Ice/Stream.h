// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
#include <IceUtil/Shared.h>

namespace Ice
{
    
#if defined(_MSC_VER) && (_MSC_VER < 1300) // COMPILERBUG
//
// VC++ 6 compiler bugs doesn't allow using templates for the Stream API.
//
// see: http://support.microsoft.com/kb/240866
//      http://support.microsoft.com/kb/241569
//
#else

enum StreamTraitType
{
    StreamTraitTypeBuiltin,
    StreamTraitTypeStruct,
    StreamTraitTypeStructClass, // struct with cpp:class metadata
    StreamTraitTypeByteEnum,    // Enums with up to 127 enumerators
    StreamTraitTypeShortEnum,   // Enums with up to 32767 enumerators
    StreamTraitTypeIntEnum,     // Enums with more than 32767 enumerators
    StreamTraitTypeSequence,
    StreamTraitTypeDictionary,
    StreamTraitTypeUserException,
    StreamTraitTypeUnknown
};

// Forward declaration required for writer specializations.
class MarshalException;

//
// Base trait template. This doesn't actually do anything -- we just
// use it as a template that we can specialize.
//
template<typename T>
struct StreamTrait
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeUnknown;
    static const int minWireSize = 0;
};

//
// StreamTrait specialization for std::vector
//
template<typename T>
struct StreamTrait< std::vector<T> >
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeSequence;
    static const int minWireSize = 1;
};

template<>
struct StreamTrait<UserException>
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeUserException;
};

//
// StreamTrait specialization for std::map.
//
template<typename K, typename V>
struct StreamTrait< std::map<K, V> >
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeDictionary;
    static const int minWireSize = 1;
};

//
// StreamTrait specialization for builtins (these are need for sequence
// marshalling to figure out the minWireSize of each built-in).
//
template<>
struct StreamTrait< bool>
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeBuiltin;
    static const int minWireSize = 1;
};

template<>
struct StreamTrait< ::Ice::Byte>
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeBuiltin;
    static const int minWireSize = 1;
};

template<>
struct StreamTrait< ::Ice::Short>
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeBuiltin;
    static const int minWireSize = 2;
};

template<>
struct StreamTrait< ::Ice::Int>
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeBuiltin;
    static const int minWireSize = 4;
};

template<>
struct StreamTrait< ::Ice::Long>
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeBuiltin;
    static const int minWireSize = 8;
};

template<>
struct StreamTrait< ::Ice::Float>
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeBuiltin;
    static const int minWireSize = 4;
};

template<>
struct StreamTrait< ::Ice::Double>
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeBuiltin;
    static const int minWireSize = 8;
};

template<>
struct StreamTrait< ::std::string>
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeBuiltin;
    static const int minWireSize = 1;
};

template<>
struct StreamTrait< ::std::wstring>
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeBuiltin;
    static const int minWireSize = 1;
};

template<typename T>
struct StreamTrait< ::IceInternal::ProxyHandle<T> >
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeBuiltin;
    static const int minWireSize = 2;
};

template<typename T>
struct StreamTrait< ::IceInternal::Handle<T> >
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeBuiltin;
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
    static void write(const ::Ice::OutputStreamPtr&, const T&)
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
    static void read(const ::Ice::InputStreamPtr&, T&)
    {
        // This asserts because we end up reading something for wich
        // we never define a trait.
        assert(false);
    }
};

template<typename T>void
__patch__Ptr(void* __addr, ::Ice::ObjectPtr& v)
{
   ::IceInternal::Handle<T>* p = static_cast< ::IceInternal::Handle<T>*>(__addr);
   assert(p);
   *p = ::IceInternal::Handle<T>::dynamicCast(v);
   if(v && !*p)
   {
       IceInternal::Ex::throwUOE(T::ice_staticId(), v->ice_id());
   }
}
#endif

class ICE_API ReadObjectCallback : public ::IceUtil::Shared
{
public:

    virtual void invoke(const ::Ice::ObjectPtr&) = 0;
};
typedef IceUtil::Handle< ReadObjectCallback > ReadObjectCallbackPtr;

class ICE_API ReadObjectCallbackI : public ReadObjectCallback
{
public:

    typedef void (*PatchFunc)(void*, Ice::ObjectPtr&);

    ReadObjectCallbackI(PatchFunc, void*);

    virtual void invoke(const ::Ice::ObjectPtr&);

private:

    PatchFunc _func;
    void* _arg;
};

class ICE_API InputStream : public ::IceUtil::Shared
{
public:

    virtual Ice::CommunicatorPtr communicator() const = 0;

    virtual void sliceObjects(bool) = 0;

    virtual bool readBool() = 0;
    virtual ::std::vector< bool > readBoolSeq() = 0;
    virtual bool* readBoolSeq(::std::pair<const bool*, const bool*>&) = 0;

    virtual ::Ice::Byte readByte() = 0;
    virtual ::std::vector< ::Ice::Byte > readByteSeq() = 0;
    virtual void readByteSeq(::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&) = 0;

    virtual ::Ice::Short readShort() = 0;
    virtual ::std::vector< ::Ice::Short > readShortSeq() = 0;
    virtual ::Ice::Short* readShortSeq(::std::pair<const ::Ice::Short*, const ::Ice::Short*>&) = 0;

    virtual ::Ice::Int readInt() = 0;
    virtual ::std::vector< ::Ice::Int > readIntSeq() = 0;
    virtual ::Ice::Int* readIntSeq(::std::pair<const ::Ice::Int*, const ::Ice::Int*>&) = 0;

    virtual ::Ice::Long readLong() = 0;
    virtual ::std::vector< ::Ice::Long > readLongSeq() = 0;
    virtual ::Ice::Long* readLongSeq(::std::pair<const ::Ice::Long*, const ::Ice::Long*>&) = 0;

    virtual ::Ice::Float readFloat() = 0;
    virtual ::std::vector< ::Ice::Float > readFloatSeq() = 0;
    virtual ::Ice::Float* readFloatSeq(::std::pair<const ::Ice::Float*, const ::Ice::Float*>&) = 0;

    virtual ::Ice::Double readDouble() = 0;
    virtual ::std::vector< ::Ice::Double > readDoubleSeq() = 0;
    virtual ::Ice::Double* readDoubleSeq(::std::pair<const ::Ice::Double*, const ::Ice::Double*>&) = 0;

    virtual ::std::string readString(bool = true) = 0;
    virtual ::std::vector< ::std::string > readStringSeq(bool = true) = 0;

    virtual ::std::wstring readWstring() = 0;
    virtual ::std::vector< ::std::wstring > readWstringSeq() = 0;

    virtual ::Ice::Int readSize() = 0;
    virtual ::Ice::Int readAndCheckSeqSize(int) = 0;

    virtual ::Ice::ObjectPrx readProxy() = 0;

    virtual void readObject(const ::Ice::ReadObjectCallbackPtr&) = 0;

    virtual ::std::string readTypeId() = 0;

    virtual void throwException() = 0;

    virtual void startSlice() = 0;
    virtual void endSlice() = 0;
    virtual void skipSlice() = 0;

    virtual void startEncapsulation() = 0;
    virtual void endEncapsulation() = 0;
    virtual void skipEncapsulation() = 0;

    virtual void readPendingObjects() = 0;

    virtual void rewind() = 0;

#if defined(_MSC_VER) && (_MSC_VER < 1300) // COMPILERBUG
    //
    // VC++ 6 compiler bugs doesn't allow using templates for the Stream API.
    //
    // see: http://support.microsoft.com/kb/240866
    //      http://support.microsoft.com/kb/241569
    //
#else
    //
    // read overloads for std::vector<bool>.
    //
    // We need all this variants because std::vector<bool> is defined in C++ standard
    // as a vector of bit references, but there isn't standar type for bit references.
    //
    void
#   if defined(_MSC_VER) && (_MSC_VER >= 1300)
#      if defined (ICE_64)
    // std::vector<bool> optimization for Vs2008 x64
    read(std::_Vb_reference<unsigned __int64, __int64, std::vector<bool, std::allocator<bool> > > v)
#      else
    // std::vector<bool> optimization for Vs2008 x86
    read(std::_Vb_reference<unsigned int, int, std::vector<bool, std::allocator<bool> > > v)
#      endif
#   elif defined(__BCPLUSPLUS__)
    read(std::_Vb_reference<unsigned int, int> v) // std::vector<bool> optimization for Borland C++ Builder
#   else
    read(std::_Bit_reference v) // default optimization for GCC
#   endif
    {
        v = readBool();
    }

    inline void
    read(bool& v)
    {
        v = readBool();
    }

    inline void
    read(::Ice::Byte& v)
    {
        v = readByte();
    }

    inline void
    read(::Ice::Short& v)
    {
        v = readShort();
    }

    inline void
    read(Ice::Int& v)
    {
        v = readInt();
    }

    inline void
    read(Ice::Long& v)
    {
        v = readLong();
    }

    inline void
    read(Ice::Float& v)
    {
        v = readFloat();
    }

    inline void
    read(Ice::Double& v)
    {
        v = readDouble();
    }

    inline void
    read(std::string& v)
    {
        v = readString();
    }

    inline void
    read(std::wstring& v)
    {
        v = readWstring();
    }

    template<typename T> inline void
    read(::IceInternal::ProxyHandle<T>& v)
    {
        ::Ice::ObjectPrx proxy = readProxy();
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
        ::Ice::ReadObjectCallbackPtr cb = new ::Ice::ReadObjectCallbackI(&__patch__Ptr< T>, &v);
        readObject(cb);
    }

    template<typename T> inline void
    read(T& v)
    {
        StreamReader< ::Ice::StreamTrait<T>::type>::read(this, v);
    }
#endif
};

class ICE_API OutputStream : public ::IceUtil::Shared
{
public:

    virtual Ice::CommunicatorPtr communicator() const = 0;

    virtual void writeBool(bool) = 0;
    virtual void writeBoolSeq(const ::std::vector< bool >&) = 0;
    virtual void writeBoolSeq(const bool*, const bool*) = 0;

    virtual void writeByte(::Ice::Byte) = 0;
    virtual void writeByteSeq(const ::std::vector< ::Ice::Byte >&) = 0;
    virtual void writeByteSeq(const Ice::Byte*, const Ice::Byte*) = 0;

    virtual void writeShort(::Ice::Short) = 0;
    virtual void writeShortSeq(const ::std::vector< ::Ice::Short >&) = 0;
    virtual void writeShortSeq(const Ice::Short*, const Ice::Short*) = 0;

    virtual void writeInt(::Ice::Int) = 0;
    virtual void writeIntSeq(const ::std::vector< ::Ice::Int >&) = 0;
    virtual void writeIntSeq(const Ice::Int*, const Ice::Int*) = 0;

    virtual void writeLong(::Ice::Long) = 0;
    virtual void writeLongSeq(const ::std::vector< ::Ice::Long >&) = 0;
    virtual void writeLongSeq(const Ice::Long*, const Ice::Long*) = 0;

    virtual void writeFloat(::Ice::Float) = 0;
    virtual void writeFloatSeq(const ::std::vector< ::Ice::Float >&) = 0;
    virtual void writeFloatSeq(const Ice::Float*, const Ice::Float*) = 0;

    virtual void writeDouble(::Ice::Double) = 0;
    virtual void writeDoubleSeq(const ::std::vector< ::Ice::Double >&) = 0;
    virtual void writeDoubleSeq(const Ice::Double*, const Ice::Double*) = 0;

    virtual void writeString(const ::std::string&, bool = true) = 0;
    virtual void writeStringSeq(const ::std::vector< ::std::string >&, bool = true) = 0;

    virtual void writeWstring(const ::std::wstring&) = 0;
    virtual void writeWstringSeq(const ::std::vector< ::std::wstring >&) = 0;

    virtual void writeSize(::Ice::Int) = 0;

    virtual void writeProxy(const ::Ice::ObjectPrx&) = 0;

    virtual void writeObject(const ::Ice::ObjectPtr&) = 0;

    virtual void writeTypeId(const ::std::string&) = 0;

    virtual void writeException(const ::Ice::UserException&) = 0;

    virtual void startSlice() = 0;
    virtual void endSlice() = 0;

    virtual void startEncapsulation() = 0;
    virtual void endEncapsulation() = 0;

    virtual void writePendingObjects() = 0;

    virtual void finished(::std::vector< ::Ice::Byte >&) = 0;

    virtual void reset(bool) = 0;

#if defined(_MSC_VER) && (_MSC_VER < 1300) // COMPILERBUG
//
// VC++ 6 compiler bugs doesn't allow using templates for the Stream API.
//
// see: http://support.microsoft.com/kb/240866
//      http://support.microsoft.com/kb/241569
//
#else
    inline void
    write(bool v)
    {
        writeBool(v);
    }

    inline  void
    write(::Ice::Byte v)
    {
        writeByte(v);
    }

    inline  void
    write(::Ice::Short v)
    {
        writeShort(v);
    }

    inline  void
    write(::Ice::Int v)
    {
        writeInt(v);
    }

    inline  void
    write(::Ice::Long v)
    {
        writeLong(v);
    }

    inline  void
    write(::Ice::Float v)
    {
        writeFloat(v);
    }

    inline  void
    write(::Ice::Double v)
    {
        writeDouble(v);
    }

    inline  void
    write(const std::string& v)
    {
        writeString(v);
    }
    
    inline  void
    write(const char* v)
    {
        writeString(v);
    }

    inline  void
    write(const std::wstring& v)
    {
        writeWstring(v);
    }

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
        StreamWriter< ::Ice::StreamTrait<T>::type>::write(this, v);
    }
#endif
};

#if defined(_MSC_VER) && (_MSC_VER < 1300) // COMPILERBUG
//
// VC++ 6 compiler bugs doesn't allow using templates for the Stream API.
//
// see: http://support.microsoft.com/kb/240866
//      http://support.microsoft.com/kb/241569
//
#else
template<> // StreamWriter specialization for structs
struct StreamWriter<StreamTraitTypeStruct>
{
    template<typename T>
    static void write(const ::Ice::OutputStreamPtr& outS, const T& v)
    {
        v.ice_write(outS);
    }
};

template<> // StreamReader specialization for structs
struct StreamReader<StreamTraitTypeStruct>
{
    template<typename T>
    static void read(const ::Ice::InputStreamPtr& inS, T& v)
    {
        v.ice_read(inS);
    }
};

template<> // StreamWriter specialization for structs with cpp:class metadata
struct StreamWriter<StreamTraitTypeStructClass>
{
    template<typename T>
    static void write(const ::Ice::OutputStreamPtr& outS, const T& v)
    {
        v->ice_write(outS);
    }
};

template<> // StreamReader specialization for structs with cpp:class metadata
struct StreamReader<StreamTraitTypeStructClass>
{
    template<typename T>
    static void read(const ::Ice::InputStreamPtr& inS, const T& v)
    {
        v->ice_read(inS);
    }
};

template<> // StreamWriter specialization for byte enums
struct StreamWriter<StreamTraitTypeByteEnum>
{
    template<typename T>
    static void write(const ::Ice::OutputStreamPtr& outS, const T& v)
    {
        if(static_cast<int>(v) < 0 || static_cast<int>(v) >= ::Ice::StreamTrait<T>::enumLimit)
        {
            throw ::Ice::MarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        outS->writeByte(static_cast< ::Ice::Byte>(v));
    }
};

template<> // StreamReader specialization for byte enums
struct StreamReader<StreamTraitTypeByteEnum>
{
    template<typename T>
    static void read(const ::Ice::InputStreamPtr& inS, T& v)
    {
        ::Ice::Byte val = inS->readByte();
        if(val > ::Ice::StreamTrait<T>::enumLimit)
        {
            throw ::Ice::MarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        v = static_cast<T>(val);
    }
};


template<> // StreamWriter specialization for short enums
struct StreamWriter<StreamTraitTypeShortEnum>
{
    template<typename T>
    static void write(const ::Ice::OutputStreamPtr& outS, const T& v)
    {
        if(static_cast<int>(v) < 0 || static_cast<int>(v) >= ::Ice::StreamTrait<T>::enumLimit)
        {
            throw ::Ice::MarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        outS->writeShort(static_cast< ::Ice::Short>(v));
    }
};

template<> // StreamReader specialization for short enums
struct StreamReader<StreamTraitTypeShortEnum>
{
    template<typename T>
    static void read(const ::Ice::InputStreamPtr& inS, T& v)
    {
        ::Ice::Short val = inS->readShort();
        if(val < 0 || val > ::Ice::StreamTrait<T>::enumLimit)
        {
            throw ::Ice::MarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        v = static_cast<T>(val);
    }
};

template<> // StreamWriter specialization for int enums
struct StreamWriter<StreamTraitTypeIntEnum>
{
    template<typename T>
    static void write(const ::Ice::OutputStreamPtr& outS, const T& v)
    {
        if(static_cast<int>(v) < 0 || static_cast<int>(v) >= ::Ice::StreamTrait<T>::enumLimit)
        {
            throw ::Ice::MarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        outS->writeInt(static_cast< ::Ice::Int>(v));
    }
};

template<> // StreamReader specialization for int enums
struct StreamReader<StreamTraitTypeIntEnum>
{
    template<typename T>
    static void read(const ::Ice::InputStreamPtr& inS, T& v)
    {
        ::Ice::Int val = inS->readInt();
        if(val < 0 || val > ::Ice::StreamTrait<T>::enumLimit)
        {
            throw ::Ice::MarshalException(__FILE__, __LINE__, "enumerator out of range");
        }
        v = static_cast<T>(val);
    }
};

template<> // StreamWriter specialization for sequences
struct StreamWriter<StreamTraitTypeSequence>
{
    template<typename T>
    static void write(const ::Ice::OutputStreamPtr& outS, const T& v)
    {
        outS->writeSize(static_cast< ::Ice::Int>(v.size()));
        typename T::const_iterator p;
        for(p = v.begin(); p != v.end(); ++p)
        {
            outS->write((*p));
        }
    }
};

template<> // StreamReader specialization for sequences
struct StreamReader<StreamTraitTypeSequence>
{
    template<typename T>
    static void read(const ::Ice::InputStreamPtr& inS, T& v)
    {
        ::Ice::Int sz = inS->readAndCheckSeqSize(::Ice::StreamTrait<typename T::value_type>::minWireSize);
        v.resize(sz);
        for(int i = 0; i < sz; ++i)
        {
            inS->read(v[i]);
        }
    }
};

template<> // StreamWriter specialization for dictionaries.
struct StreamWriter<StreamTraitTypeDictionary>
{
    template<typename T>
    static void write(const ::Ice::OutputStreamPtr& outS, const T& v)
    {
        outS->writeSize(static_cast< ::Ice::Int>(v.size()));
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
    static void read(const ::Ice::InputStreamPtr& inS, T& v)
    {
        ::Ice::Int sz = inS->readSize();
        while(sz--)
        {
            typename T::value_type p;
            inS->read(const_cast< typename T::key_type&>(p.first));
            typename T::iterator i = v.insert(v.end(), p);
            inS->read(i->second);
        }
    }
};

template<> // StreamWriter specialization for UserExceptions.
struct StreamWriter<StreamTraitTypeUserException>
{
    template<typename T>
    static void write(const ::Ice::OutputStreamPtr& outS, const T& v)
    {
        outS->writeException(v);
    }
};

#endif

class ICE_API ObjectReader : public ::Ice::Object
{
public:

    virtual void read(const InputStreamPtr&, bool) = 0;

private:

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool = true);

    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};
typedef ::IceInternal::Handle< ObjectReader > ObjectReaderPtr;

class ICE_API ObjectWriter : public ::Ice::Object
{
public:

    virtual void write(const OutputStreamPtr&) const = 0;

private:

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool = true);

    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};
typedef ::IceInternal::Handle< ObjectWriter > ObjectWriterPtr;


class ICE_API UserExceptionWriter : public UserException
{
public:

    UserExceptionWriter(const Ice::CommunicatorPtr&);
    ~UserExceptionWriter() throw();

    virtual void write(const OutputStreamPtr&) const = 0;
    virtual bool usesClasses() const = 0;

    virtual std::string ice_name() const = 0;
    virtual Ice::Exception* ice_clone() const = 0;
    virtual void ice_throw() const = 0;

    virtual void __write(IceInternal::BasicStream*) const;
    virtual void __read(IceInternal::BasicStream*, bool);

    virtual bool __usesClasses() const;

protected:

    Ice::CommunicatorPtr _communicator;
};

}

#endif
