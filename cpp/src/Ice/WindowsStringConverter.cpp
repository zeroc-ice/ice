// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/StringConverter.h>

#ifdef _WIN32

#include <IceUtil/StringUtil.h>


using namespace Ice;
using namespace std;

namespace
{
//
// Converts to/from UTF-8 using MultiByteToWideChar and WideCharToMultiByte
//
class WindowsStringConverter : public StringConverter
{
public:

    explicit WindowsStringConverter(unsigned int);

    virtual Byte* toUTF8(const char*, const char*, UTF8Buffer&) const;

    virtual void fromUTF8(const Byte*, const Byte*, string& target) const;

private:
    unsigned int _cp;
    WstringConverterPtr _unicodeConverter;

};

WindowsStringConverter::WindowsStringConverter(unsigned int cp) :
    _cp(cp),
    _unicodeConverter(createUnicodeWstringConverter())
{
}

Byte*
WindowsStringConverter::toUTF8(const char* sourceStart, const char* sourceEnd, UTF8Buffer& buffer) const
{
    //
    // First convert to UTF-16
    //
    int sourceSize = static_cast<int>(sourceEnd - sourceStart);
    if(sourceSize == 0)
    {
        return buffer.getMoreBytes(1, 0);
    }

    int writtenWchar = 0;
    wstring wbuffer;

    //
    // The following code pages doesn't support MB_ERR_INVALID_CHARS flag
    // see http://msdn.microsoft.com/en-us/library/windows/desktop/dd319072(v=vs.85).aspx
    //
    DWORD flags =
        (_cp == 50220 || _cp == 50221 || _cp == 50222 ||
         _cp == 50225 || _cp == 50227 || _cp == 50229 ||
         _cp == 65000 || _cp == 42 || (_cp >= 57002 && _cp <= 57011)) ? 0 : MB_ERR_INVALID_CHARS;

    do
    {
        wbuffer.resize(wbuffer.size() == 0 ? sourceSize + 2 : 2 * wbuffer.size());
        writtenWchar = MultiByteToWideChar(_cp, flags, sourceStart, sourceSize,
                                           const_cast<wchar_t*>(wbuffer.data()), static_cast<int>(wbuffer.size()));
    } while(writtenWchar == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    if(writtenWchar == 0)
    {
        throw IllegalConversionException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }

    wbuffer.resize(static_cast<size_t>(writtenWchar));

    //
    // Then convert this UTF-16 wbuffer into UTF-8
    //
    return _unicodeConverter->toUTF8(wbuffer.data(), wbuffer.data() + wbuffer.size(), buffer);
}

void
WindowsStringConverter::fromUTF8(const Byte* sourceStart, const Byte* sourceEnd, string& target) const
{
    if(sourceStart == sourceEnd)
    {
        target = "";
        return;
    }

    if(_cp == CP_UTF8)
    {
        string tmp(reinterpret_cast<const char*>(sourceStart), sourceEnd - sourceStart);
        tmp.swap(target);
        return;
    }

    //
    // First convert to wstring (UTF-16)
    //
    wstring wtarget;
    _unicodeConverter->fromUTF8(sourceStart, sourceEnd, wtarget);

    //
    // WC_ERR_INVALID_CHARS conversion flag is only supported with 65001 (UTF-8) and
    // 54936 (GB18030 Simplified Chinese)
    //
    DWORD flags = (_cp == 65001 || _cp == 54936) ? WC_ERR_INVALID_CHARS : 0;

    //
    // And then to a multi-byte narrow string
    //
    int writtenChar = -1;
    do
    {
        target.resize(writtenChar == -1 ?
                      std::max<size_t>(sourceEnd - sourceStart + 2, target.size()) :
                      2 * target.size());

        writtenChar = WideCharToMultiByte(_cp, flags, wtarget.data(), static_cast<int>(wtarget.size()),
                                          const_cast<char*>(target.data()), static_cast<int>(target.size()),
                                          0, 0);
    } while(writtenChar == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    if(writtenChar == 0)
    {
        throw IllegalConversionException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }

    target.resize(static_cast<size_t>(writtenChar));
}
}

StringConverterPtr
Ice::createWindowsStringConverter(unsigned int cp)
{
    return ICE_MAKE_SHARED(WindowsStringConverter, cp);
}
#endif
