// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/DeprecatedStringConverter.h>
#include <Ice/Initialize.h>
#include <Ice/Instance.h>

std::string
Ice::nativeToUTF8(const Ice::StringConverterPtr& converter, const std::string& s)
{
    return IceUtil::nativeToUTF8(s, converter);
}

std::string
Ice::nativeToUTF8(const Ice::CommunicatorPtr& communicator, const std::string& s)
{
    return IceUtil::nativeToUTF8(s, IceInternal::getInstance(communicator)->getStringConverter());
}

std::string
Ice::UTF8toNative(const Ice::StringConverterPtr& converter, const std::string& s)
{
    return IceUtil::UTF8ToNative(s, converter);
}

std::string
Ice::UTF8ToNative(const Ice::CommunicatorPtr& communicator, const std::string& s)
{
    return IceUtil::UTF8ToNative(s, IceInternal::getInstance(communicator)->getStringConverter());
}
