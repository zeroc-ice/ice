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

/** Encapsulates bytes in the UTF-8 encoding. */
typedef IceUtil::UTF8Buffer UTF8Buffer;

/** Narrow string converter. */
typedef IceUtil::StringConverter StringConverter;
typedef IceUtil::StringConverterPtr StringConverterPtr;

/** Wide string converter. */
typedef IceUtil::WstringConverter WstringConverter;
typedef IceUtil::WstringConverterPtr WstringConverterPtr;

/** Indicates an error occurred during string conversion. */
typedef IceUtil::IllegalConversionException IllegalConversionException;

#ifdef ICE_CPP11_MAPPING
/** Base class for a string converter. */
template<typename charT>
using BasicStringConverter = IceUtil::BasicStringConverter<charT>;
#endif

#ifdef _WIN32
/**
 * Creates a string converter that converts between multi-byte and UTF-8 strings and uses MultiByteToWideChar
 * and WideCharToMultiByte for its implementation.
 */
using IceUtil::createWindowsStringConverter;
#endif

/** Creates a string converter that converts between Unicode wide strings and UTF-8 strings. */
using IceUtil::createUnicodeWstringConverter;

/** Installs a default narrow string converter for the process. */
using IceUtil::setProcessStringConverter;

/** Obtains the default narrow string converter for the process. */
using IceUtil::getProcessStringConverter;

/** Installs a default wide string converter for the process. */
using IceUtil::setProcessWstringConverter;

/** Obtains the default wide string converter for the process. */
using IceUtil::getProcessWstringConverter;

/** Converts a wide string to a narrow string. */
using IceUtil::wstringToString;

/** Converts a narrow string to a wide string. */
using IceUtil::stringToWstring;

/**
 * Converts a narrow string to a UTF-8 encoded string using a string converter.
 * No conversion is performed if the string converter is nil.
 */
using IceUtil::nativeToUTF8;

/**
 * Converts a UTF-8 encoded string to a narrow string using a string converter.
 * No conversion is performed if the string converter is nil.
 */
using IceUtil::UTF8ToNative;

}

#endif
