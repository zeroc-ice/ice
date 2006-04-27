// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STRING_CONVERTER_H
#define ICE_STRING_CONVERTER_H

#include <Ice/Config.h>
#include <IceUtil/Exception.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>

#include <string>

namespace Ice
{

//
// Provides bytes to toUTF8. Raises MemoryLimitException when too many
// bytes are requested.
//
class ICE_API UTF8Buffer
{
public:
    virtual Byte* getMoreBytes(size_t howMany, Byte* firstUnused) = 0;
    
    virtual ~UTF8Buffer() {}
};

//
// A StringConverter converts narrow or wide-strings to and from UTF-8 byte sequences.
// It's used by the communicator during marshaling (toUTF8) and unmarshaling (fromUTF8).
// It report errors by raising StringConversionFailed or MemoryLimitException.
//
template<typename charT>
class BasicStringConverter : public IceUtil::Shared
{
public:
    
    //
    // Returns a pointer to byte after the last written byte (which may be
    // past the last byte returned by getMoreBytes).
    //
    virtual Byte* toUTF8(const charT* sourceStart, const charT* sourceEnd,
			 UTF8Buffer&) const = 0;

    //
    // This fromUTF8 function allocates the result (targetStart and targetEnd are
    // out parameters); when it succeeds, the caller is responsible to free the 
    // allocated target with freeTarget.
    // This way, an implementation of fromUTF8 using iconv() can use a single iconv_t
    // even when it discovers during conversion that it needs a larger target buffer.
    //
    virtual void fromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
			  const charT*& targetStart, const charT*& targetEnd) const = 0;

    virtual void freeTarget(const charT* targetStart) const = 0;


    //
    // You may want to override this fromUTF8 function to provide a more efficient 
    // implementation, without a temporary charT buffer.
    //
    virtual void fromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
			  std::basic_string<charT>& target) const
    {
	const charT* targetStart = 0;
	const charT* targetEnd = 0;
	fromUTF8(sourceStart, sourceEnd, targetStart, targetEnd);
	std::basic_string<charT> s(targetStart, static_cast<size_t>(targetEnd - targetStart));
	freeTarget(targetStart);
	s.swap(target);
    } 
};

typedef BasicStringConverter<char> StringConverter;
typedef IceUtil::Handle<StringConverter> StringConverterPtr;

typedef BasicStringConverter<wchar_t> WstringConverter;
typedef IceUtil::Handle<WstringConverter> WstringConverterPtr;


//
// Converts to and from UTF-16 or UTF-32 depending on sizeof(wchar_t)
//
class ICE_API UnicodeWstringConverter : public WstringConverter
{
public:
    
    virtual Byte* toUTF8(const wchar_t* sourceStart, const wchar_t* sourceEnd,
			 UTF8Buffer&) const;

    virtual void fromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
			  const wchar_t*& targetStart, const wchar_t*& targetEnd) const;

    virtual void freeTarget(const wchar_t* targetStart) const;

    virtual void fromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
			  std::wstring& target) const;
};
}
#endif
