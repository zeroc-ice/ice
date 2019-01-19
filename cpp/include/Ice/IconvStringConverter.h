//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ICONV_STRING_CONVERTER
#define ICE_ICONV_STRING_CONVERTER

#include <Ice/Config.h>

//
// For all platforms except Windows
//
#ifndef _WIN32

#include <Ice/StringConverter.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/ThreadException.h>
#include <IceUtil/UndefSysMacros.h>

#include <algorithm>
#include <iconv.h>
#include <langinfo.h>

#if (defined(__APPLE__) && _LIBICONV_VERSION < 0x010B)
//
// See http://sourceware.org/bugzilla/show_bug.cgi?id=2962
//
#   define ICE_CONST_ICONV_INBUF 1
#endif

namespace Ice
{

/**
 * Indicates that Iconv does not support the code.
 * \headerfile Ice/Ice.h
 */
class ICE_API IconvInitializationException : public IceUtil::ExceptionHelper<IconvInitializationException>
{
public:

    /**
     * Constructs the exception with a reason. The file and line number are required.
     * @param file The file name in which the exception was raised, typically __FILE__.
     * @param line The line number at which the exception was raised, typically __LINE__.
     * @param reason More detail about the failure.
     */
    IconvInitializationException(const char* file, int line, const std::string& reason);

#ifndef ICE_CPP11_COMPILER
    virtual ~IconvInitializationException() throw();
#endif

    /**
     * Obtains the Slice type ID of this exception.
     * @return The fully-scoped type ID.
     */
    virtual std::string ice_id() const;

    /**
     * Prints a description of this exception to the given stream.
     * @param str The output stream.
     */
    virtual void ice_print(std::ostream& str) const;

#ifndef ICE_CPP11_MAPPING
    /**
     * Polymporphically clones this exception.
     * @return A shallow copy of this exception.
     */
    virtual IconvInitializationException* ice_clone() const;
#endif

    /**
     * Obtains the reason for the failure.
     * @return The reason.
     */
    std::string reason() const;

private:

    std::string _reason;
};

}

namespace IceInternal
{

//
// Converts charT encoded with internalCode to and from UTF-8 byte sequences
//
// The implementation allocates a pair of iconv_t on each thread, to avoid
// opening / closing iconv_t objects all the time.
//
//
template<typename charT>
class IconvStringConverter : public IceUtil::BasicStringConverter<charT>
{
public:

    IconvStringConverter(const std::string&);

    virtual ~IconvStringConverter();

    virtual Ice::Byte* toUTF8(const charT*, const charT*, Ice::UTF8Buffer&) const;

    virtual void fromUTF8(const Ice::Byte*, const Ice::Byte*, std::basic_string<charT>&) const;

private:

    std::pair<iconv_t, iconv_t> createDescriptors() const;
    std::pair<iconv_t, iconv_t> getDescriptors() const;

    static void cleanupKey(void*);
    static void close(std::pair<iconv_t, iconv_t>);

    mutable pthread_key_t _key;
    const std::string _internalCode;
};

//
// Implementation
//

#ifdef __SUNPRO_CC
extern "C"
{
    typedef void (*IcePthreadKeyDestructor)(void*);
}
#endif

template<typename charT>
IconvStringConverter<charT>::IconvStringConverter(const std::string& internalCode) :
    _internalCode(internalCode)
{
    //
    // Verify that iconv supports conversion to/from internalCode
    //
    try
    {
        close(createDescriptors());
    }
    catch(const Ice::IllegalConversionException& sce)
    {
        throw Ice::IconvInitializationException(__FILE__, __LINE__, sce.reason());
    }

    //
    // Create thread-specific key
    //
#ifdef __SUNPRO_CC
    int rs = pthread_key_create(&_key, reinterpret_cast<IcePthreadKeyDestructor>(&cleanupKey));
#else
    int rs = pthread_key_create(&_key, &cleanupKey);
#endif

    if(rs != 0)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, rs);
    }
}

template<typename charT>
IconvStringConverter<charT>::~IconvStringConverter()
{
    void* val = pthread_getspecific(_key);
    if(val != 0)
    {
        cleanupKey(val);
    }
    if(pthread_key_delete(_key) != 0)
    {
        assert(0);
    }
}

template<typename charT> std::pair<iconv_t, iconv_t>
IconvStringConverter<charT>::createDescriptors() const
{
    std::pair<iconv_t, iconv_t> cdp;

    const char* externalCode = "UTF-8";

    cdp.first = iconv_open(_internalCode.c_str(), externalCode);
    if(cdp.first == iconv_t(-1))
    {
        std::ostringstream os;
        os << "iconv cannot convert from " << externalCode << " to " << _internalCode;
        throw Ice::IllegalConversionException(__FILE__, __LINE__, os.str());
    }

    cdp.second = iconv_open(externalCode, _internalCode.c_str());
    if(cdp.second == iconv_t(-1))
    {
        iconv_close(cdp.first);
        std::ostringstream os;
        os << "iconv cannot convert from " << _internalCode << " to " << externalCode;
        throw Ice::IllegalConversionException(__FILE__, __LINE__, os.str());
    }
    return cdp;
}

template<typename charT> std::pair<iconv_t, iconv_t>
IconvStringConverter<charT>::getDescriptors() const
{
    void* val = pthread_getspecific(_key);
    if(val != 0)
    {
        return *static_cast<std::pair<iconv_t, iconv_t>*>(val);
    }
    else
    {
        std::pair<iconv_t, iconv_t> cdp = createDescriptors();
        int rs = pthread_setspecific(_key, new std::pair<iconv_t, iconv_t>(cdp));
        if(rs != 0)
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, rs);
        }
        return cdp;
    }
}

template<typename charT> /*static*/ void
IconvStringConverter<charT>::cleanupKey(void* val)
{
    std::pair<iconv_t, iconv_t>* cdp = static_cast<std::pair<iconv_t, iconv_t>*>(val);

    close(*cdp);
    delete cdp;
}

template<typename charT> /*static*/ void
IconvStringConverter<charT>::close(std::pair<iconv_t, iconv_t> cdp)
{
#ifndef NDEBUG
    int rs = iconv_close(cdp.first);
    assert(rs == 0);

    rs = iconv_close(cdp.second);
    assert(rs == 0);
#else
    iconv_close(cdp.first);
    iconv_close(cdp.second);
#endif
}

template<typename charT> Ice::Byte*
IconvStringConverter<charT>::toUTF8(const charT* sourceStart,
                                    const charT* sourceEnd,
                                    Ice::UTF8Buffer& buf) const
{
    iconv_t cd = getDescriptors().second;

    //
    // Reset cd
    //
#ifdef NDEBUG
    iconv(cd, 0, 0, 0, 0);
#else
    size_t rs = iconv(cd, 0, 0, 0, 0);
    assert(rs == 0);
#endif

#ifdef ICE_CONST_ICONV_INBUF
    const char* inbuf = reinterpret_cast<const char*>(sourceStart);
#else
    char* inbuf = reinterpret_cast<char*>(const_cast<charT*>(sourceStart));
#endif
    size_t inbytesleft = (sourceEnd - sourceStart) * sizeof(charT);
    char* outbuf  = 0;

    size_t count = 0;
    //
    // Loop while we need more buffer space
    //
    do
    {
        size_t howMany = std::max(inbytesleft, size_t(4));
        outbuf = reinterpret_cast<char*>(buf.getMoreBytes(howMany,
                                                          reinterpret_cast<Ice::Byte*>(outbuf)));
        count = iconv(cd, &inbuf, &inbytesleft, &outbuf, &howMany);
    } while(count == size_t(-1) && errno == E2BIG);

    if(count == size_t(-1))
    {
        throw Ice::IllegalConversionException(__FILE__, __LINE__,
                                              errno == 0 ? "Unknown error" : IceUtilInternal::errorToString(errno));
    }
    return reinterpret_cast<Ice::Byte*>(outbuf);
}

template<typename charT> void
IconvStringConverter<charT>::fromUTF8(const Ice::Byte* sourceStart, const Ice::Byte* sourceEnd,
                                      std::basic_string<charT>& target) const
{
    iconv_t cd = getDescriptors().first;

    //
    // Reset cd
    //
#ifdef NDEBUG
    iconv(cd, 0, 0, 0, 0);
#else
    size_t rs = iconv(cd, 0, 0, 0, 0);
    assert(rs == 0);
#endif

#ifdef ICE_CONST_ICONV_INBUF
    const char* inbuf = reinterpret_cast<const char*>(sourceStart);
#else
    char* inbuf = reinterpret_cast<char*>(const_cast<Ice::Byte*>(sourceStart));
#endif
    size_t inbytesleft = sourceEnd - sourceStart;

    char* outbuf = 0;
    size_t outbytesleft = 0;
    size_t count = 0;

    //
    // Loop while we need more buffer space
    //
    do
    {
        size_t bytesused = 0;
        if(outbuf != 0)
        {
            bytesused = outbuf - reinterpret_cast<const char*>(target.data());
        }

        const size_t increment = std::max<size_t>(inbytesleft, 4);
        target.resize(target.size() + increment);
        outbuf = const_cast<char*>(reinterpret_cast<const char*>(target.data())) + bytesused;
        outbytesleft += increment * sizeof(charT);

        count = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);

    } while(count == size_t(-1) && errno == E2BIG);

    if(count == size_t(-1))
    {
        throw Ice::IllegalConversionException(__FILE__,  __LINE__,
                                              errno == 0 ? "Unknown error" : IceUtilInternal::errorToString(errno));
    }

    target.resize(target.size() - (outbytesleft / sizeof(charT)));
}
}

namespace Ice
{

/**
 * Creates a string converter for the given code.
 * @param internalCodeWithDefault The desired code. If empty or not provided, a default code is used.
 * @return The converter object.
 * @throws IconvInitializationException If the code is not supported.
 */
template<typename charT>
ICE_HANDLE<IceUtil::BasicStringConverter<charT> >
createIconvStringConverter(const std::string& internalCodeWithDefault = "")
{
    std::string internalCode = internalCodeWithDefault;

    if(internalCode.empty())
    {
        internalCode = nl_langinfo(CODESET);
    }

    return ICE_MAKE_SHARED(IceInternal::IconvStringConverter<charT>, internalCode);
}

}

#endif
#endif
