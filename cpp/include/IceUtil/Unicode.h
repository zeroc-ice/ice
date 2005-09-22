// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_UNICODE_H
#define ICE_UTIL_UNICODE_H

#include <IceUtil/Config.h>

namespace IceUtil
{

#if defined(_MSC_VER) && (_MSC_VER >= 1300)

//
// With Visual C++ 7.x, wchar_t is either a typedef unsigned short or a 
// native type (when /Zc:wchar_t is used).
// Since wstring is a typedef basic_string<wchar_t>, its type is also 
// different depending on whether /Zc:wchar_t is used or not.
//
// With Visual C++ 7.x, the default is typedef; with Visual C++ 8.0,
// the default is native type.
//
// Ice is always built with the default, but provides wstringToString() 
// and stringToWstring() implementations for both flavors of wstring.
//

#   if defined(_NATIVE_WCHAR_T_DEFINED)
ICE_UTIL_API std::string wstringToString(const std::wstring&);

#      if _MSC_VER >= 1400
//
// Building or using with VC8
//
ICE_UTIL_API std::wstring stringToWstring(const std::string&);
ICE_UTIL_API std::string wstringToString(const std::basic_string<unsigned short>&);
ICE_UTIL_API std::basic_string<unsigned short> stringToTypedefWstring(const std::string&);
#     else
//
// Using a VC7.x build with the non-default /Zc
//
ICE_UTIL_API std::wstring stringToNativeWstring(const std::string&);
inline std::wstring 
stringToWstring(const std::string& str)
{
    return stringToNativeWstring(str);
}
#     endif

#   else
ICE_UTIL_API std::string wstringToString(const std::wstring&);

#      if _MSC_VER < 1400
//
// Building or using with VC7.x
//
ICE_UTIL_API std::wstring stringToWstring(const std::string&);
ICE_UTIL_API std::string wstringToString(const std::basic_string<__wchar_t>&);
ICE_UTIL_API std::basic_string<__wchar_t> stringToNativeWstring(const std::string&);
#      else
//
// Using a VC8.x build the non-default /Zc
//
ICE_UTIL_API std::wstring stringToTypedefWstring(const std::string&);
inline std::wstring 
stringToWstring(const std::string& str)
{
    return stringToTypedefWstring(str);
}
#      endif
#   endif

#else

ICE_UTIL_API std::string wstringToString(const std::wstring&);
ICE_UTIL_API std::wstring stringToWstring(const std::string&);

#endif

}

#endif
