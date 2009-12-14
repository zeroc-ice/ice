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
#include <Ice/Proxy.h>
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
    StreamTraitTypeStructClass,     // struct with cpp:class metadata
    StreamTraitTypeByteEnum,        // Enums with up to 127 enumerators
    StreamTraitTypeShortEnum,       // Enums with up to 32767 enumerators
    StreamTraitTypeIntEnum,         // Enums with more than 32767 enumerators
    StreamTraitTypeSequence,
//  
// BCC2010 compiler bugs doesn't allow use full specialization over partial specialization.
//
#   ifndef __BCPLUSPLUS__
    StreamTraitTypeSequenceBool,    // Sequences of bool need a special case
                                    // becasue c++ optimizations for vector<bool>
                                    // are not portable across compilers.
#   endif
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

//
// BCC2010 compiler bugs doesn't allow use full specialization over partial specialization.
//
#   ifndef __BCPLUSPLUS__
//
// StreamTrait specialization for std::vector<bool>
//
template<>
struct StreamTrait< std::vector<bool, std::allocator<bool> > > 
{
    static const ::Ice::StreamTraitType type = Ice::StreamTraitTypeSequenceBool;
    static const int minWireSize = 1;
};
#   endif

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

//
// BCC2010 compiler bugs doesn't allow use full specialization over partial specialization.
//
#   if defined(__BCPLUSPLUS__)
    //
    // std::vector<bool> optimization use a hidden reference to
    // a bit instead of a reference to bool.
    //
    void
    read(std::_Vb_reference<unsigned int, int> v)
    {
        v = readBool();
    }
#   endif

    ICE_DEPRECATED_API virtual bool readBool()
    {
        return internalReadBool();
    }
    
    ICE_DEPRECATED_API virtual ::std::vector< bool > readBoolSeq()
    {
        return internalReadBoolSeq();
    }
    
    ICE_DEPRECATED_API virtual bool* readBoolSeq(::std::pair<const bool*, const bool*>& p)
    {
        return internalReadBoolSeq(p);
    }

    ICE_DEPRECATED_API virtual ::Ice::Byte readByte()
    {
        return internalReadByte();
    }
    
    ICE_DEPRECATED_API virtual ::std::vector< ::Ice::Byte > readByteSeq()
    {
        return internalReadByteSeq();
    }
    
    ICE_DEPRECATED_API virtual void readByteSeq(::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& p)
    {
        return internalReadByteSeq(p);
    }

    ICE_DEPRECATED_API virtual ::Ice::Short readShort()
    {
        return internalReadShort();
    }
    
    ICE_DEPRECATED_API virtual ::std::vector< ::Ice::Short > readShortSeq()
    {
        return internalReadShortSeq();
    }
    
    ICE_DEPRECATED_API virtual ::Ice::Short* readShortSeq(::std::pair<const ::Ice::Short*, const ::Ice::Short*>& p)
    {
        return internalReadShortSeq(p);
    }

    ICE_DEPRECATED_API virtual ::Ice::Int readInt()
    {
        return internalReadInt();
    }
    
    ICE_DEPRECATED_API virtual ::std::vector< ::Ice::Int > readIntSeq()
    {
        return internalReadIntSeq();
    }
    
    ICE_DEPRECATED_API virtual ::Ice::Int* readIntSeq(::std::pair<const ::Ice::Int*, const ::Ice::Int*>& p)
    {
        return internalReadIntSeq(p);
    }

    ICE_DEPRECATED_API virtual ::Ice::Long readLong()
    {
        return internalReadLong();
    }
    
    ICE_DEPRECATED_API virtual ::std::vector< ::Ice::Long > readLongSeq()
    {
        return internalReadLongSeq();
    }
    
    ICE_DEPRECATED_API virtual ::Ice::Long* readLongSeq(::std::pair<const ::Ice::Long*, const ::Ice::Long*>& p)
    {
        return internalReadLongSeq(p);
    }

    ICE_DEPRECATED_API virtual ::Ice::Float readFloat()
    {
        return internalReadFloat();
    }
    
    ICE_DEPRECATED_API virtual ::std::vector< ::Ice::Float > readFloatSeq()
    {
        return internalReadFloatSeq();
    }
    
    ICE_DEPRECATED_API virtual ::Ice::Float* readFloatSeq(::std::pair<const ::Ice::Float*, const ::Ice::Float*>& p)
    {
        return internalReadFloatSeq(p);
    }

    ICE_DEPRECATED_API virtual ::Ice::Double readDouble()
    {
        return internalReadDouble();
    }
    
    ICE_DEPRECATED_API virtual ::std::vector< ::Ice::Double > readDoubleSeq()
    {
        return internalReadDoubleSeq();
    }
    
    ICE_DEPRECATED_API virtual ::Ice::Double* readDoubleSeq(::std::pair<const ::Ice::Double*, const ::Ice::Double*>& p)
    {
        return internalReadDoubleSeq(p);
    }

    ICE_DEPRECATED_API virtual ::std::string readString(bool convert = true)
    {
        return internalReadString(convert);
    }
    
    ICE_DEPRECATED_API virtual ::std::vector< ::std::string > readStringSeq(bool convert = true)
    {
        return internalReadStringSeq(convert);
    }

    ICE_DEPRECATED_API virtual ::std::wstring readWstring()
    {
        return internalReadWstring();
    }

    ICE_DEPRECATED_API virtual ::std::vector< ::std::wstring > readWstringSeq()
    {
        return internalReadWstringSeq();
    }

    virtual ::Ice::Int readSize() = 0;
    virtual ::Ice::Int readAndCheckSeqSize(int) = 0;

    ICE_DEPRECATED_API virtual ::Ice::ObjectPrx readProxy()
    {
        return internalReadProxy();
    }

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
    inline void
    read(bool& v)
    {
        v = internalReadBool();
    }

    inline void
    read(::Ice::Byte& v)
    {
        v = internalReadByte();
    }

    inline void
    read(::Ice::Short& v)
    {
        v = internalReadShort();
    }

    inline void
    read(Ice::Int& v)
    {
        v = internalReadInt();
    }

    inline void
    read(Ice::Long& v)
    {
        v = internalReadLong();
    }

    inline void
    read(Ice::Float& v)
    {
        v = internalReadFloat();
    }

    inline void
    read(Ice::Double& v)
    {
        v = internalReadDouble();
    }

    inline void
    read(std::string& v, bool convert = true)
    {
        v = internalReadString();
    }

    inline void
    read(std::wstring& v)
    {
        v = internalReadWstring();
    }

    template<typename T> inline void
    read(::IceInternal::ProxyHandle<T>& v)
    {
        ::Ice::ObjectPrx proxy = internalReadProxy();
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

protected:
    
    virtual bool internalReadBool() = 0;
    virtual ::Ice::Byte internalReadByte() = 0;
    virtual ::Ice::Short internalReadShort() = 0;
    virtual ::Ice::Int internalReadInt() = 0;
    virtual ::Ice::Long internalReadLong() = 0;
    virtual ::Ice::Float internalReadFloat() = 0;
    virtual ::Ice::Double internalReadDouble() = 0;
    virtual ::std::string internalReadString(bool = true) = 0;
    virtual ::std::wstring internalReadWstring() = 0;
    virtual ::Ice::ObjectPrx internalReadProxy() = 0;
    
    //
    // Remove these methods when the old Stream api, is removed.
    //
    virtual std::vector< bool > internalReadBoolSeq() = 0;
    virtual bool* internalReadBoolSeq(std::pair<const bool*, const bool*>&) = 0;

    virtual std::vector< Ice::Byte > internalReadByteSeq() = 0;
    virtual void internalReadByteSeq(std::pair<const Ice::Byte*, const Ice::Byte*>&) = 0;

    virtual std::vector< Ice::Short > internalReadShortSeq() = 0;
    virtual Ice::Short* internalReadShortSeq(std::pair<const Ice::Short*, const Ice::Short*>&) = 0;

    virtual std::vector< Ice::Int > internalReadIntSeq() = 0;
    virtual Ice::Int* internalReadIntSeq(std::pair<const Ice::Int*, const Ice::Int*>&) = 0;

    virtual std::vector< Ice::Long > internalReadLongSeq() = 0;
    virtual Ice::Long* internalReadLongSeq(std::pair<const Ice::Long*, const Ice::Long*>&) = 0;

    virtual std::vector< Ice::Float > internalReadFloatSeq() = 0;
    virtual Ice::Float* internalReadFloatSeq(std::pair<const Ice::Float*, const Ice::Float*>&) = 0;

    virtual std::vector< Ice::Double > internalReadDoubleSeq() = 0;
    virtual Ice::Double* internalReadDoubleSeq(std::pair<const Ice::Double*, const Ice::Double*>&) = 0;

    virtual std::vector< std::string > internalReadStringSeq(bool = true) = 0;

    virtual std::vector< std::wstring > internalReadWstringSeq() = 0;
};

class ICE_API OutputStream : public ::IceUtil::Shared
{
public:

    virtual Ice::CommunicatorPtr communicator() const = 0;

    ICE_DEPRECATED_API virtual void writeBool(bool v)
    {
        internalWriteBool(v);
    }
    
    ICE_DEPRECATED_API virtual void writeBoolSeq(const ::std::vector< bool >& v)
    {
        internalWriteBoolSeq(v);
    }
    
    ICE_DEPRECATED_API virtual void writeBoolSeq(const bool* begin, const bool* end)
    {
        internalWriteBoolSeq(begin, end);
    }

    ICE_DEPRECATED_API virtual void writeByte(::Ice::Byte v)
    {
        internalWriteByte(v);
    }
    
    ICE_DEPRECATED_API virtual void writeByteSeq(const ::std::vector< ::Ice::Byte >& v)
    {
        internalWriteByteSeq(v);
    }
    
    ICE_DEPRECATED_API virtual void writeByteSeq(const Ice::Byte* begin, const Ice::Byte* end)
    {
        internalWriteByteSeq(begin, end);
    }

    ICE_DEPRECATED_API virtual void writeShort(::Ice::Short v)
    {
        internalWriteShort(v);
    }
    
    ICE_DEPRECATED_API virtual void writeShortSeq(const ::std::vector< ::Ice::Short >& v)
    {
        internalWriteShortSeq(v);
    }
    
    ICE_DEPRECATED_API virtual void writeShortSeq(const Ice::Short* begin, const Ice::Short* end)
    {
        internalWriteShortSeq(begin, end);
    }

    ICE_DEPRECATED_API virtual void writeInt(::Ice::Int v)
    {
        internalWriteInt(v);
    }
    
    ICE_DEPRECATED_API virtual void writeIntSeq(const ::std::vector< ::Ice::Int >& v)
    {
        internalWriteIntSeq(v);
    }
    
    ICE_DEPRECATED_API virtual void writeIntSeq(const Ice::Int* begin, const Ice::Int* end)
    {
        internalWriteIntSeq(begin, end);
    }

    ICE_DEPRECATED_API virtual void writeLong(::Ice::Long v)
    {
        internalWriteLong(v);
    }
    
    ICE_DEPRECATED_API virtual void writeLongSeq(const ::std::vector< ::Ice::Long >& v)
    {
        internalWriteLongSeq(v);
    }
    
    ICE_DEPRECATED_API virtual void writeLongSeq(const Ice::Long* begin, const Ice::Long* end)
    {
        internalWriteLongSeq(begin, end);
    }

    ICE_DEPRECATED_API virtual void writeFloat(::Ice::Float v)
    {
        internalWriteFloat(v);
    }
    
    ICE_DEPRECATED_API virtual void writeFloatSeq(const ::std::vector< ::Ice::Float >& v)
    {
        internalWriteFloatSeq(v);
    }
    
    ICE_DEPRECATED_API virtual void writeFloatSeq(const Ice::Float* begin, const Ice::Float* end)
    {
        internalWriteFloatSeq(begin, end);
    }

    ICE_DEPRECATED_API virtual void writeDouble(::Ice::Double v)
    {
        internalWriteDouble(v);
    }
    
    ICE_DEPRECATED_API virtual void writeDoubleSeq(const ::std::vector< ::Ice::Double >& v)
    {
        internalWriteDoubleSeq(v);
    }
    
    ICE_DEPRECATED_API virtual void writeDoubleSeq(const Ice::Double* begin, const Ice::Double* end)
    {
        internalWriteDoubleSeq(begin, end);
    }

    ICE_DEPRECATED_API virtual void writeString(const ::std::string& v, bool convert = true)
    {
        internalWriteString(v, convert);
    }
    
    ICE_DEPRECATED_API virtual void writeStringSeq(const ::std::vector< ::std::string >& v, bool convert = true)
    {
        internalWriteStringSeq(v, convert);
    }

    ICE_DEPRECATED_API virtual void writeWstring(const ::std::wstring& v)
    {
        internalWriteWstring(v);
    }
    
    ICE_DEPRECATED_API virtual void writeWstringSeq(const ::std::vector< ::std::wstring >& v)
    {
        internalWriteWstringSeq(v);
    }

    virtual void writeSize(::Ice::Int) = 0;

    ICE_DEPRECATED_API virtual void writeProxy(const ::Ice::ObjectPrx& v)
    {
        internalWriteProxy(v);
    }

    virtual void writeObject(const ::Ice::ObjectPtr& v) = 0;

    virtual void writeTypeId(const ::std::string&) = 0;

    virtual void writeException(const ::Ice::UserException& v) = 0;

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
        internalWriteBool(v);
    }

    inline  void
    write(::Ice::Byte v)
    {
        internalWriteByte(v);
    }

    inline  void
    write(::Ice::Short v)
    {
        internalWriteShort(v);
    }

    inline  void
    write(::Ice::Int v)
    {
        internalWriteInt(v);
    }

    inline  void
    write(::Ice::Long v)
    {
        internalWriteLong(v);
    }

    inline  void
    write(::Ice::Float v)
    {
        internalWriteFloat(v);
    }

    inline  void
    write(::Ice::Double v)
    {
        internalWriteDouble(v);
    }

    inline  void
    write(const std::string& v)
    {
        internalWriteString(v);
    }
    
    inline  void
    write(const char* v)
    {
        internalWriteString(v);
    }

    inline  void
    write(const std::wstring& v)
    {
        internalWriteWstring(v);
    }

    template<typename T> inline void
    write(const ::IceInternal::ProxyHandle<T>& v)
    {
        internalWriteProxy(v);
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

protected:

    virtual void internalWriteBool(bool) = 0;
    virtual void internalWriteByte(::Ice::Byte) = 0;
    virtual void internalWriteShort(::Ice::Short) = 0;
    virtual void internalWriteInt(::Ice::Int) = 0;
    virtual void internalWriteLong(::Ice::Long) = 0;
    virtual void internalWriteFloat(::Ice::Float) = 0;
    virtual void internalWriteDouble(::Ice::Double) = 0;
    virtual void internalWriteString(const ::std::string&, bool = true) = 0;
    virtual void internalWriteWstring(const ::std::wstring&) = 0;
    virtual void internalWriteProxy(const ::Ice::ObjectPrx&) = 0;
    
    //
    // Remove these methods when the old Stream api, is removed.
    //
    virtual void internalWriteBoolSeq(const std::vector< bool >&) = 0;
    virtual void internalWriteBoolSeq(const bool*, const bool*) = 0;

    virtual void internalWriteByteSeq(const std::vector< Ice::Byte >&) = 0;
    virtual void internalWriteByteSeq(const Ice::Byte*, const Ice::Byte*) = 0;

    virtual void internalWriteShortSeq(const std::vector< Ice::Short >&) = 0;
    virtual void internalWriteShortSeq(const Ice::Short*, const Ice::Short*) = 0;

    virtual void internalWriteIntSeq(const std::vector< Ice::Int >&) = 0;
    virtual void internalWriteIntSeq(const Ice::Int*, const Ice::Int*) = 0;

    virtual void internalWriteLongSeq(const std::vector< Ice::Long >&) = 0;
    virtual void internalWriteLongSeq(const Ice::Long*, const Ice::Long*) = 0;

    virtual void internalWriteFloatSeq(const std::vector< Ice::Float >&) = 0;
    virtual void internalWriteFloatSeq(const Ice::Float*, const Ice::Float*) = 0;

    virtual void internalWriteDoubleSeq(const std::vector< Ice::Double >&) = 0;
    virtual void internalWriteDoubleSeq(const Ice::Double*, const Ice::Double*) = 0;

    virtual void internalWriteStringSeq(const std::vector< std::string >&, bool = true) = 0;

    virtual void internalWriteWstringSeq(const std::vector< std::wstring >&) = 0;
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
        outS->write(static_cast< ::Ice::Byte>(v));
    }
};

template<> // StreamReader specialization for byte enums
struct StreamReader<StreamTraitTypeByteEnum>
{
    template<typename T>
    static void read(const ::Ice::InputStreamPtr& inS, T& v)
    {
        ::Ice::Byte val;
        inS->read(val);
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
        outS->write(static_cast< ::Ice::Short>(v));
    }
};

template<> // StreamReader specialization for short enums
struct StreamReader<StreamTraitTypeShortEnum>
{
    template<typename T>
    static void read(const ::Ice::InputStreamPtr& inS, T& v)
    {
        ::Ice::Short val;
        inS->read(val);
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
        outS->write(static_cast< ::Ice::Int>(v));
    }
};

template<> // StreamReader specialization for int enums
struct StreamReader<StreamTraitTypeIntEnum>
{
    template<typename T>
    static void read(const ::Ice::InputStreamPtr& inS, T& v)
    {
        ::Ice::Int val;
        inS->read(val);
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

//
// BCC2010 compiler bugs doesn't allow use full specialization over partial specialization.
//
#   ifndef __BCPLUSPLUS__

template<> // StreamWriter specialization for sequences of bool
struct StreamWriter<StreamTraitTypeSequenceBool>
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

template<> // Reader specialization for sequences of bool
struct StreamReader<StreamTraitTypeSequenceBool>
{
    template<typename T>
    static void read(const ::Ice::InputStreamPtr& inS, T& v)
    {
        ::Ice::Int sz = inS->readAndCheckSeqSize(::Ice::StreamTrait<bool>::minWireSize);
        v.resize(sz);
        for(int i = 0; i < sz; ++i)
        {
            bool b;
            inS->read(b);
            v[i] = b;
        }
    }
};

#   endif

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
