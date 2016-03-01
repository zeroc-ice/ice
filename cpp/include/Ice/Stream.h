// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <Ice/StreamHelpers.h>

namespace Ice
{

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
            IceInternal::Ex::throwUOE(T::ice_staticId(), p);
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

    virtual ::std::string ice_name() const = 0;
    virtual UserException* ice_clone() const = 0;
    virtual void ice_throw() const = 0;

    virtual void __write(IceInternal::BasicStream*) const;
    virtual void __read(IceInternal::BasicStream*);

    virtual bool __usesClasses() const;

    using UserException::__read;
    using UserException::__write;

protected:

    virtual void __writeImpl(::IceInternal::BasicStream*) const;
    virtual void __readImpl(::IceInternal::BasicStream*);

    using UserException::__writeImpl;
    using UserException::__readImpl;

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

    typedef size_t size_type;

    virtual CommunicatorPtr communicator() const = 0;

    virtual void sliceObjects(bool) = 0;

    virtual Int readSize() = 0;
    virtual Int readAndCheckSeqSize(int) = 0;

    virtual ObjectPrx readProxy() = 0;
    template<typename T> void read(IceInternal::ProxyHandle<T>& v)
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

    virtual void readObject(const ReadObjectCallbackPtr&) = 0;
    template<typename T> void read(IceInternal::Handle<T>& v)
    {
        readObject(new ReadObjectCallbackI<T>(v));
    }

    virtual Int readEnum(Int maxValue)
    {
        if(getEncoding() == Encoding_1_0)
        {
            if(maxValue < 127)
            {
                Byte value;
                read(value);
                return value;
            }
            else if(maxValue < 32767)
            {
                Short value;
                read(value);
                return value;
            }
            else
            {
                Int value;
                read(value);
                return value;
            }
        }
        else
        {
            return readSize();
        }
    }

    virtual void throwException() = 0;
    virtual void throwException(const UserExceptionReaderFactoryPtr&) = 0;

    virtual void startObject() = 0;
    virtual SlicedDataPtr endObject(bool) = 0;

    virtual void startException() = 0;
    virtual SlicedDataPtr endException(bool) = 0;

    virtual std::string startSlice() = 0;
    virtual void endSlice() = 0;
    virtual void skipSlice() = 0;

    virtual EncodingVersion startEncapsulation() = 0;
    virtual void endEncapsulation() = 0;
    virtual EncodingVersion skipEncapsulation() = 0;

    virtual EncodingVersion getEncoding() const = 0;

    virtual void readPendingObjects() = 0;

    virtual size_type pos() = 0;
    virtual void rewind() = 0;

    virtual void skip(Int) = 0;
    virtual void skipSize() = 0;

    virtual void read(bool&) = 0;
    virtual void read(Byte&) = 0;
    virtual void read(Short&) = 0;
    virtual void read(Int&) = 0;
    virtual void read(Long&) = 0;
    virtual void read(Float&) = 0;
    virtual void read(Double&) = 0;
    virtual void read(::std::string&, bool = true) = 0;
    virtual void read(const char*&, size_t&) = 0;
    virtual void read(const char*&, size_t&, std::string&) = 0;
    virtual void read(::std::vector< ::std::string>&, bool) = 0; // Overload required for additional bool argument.
    virtual void read(::std::wstring&) = 0;

    //
    // std::vector<bool> is a special C++ type, so we give it its own read function
    //
    virtual void read(::std::vector<bool>&) = 0;

    virtual void read(::std::pair<const bool*, const bool*>&, ::IceUtil::ScopedArray<bool>&) = 0;
    virtual void read(::std::pair<const Byte*, const Byte*>&) = 0;
    virtual void read(::std::pair<const Short*, const Short*>&, ::IceUtil::ScopedArray<Short>&) = 0;
    virtual void read(::std::pair<const Int*, const Int*>&, ::IceUtil::ScopedArray<Int>&) = 0;
    virtual void read(::std::pair<const Long*, const Long*>&, ::IceUtil::ScopedArray<Long>&) = 0;
    virtual void read(::std::pair<const Float*, const Float*>&, ::IceUtil::ScopedArray<Float>&) = 0;
    virtual void read(::std::pair<const Double*, const Double*>&, ::IceUtil::ScopedArray<Double>&) = 0;

    // This method is useful for generic stream helpers
    void read(::std::pair<const Byte*, const Byte*>& p, ::IceUtil::ScopedArray<Byte>& result)
    {
        result.reset();
        read(p);
    }

    virtual bool readOptional(Int, OptionalFormat) = 0;

    template<typename T> inline void read(T& v)
    {
        StreamHelper<T, StreamableTraits<T>::helper>::read(this, v);
    }

    template<typename T> inline void read(Int tag, IceUtil::Optional<T>& v)
    {
        if(readOptional(tag, StreamOptionalHelper<T,
                                                  StreamableTraits<T>::helper,
                                                  StreamableTraits<T>::fixedLength>::optionalFormat))
        {
            v.__setIsSet();
            StreamOptionalHelper<T, StreamableTraits<T>::helper, StreamableTraits<T>::fixedLength>::read(this, *v);
        }
        else
        {
            v = IceUtil::None;
        }
    }

    virtual void closure(void*) = 0;
    virtual void* closure() const = 0;
};

class ICE_API OutputStream : public ::IceUtil::Shared
{
public:

    typedef size_t size_type;

    virtual CommunicatorPtr communicator() const = 0;

    virtual void writeSize(Int) = 0;

    virtual void writeProxy(const ObjectPrx&) = 0;
    template<typename T> void write(const IceInternal::ProxyHandle<T>& v)
    {
        writeProxy(ObjectPrx(v.get()));
    }

    virtual void writeObject(const ObjectPtr&) = 0;
    template<typename T> void write(const IceInternal::Handle<T>& v)
    {
        writeObject(ObjectPtr(v.get()));
    }

    virtual void writeEnum(Int v, Int maxValue)
    {
        if(getEncoding() == Encoding_1_0)
        {
            if(maxValue < 127)
            {
                write(static_cast<Byte>(v));
            }
            else if(maxValue < 32767)
            {
                write(static_cast<Short>(v));
            }
            else
            {
                write(v);
            }
        }
        else
        {
            writeSize(v);
        }
    }

    virtual void writeException(const UserException&) = 0;

    virtual void startObject(const SlicedDataPtr&) = 0;
    virtual void endObject() = 0;

    virtual void startException(const SlicedDataPtr&) = 0;
    virtual void endException() = 0;

    virtual void startSlice(const ::std::string&, int, bool) = 0;
    virtual void endSlice() = 0;

    virtual void startEncapsulation(const EncodingVersion&, FormatType) = 0;
    virtual void startEncapsulation() = 0;
    virtual void endEncapsulation() = 0;

    virtual EncodingVersion getEncoding() const = 0;

    virtual void writePendingObjects() = 0;

    virtual void finished(::std::vector<Byte>&) = 0;
    virtual std::pair<const Byte*, const Byte*> finished() = 0;

    virtual size_type pos() = 0;
    virtual void rewrite(Int, size_type) = 0;

    virtual void reset(bool) = 0;

    virtual void write(bool) = 0;
    virtual void write(Byte) = 0;
    virtual void write(Short) = 0;
    virtual void write(Int) = 0;
    virtual void write(Long) = 0;
    virtual void write(Float) = 0;
    virtual void write(Double) = 0;
    virtual void write(const ::std::string&, bool = true) = 0;
    virtual void write(const char*, size_t, bool = true) = 0;
    virtual void write(const ::std::vector< ::std::string>&, bool) = 0; // Overload required for bool argument.
    virtual void write(const char*, bool = true) = 0;
    virtual void write(const ::std::wstring&) = 0;

    //
    // std::vector<bool> is a special C++ type, so we give it its own write function
    //
    virtual void write(const ::std::vector<bool>&) = 0;

    virtual void write(const bool*, const bool*) = 0;
    virtual void write(const Byte*, const Byte*) = 0;
    virtual void write(const Short*, const Short*) = 0;
    virtual void write(const Int*, const Int*) = 0;
    virtual void write(const Long*, const Long*) = 0;
    virtual void write(const Float*, const Float*) = 0;
    virtual void write(const Double*, const Double*) = 0;

    virtual bool writeOptional(Int, OptionalFormat) = 0;

    virtual size_type startSize() = 0;
    virtual void endSize(size_type pos) = 0;

    template<typename T> inline void write(const T& v)
    {
        StreamHelper<T, StreamableTraits<T>::helper>::write(this, v);
    }

    template<typename T> inline void write(Int tag, const IceUtil::Optional<T>& v)
    {
        if(v)
        {
            writeOptional(tag, StreamOptionalHelper<T,
                                                    StreamableTraits<T>::helper,
                                                    StreamableTraits<T>::fixedLength>::optionalFormat);
            StreamOptionalHelper<T, StreamableTraits<T>::helper, StreamableTraits<T>::fixedLength>::write(this, *v);
        }
    }

    //
    // Template functions for sequences and custom sequences
    //
    template<typename T> void write(const T* begin, const T* end)
    {
        writeSize(static_cast<Int>(end - begin));
        for(const T* p = begin; p != end; ++p)
        {
            write(*p);
        }
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
    virtual UserException* ice_clone() const = 0;
    virtual void ice_throw() const = 0;

    virtual void __write(IceInternal::BasicStream*) const;
    virtual void __read(IceInternal::BasicStream*);

    virtual bool __usesClasses() const;

    using UserException::__read;
    using UserException::__write;

protected:

    virtual void __writeImpl(::IceInternal::BasicStream*) const;
    virtual void __readImpl(::IceInternal::BasicStream*);

    using UserException::__writeImpl;
    using UserException::__readImpl;

    const CommunicatorPtr _communicator;
};

}

#endif
