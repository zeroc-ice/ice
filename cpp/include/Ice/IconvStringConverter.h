// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ICONV_STRING_CONVERTER
#define ICE_ICONV_STRING_CONVERTER

#include <Ice/StringConverter.h>
#include <Ice/UndefSysMacros.h>

#include <algorithm>
#include <iconv.h>

#ifndef _WIN32
#include <langinfo.h>
#endif

#if (defined(__APPLE__) && _LIBICONV_VERSION < 0x010B) || defined(__FreeBSD__)
    //
    // See http://sourceware.org/bugzilla/show_bug.cgi?id=2962
    //
#   define ICE_CONST_ICONV_INBUF 1
#endif

//
// On Windows, we need to be very careful with errno: if we use different C 
// runtime libraries for the main program and the libiconv DLL, we end up with
// two different errnos ... a not-so-good work-around is to ignore errno 
// altogether, by defining ICE_NO_ERRNO
//

namespace Ice
{

//
// Converts charT encoded with internalCode to and from UTF-8 byte sequences
//
// The implementation allocates a pair of iconv_t on each thread, to avoid
// opening / closing iconv_t objects all the time.
//
//
template<typename charT>
class IconvStringConverter : public Ice::BasicStringConverter<charT>
{
public:

#ifdef _WIN32
    IconvStringConverter(const char*);
#else
    IconvStringConverter(const char* = nl_langinfo(CODESET));
#endif

    virtual ~IconvStringConverter();

    virtual Ice::Byte* toUTF8(const charT*, const charT*, Ice::UTF8Buffer&) const;
    
    virtual void fromUTF8(const Ice::Byte*, const Ice::Byte*, std::basic_string<charT>&) const;
    
private:

    std::pair<iconv_t, iconv_t> createDescriptors() const;
    std::pair<iconv_t, iconv_t> getDescriptors() const;

    static void cleanupKey(void*);
    static void close(std::pair<iconv_t, iconv_t>);

#ifdef _WIN32
    DWORD _key;
#else    
    mutable pthread_key_t _key;
#endif
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
IconvStringConverter<charT>::IconvStringConverter(const char* internalCode) :
    _internalCode(internalCode)
{
    //
    // Verify that iconv supports conversion to/from internalCode
    //
    try
    {
        close(createDescriptors());
    }
    catch(const Ice::StringConversionException& sce)
    {
        throw Ice::InitializationException(__FILE__, __LINE__, sce.reason);
    }

    //
    // Create thread-specific key
    //
#ifdef _WIN32
    _key = TlsAlloc();
    if(_key == TLS_OUT_OF_INDEXES)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
#else
    #ifdef __SUNPRO_CC
    int rs = pthread_key_create(&_key, reinterpret_cast<IcePthreadKeyDestructor>(&cleanupKey));
    #else
    int rs = pthread_key_create(&_key, &cleanupKey);
    #endif

    if(rs != 0)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, rs);
    }
#endif
}

template<typename charT>
IconvStringConverter<charT>::~IconvStringConverter()
{
#ifdef _WIN32
    void* val = TlsGetValue(_key);
    if(val != 0)
    {
        cleanupKey(val);
    }
    if(TlsFree(_key) == 0)
    {
        assert(0);
    }
#else
    void* val = pthread_getspecific(_key);
    if(val != 0)
    {
        cleanupKey(val);
    }
    if(pthread_key_delete(_key) != 0)
    {
        assert(0);
    }
#endif
}

template<typename charT> std::pair<iconv_t, iconv_t>
IconvStringConverter<charT>::createDescriptors() const
{
    std::pair<iconv_t, iconv_t> cdp;

    const char* externalCode = "UTF-8";

    cdp.first = iconv_open(_internalCode.c_str(), externalCode);
    if(cdp.first == iconv_t(-1))
    {
        throw Ice::StringConversionException(
            __FILE__, __LINE__,
            std::string("iconv cannot convert from ") 
            + externalCode + " to " + _internalCode);                      
    }
    
    cdp.second = iconv_open(externalCode, _internalCode.c_str());
    if(cdp.second == iconv_t(-1))
    {
        iconv_close(cdp.first);

        throw Ice::StringConversionException(
            __FILE__, __LINE__,
            std::string("iconv cannot convert from ") + _internalCode + " to " + externalCode);                    
    }
    return cdp;
}

template<typename charT> std::pair<iconv_t, iconv_t>
IconvStringConverter<charT>::getDescriptors() const
{
#ifdef _WIN32
    void* val = TlsGetValue(_key);
#else
    void* val = pthread_getspecific(_key);
#endif
    if(val != 0)
    {
        return *static_cast<std::pair<iconv_t, iconv_t>*>(val);
    }
    else
    {
        std::pair<iconv_t, iconv_t> cdp = createDescriptors();
        
#ifdef _WIN32
        if(TlsSetValue(_key, new std::pair<iconv_t, iconv_t>(cdp)) == 0)
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
        }
#else
        int rs = pthread_setspecific(_key, new std::pair<iconv_t, iconv_t>(cdp));
        if(rs != 0)
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, rs);
        }
#endif
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
IconvStringConverter<charT>::toUTF8(const charT* sourceStart, const charT* sourceEnd, Ice::UTF8Buffer& buf) const
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
        outbuf = reinterpret_cast<char*>(buf.getMoreBytes(howMany, reinterpret_cast<Ice::Byte*>(outbuf)));
        count = iconv(cd, &inbuf, &inbytesleft, &outbuf, &howMany);
#ifdef ICE_NO_ERRNO
    } while(count == size_t(-1));
#else
    } while(count == size_t(-1) && errno == E2BIG);
#endif

    if(count == size_t(-1))
    {
       std::string msg = "Unknown error";
#ifndef ICE_NO_ERRNO
        if(errno != 0)
        {
            msg = strerror(errno);
        }
#endif
        throw Ice::StringConversionException(__FILE__, __LINE__, msg);
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

    //
    // Result buffer
    //
    char* buf = 0;
    size_t bufsize = 0;

    char* outbuf = 0;
    size_t outbytesleft = 0;

    size_t count = 0;

    //
    // Loop while we need more buffer space
    //
    do
    {
        size_t increment = std::max(inbytesleft * sizeof(wchar_t), size_t(8));
        bufsize += increment;   
                                    
        char* newbuf = static_cast<char*>(realloc(buf, bufsize));

        if(newbuf == 0)
        {
            free(buf);
            throw Ice::StringConversionException(
                __FILE__, __LINE__, "Out of memory");
        }

        outbuf = newbuf + (outbuf - buf);
        outbytesleft += increment;

        buf = newbuf;
        
        count = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
#ifdef ICE_NO_ERRNO
    } while(count == size_t(-1));
#else
    } while(count == size_t(-1) && errno == E2BIG);
#endif

    if(count == size_t(-1))
    {
        std::string msg = "Unknown error";
#ifndef ICE_NO_ERRNO
        if(errno != 0)
        {
            msg = strerror(errno);
        }
#endif
        free(buf);
        throw Ice::StringConversionException(__FILE__, __LINE__, msg);
    }
    
    size_t length = (bufsize - outbytesleft) / sizeof(charT);
    
    std::basic_string<charT> result(reinterpret_cast<charT*>(buf), length);
    target.swap(result);
    free(buf);
}

}

#endif
