// Copyright (c) ZeroC, Inc.

#include "Ice/Initialize.h"
#include "ArgVector.h"
#include "CheckIdentity.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Properties.h"
#include "Ice/StringUtil.h"
#include "Instance.h"
#include "LoggerI.h"
#include "PluginManagerI.h"

#include <algorithm>
#include <mutex>
#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    mutex globalMutex;
    Ice::LoggerPtr processLogger;
}

Ice::CommunicatorPtr
Ice::initialize(InitializationData initData)
{
    CommunicatorPtr communicator = Communicator::create(std::move(initData));
    communicator->finishSetup();
    return communicator;
}

LoggerPtr
Ice::getProcessLogger()
{
    lock_guard lock(globalMutex);

    if (processLogger == nullptr)
    {
        //
        // TODO: Would be nice to be able to use process name as prefix by default.
        //
        processLogger = make_shared<LoggerI>("", "", true);
    }
    return processLogger;
}

void
Ice::setProcessLogger(const LoggerPtr& logger)
{
    lock_guard lock(globalMutex);
    processLogger = logger;
}

InstancePtr
IceInternal::getInstance(const CommunicatorPtr& communicator)
{
    return communicator->_instance;
}

IceInternal::TimerPtr
IceInternal::getInstanceTimer(const CommunicatorPtr& communicator)
{
    return communicator->_instance->timer();
}

Identity
Ice::stringToIdentity(string_view s)
{
    Identity ident;

    //
    // Find unescaped separator; note that the string may contain an escaped
    // backslash before the separator.
    //
    string::size_type slash = string::npos;
    string::size_type pos = 0;
    while ((pos = s.find('/', pos)) != string::npos)
    {
        string::size_type escapes = 0;
        while (static_cast<int>(pos - escapes) > 0 && s[pos - escapes - 1] == '\\')
        {
            escapes++;
        }

        //
        // We ignore escaped escapes
        //
        if (escapes % 2 == 0)
        {
            if (slash == string::npos)
            {
                slash = pos;
            }
            else
            {
                //
                // Extra unescaped slash found.
                //
                throw ParseException(__FILE__, __LINE__, "unescaped '/' in identity '" + string{s} + "'");
            }
        }
        pos++;
    }

    if (slash == string::npos)
    {
        try
        {
            ident.name = unescapeString(s, 0, s.size(), "/");
        }
        catch (const invalid_argument& ex)
        {
            throw ParseException(__FILE__, __LINE__, "invalid identity name '" + string{s} + "': " + ex.what());
        }
    }
    else
    {
        try
        {
            ident.category = unescapeString(s, 0, slash, "/");
        }
        catch (const invalid_argument& ex)
        {
            throw ParseException(__FILE__, __LINE__, "invalid category in identity '" + string{s} + "': " + ex.what());
        }

        if (slash + 1 < s.size())
        {
            try
            {
                ident.name = unescapeString(s, slash + 1, s.size(), "/");
            }
            catch (const invalid_argument& ex)
            {
                throw ParseException(__FILE__, __LINE__, "invalid name in identity '" + string{s} + "': " + ex.what());
            }
        }
    }

    checkIdentity(ident, __FILE__, __LINE__);
    return ident;
}

string
Ice::identityToString(const Identity& ident, ToStringMode toStringMode)
{
    checkIdentity(ident, __FILE__, __LINE__);
    if (ident.category.empty())
    {
        return escapeString(ident.name, "/", toStringMode);
    }
    else
    {
        return escapeString(ident.category, "/", toStringMode) + '/' + escapeString(ident.name, "/", toStringMode);
    }
}
