// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_DEPRECATED_STRING_CONVERTER_H
#define ICE_DEPRECATED_STRING_CONVERTER_H

#include <Ice/Config.h>
#include <Ice/CommunicatorF.h>
#include <IceUtil/StringConverter.h>

namespace Ice
{

typedef IceUtil::StringConverterPtr StringConverterPtr;
typedef IceUtil::WStringConverterPtr WStringConverterPtr;


ICE_DEPRECATED_API string
nativeToUTF8(const StringConverterPtr&, const std::string&);

ICE_DEPRECATED_API string
nativeToUTF8(const CommunicatorPtr&, const std::string&);

ICE_DEPRECATED_API string
UTF8toNative(const StringConverterPtr&, const std::string&);

ICE_DEPRECATED_API string
UTF8ToNative(const CommunicatorPtr&, const std::string&);

}

#endif
