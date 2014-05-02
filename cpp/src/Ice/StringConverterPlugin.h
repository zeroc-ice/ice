// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STRING_CONVERTER_PLUGIN_H
#define ICE_STRING_CONVERTER_PLUGIN_H

#include <Ice/Config.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Plugin.h>
#include <IceUtil/StringConverter.h>

#include <string>

namespace Ice
{

//
// A special plug-in that sets stringConverter and wstringConverter during
// construction (when the provided stringConverter resp. wstringConverter
// are not null). Both initialize and destroy are no-op. See Ice::InitializationData.
//

class ICE_API StringConverterPlugin : public Ice::Plugin
{
public:

    StringConverterPlugin(const CommunicatorPtr&, 
                          const IceUtil::StringConverterPtr&, 
                          const IceUtil::WstringConverterPtr& = 0);

    virtual void initialize();

    virtual void destroy();
};

}

#endif
