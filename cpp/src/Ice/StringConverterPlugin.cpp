// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/StringConverterPlugin.h>
#include <IceUtil/IceUtil.h>
#include <IceUtil/StringUtil.h>

#include <Ice/Initialize.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Communicator.h>

#ifndef _WIN32
#  include <IceUtil/IconvStringConverter.h>
#endif

#ifdef __MINGW32__
#  include <limits.h>
#endif

using namespace IceUtil;
using namespace IceUtilInternal;
using namespace std;


Ice::StringConverterPlugin::StringConverterPlugin(const CommunicatorPtr& communicator,
                                                  const StringConverterPtr& stringConverter, 
                                                  const WstringConverterPtr& wstringConverter)
{
    if(communicator == 0)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "Communicator cannot be null");
    }
    
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);

    IceUtil::setProcessStringConverter(stringConverter);
    instance->setStringConverter(stringConverter);
    IceUtil::setProcessWstringConverter(wstringConverter);
    instance->setWstringConverter(wstringConverter);
}

void
Ice::StringConverterPlugin::initialize()
{
}

void
Ice::StringConverterPlugin::destroy()
{
}

//
// The entry point for the "string converter" plug-in built-in the Ice library
//
extern "C"
{

using namespace Ice;

ICE_DECLSPEC_EXPORT Plugin*
createStringConverter(const CommunicatorPtr& communicator, const string& name, const StringSeq& args)
{
    StringConverterPtr stringConverter;
    WstringConverterPtr wstringConverter;

    if(args.size() > 2)
    {
        Error out(communicator->getLogger());
        out << "Plugin " << name << ": too many arguments";
        return 0;
    }

    try
    {

#ifdef _WIN32
        int cp = -1;

        for(size_t i = 0; i < args.size(); ++i)
        {
            if(args[i].find("windows=") == 0)
            {
                cp = atoi(args[i].substr(strlen("windows=")).c_str());
            }
            else if(args[i].find("iconv=") != 0)
            {
                Error out(communicator->getLogger());
                out << "Plugin " << name << ": invalid \"" << args[i] << "\" argument";
                return 0;
            }
        }
        
        if(cp == -1)
        {
            Error out(communicator->getLogger());
            out << "Plugin " << name << ": missing windows=<code page> argument";
            return 0;
        }

        if(cp == 0 || cp == INT_MAX || cp < 0)
        {
            Error out(communicator->getLogger());
            out << "Plugin " << name << ": invalid Windows code page";
            return 0;
        }

        stringConverter = new WindowsStringConverter(static_cast<unsigned int>(cp));
#else
        StringSeq iconvArgs;

        for(size_t i = 0; i < args.size(); ++i)
        {
            if(args[i].find("iconv=") == 0)
            {
                if(!IceUtilInternal::splitString(args[i].substr(strlen("iconv=")), ", \t\r\n", iconvArgs))
                {
                    Error out(communicator->getLogger());
                    out << "Plugin " << name << ": invalid iconv argument";
                    return 0;
                }
            }
            else if(args[i].find("windows=") != 0)
            {
                Error out(communicator->getLogger());
                out << "Plugin " << name << ": invalid \"" << args[i] << "\" argument";
                return 0;
            }
        }
        
        switch(iconvArgs.size())
        {
            case 0:
            {
                stringConverter = new IconvStringConverter<char>;
                break;
            }
            case 1:
            {
                stringConverter = new IconvStringConverter<char>(iconvArgs[0].c_str());
                break;
            }
            case 2:
            {
                stringConverter = new IconvStringConverter<char>(iconvArgs[0].c_str());
                wstringConverter = new IconvStringConverter<wchar_t>(iconvArgs[1].c_str());
                break;
            }
            default:
            {
                assert(0);
            }
        }

#endif    

        return new StringConverterPlugin(communicator, stringConverter, wstringConverter);
    }
    catch(const std::exception& ex)
    {
        Error out(communicator->getLogger());
        out << "Plugin " << name << ": creation failed with " << ex.what();
        return 0;
    }
    catch(...)
    {
        Error out(communicator->getLogger());
        out << "Plugin " << name << ": creation failed with unknown exception";
        return 0;
    }
}
}
