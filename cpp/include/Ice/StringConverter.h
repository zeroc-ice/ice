// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STRING_CONVERTER_H
#define ICE_STRING_CONVERTER_H

#include <Ice/Config.h>
#include <IceUtil/StringConverter.h>
#include <IceUtil/ConsoleUtil.h>

namespace Ice
{

typedef IceUtil::UTF8Buffer UTF8Buffer;
typedef IceUtil::StringConverter StringConverter;
typedef IceUtil::StringConverterPtr StringConverterPtr;
typedef IceUtil::WstringConverter WstringConverter;
typedef IceUtil::WstringConverterPtr WstringConverterPtr;

typedef IceUtil::IllegalConversionException IllegalConversionException;

#ifdef ICE_CPP11_MAPPING
template<typename charT>
using BasicStringConverter = IceUtil::BasicStringConverter<charT>;
#endif

#ifdef _WIN32
using IceUtil::createWindowsStringConverter;
#endif

using IceUtil::createUnicodeWstringConverter;

using IceUtil::setProcessStringConverter;
using IceUtil::getProcessStringConverter;
using IceUtil::setProcessWstringConverter;
using IceUtil::getProcessWstringConverter;

using IceUtil::wstringToString;
using IceUtil::stringToWstring;

using IceUtil::nativeToUTF8;
using IceUtil::UTF8ToNative;

}

#endif
