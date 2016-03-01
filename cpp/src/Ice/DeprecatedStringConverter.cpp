// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/DeprecatedStringConverter.h>

#include <Ice/Initialize.h>
#include <Ice/Instance.h>
#include <IceUtil/StringConverter.h>

namespace Ice
{

StringConverterPlugin::StringConverterPlugin(const CommunicatorPtr& /*notused*/, 
                                             const StringConverterPtr& stringConverter, 
                                             const WstringConverterPtr& wstringConverter) 
{
    IceUtil::setProcessStringConverter(stringConverter);
    IceUtil::setProcessWstringConverter(wstringConverter);
}
    
void StringConverterPlugin::initialize()
{
    // no op
}
    
void StringConverterPlugin::destroy()
{
    // no op
}

std::string
nativeToUTF8(const StringConverterPtr& converter, const std::string& s)
{
    return IceUtil::nativeToUTF8(s, converter);
}

std::string
nativeToUTF8(const CommunicatorPtr& communicator, const std::string& s)
{
    return IceUtil::nativeToUTF8(s, IceInternal::getInstance(communicator)->getStringConverter());
}

std::string
UTF8toNative(const StringConverterPtr& converter, const std::string& s)
{
    return IceUtil::UTF8ToNative(s, converter);
}

std::string
UTF8ToNative(const CommunicatorPtr& communicator, const std::string& s)
{
    return IceUtil::UTF8ToNative(s, IceInternal::getInstance(communicator)->getStringConverter());
}

}
