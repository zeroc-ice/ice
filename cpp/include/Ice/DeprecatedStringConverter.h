// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <Ice/Plugin.h>

namespace Ice
{

typedef IceUtil::StringConverterPtr StringConverterPtr;
typedef IceUtil::WstringConverterPtr WstringConverterPtr;

//
// A plug-in that sets the process string converter and wide string in its constructor.
// Both initialize and destroy are no-op.
// It's deprecated because if you want to install your own string converters with a
// plugin, you should simply implement your own Plugin class, derived directly from
// Ice::Plugin.
//
class ICE_API StringConverterPlugin : public Plugin
{
public:

    ICE_DEPRECATED_API("StringConverterPlugin is deprecated, use Ice::Plugin instead")
    StringConverterPlugin(const CommunicatorPtr&, 
                          const StringConverterPtr&, const WstringConverterPtr& = 0);
    
    virtual void initialize();
    
    virtual void destroy();
};

ICE_DEPRECATED_API("is deprecated, use IceUtil::nativeToUTF8(const std::string&, const StringConverterPtr&) instead") 
ICE_API std::string 
nativeToUTF8(const StringConverterPtr&, const std::string&);

ICE_DEPRECATED_API("is deprecated, use IceUtil::nativeToUTF8(const std::string&, const StringConverterPtr&) instead") 
ICE_API std::string 
nativeToUTF8(const CommunicatorPtr&, const std::string&);

ICE_DEPRECATED_API("is deprecated, use IceUtil::UTF8ToNative(const std::string&, const StringConverterPtr&) instead") 
ICE_API std::string
UTF8toNative(const StringConverterPtr&, const std::string&);

ICE_DEPRECATED_API("is deprecated, use IceUtil::UTF8ToNative(const std::string&, const StringConverterPtr&) instead") 
ICE_API std::string
UTF8ToNative(const CommunicatorPtr&, const std::string&);

}

#endif
