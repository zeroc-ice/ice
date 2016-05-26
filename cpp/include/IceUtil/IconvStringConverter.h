// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_ICONV_STRING_CONVERTER
#define ICE_UTIL_ICONV_STRING_CONVERTER

#include <IceUtil/StringConverter.h>
#include <IceUtil/UndefSysMacros.h>

#include <algorithm>
#include <iconv.h>
#include <langinfo.h>
#include <string.h> // For strerror

#if (defined(__APPLE__) && _LIBICONV_VERSION < 0x010B)
    //
    // See http://sourceware.org/bugzilla/show_bug.cgi?id=2962
    //
#   define ICE_CONST_ICONV_INBUF 1
#endif

namespace IceUtil
{

//
// Converts charT encoded with internalCode to and from UTF-8 byte sequences
//
// The implementation allocates a pair of iconv_t on each thread, to avoid
// opening / closing iconv_t objects all the time.
//
//
template<typename charT>
class IconvStringConverter : public BasicStringConverter<charT>
{
public:

    IconvStringConverter(const char* = nl_langinfo(CODESET));

    virtual ~IconvStringConverter();

    virtual Byte* toUTF8(const charT*, const charT*, UTF8Buffer&) const;
    
    virtual void fromUTF8(const Byte*, const Byte*, std::basic_string<charT>&) const;
    
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
    catch(const IllegalConversionException& sce)
    {
        throw IconvInitializationException(__FILE__, __LINE__, sce.reason());
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
        throw ThreadSyscallException(__FILE__, __LINE__, rs);
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
        throw IllegalConversionException(__FILE__, __LINE__, os.str());
    }
    
    cdp.second = iconv_open(externalCode, _internalCode.c_str());
    if(cdp.second == iconv_t(-1))
    {
        iconv_close(cdp.first);
        std::ostringstream os;
        os << "iconv cannot convert from " << _internalCode << " to " << externalCode;
        throw IllegalConversionException(__FILE__, __LINE__, os.str());
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
            throw ThreadSyscallException(__FILE__, __LINE__, rs);
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
 
template<typename charT> Byte* 
IconvStringConverter<charT>::toUTF8(const charT* sourceStart, const charT* sourceEnd, UTF8Buffer& buf) const
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
        outbuf = reinterpret_cast<char*>(buf.getMoreBytes(howMany, reinterpret_cast<Byte*>(outbuf)));
        count = iconv(cd, &inbuf, &inbytesleft, &outbuf, &howMany);
    } while(count == size_t(-1) && errno == E2BIG);

    if(count == size_t(-1))
    {
        throw IllegalConversionException(__FILE__, __LINE__, errno != 0 ? strerror(errno) : "Unknown error");
    }
    return reinterpret_cast<Byte*>(outbuf);
}
  
template<typename charT> void
IconvStringConverter<charT>::fromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
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
    char* inbuf = reinterpret_cast<char*>(const_cast<Byte*>(sourceStart));
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
            throw IllegalConversionException(__FILE__, __LINE__, "Out of memory");
        }

        outbuf = newbuf + (outbuf - buf);
        outbytesleft += increment;

        buf = newbuf;
        
        count = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    } while(count == size_t(-1) && errno == E2BIG);

    if(count == size_t(-1))
    {
        free(buf);
        throw IllegalConversionException(__FILE__, __LINE__, errno != 0 ? strerror(errno) : "Unknown error");
    }
    
    size_t length = (bufsize - outbytesleft) / sizeof(charT);
    
    std::basic_string<charT> result(reinterpret_cast<charT*>(buf), length);
    target.swap(result);
    free(buf);
}

}

#endif
