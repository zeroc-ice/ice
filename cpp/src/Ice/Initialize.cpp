// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/ArgVector.h>
#include <Ice/CommunicatorI.h>
#include <Ice/PropertiesI.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <Ice/StreamI.h>
#include <Ice/LoggerI.h>
#include <Ice/Instance.h>
#include <Ice/PluginManagerI.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/StringConverter.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

pair<const Byte*, const Byte*>
makePair(const vector<Byte>& v)
{
    if(v.empty())
    {
        return pair<const Byte*, const Byte*>(static_cast<Byte*>(0), static_cast<Byte*>(0));
    }
    else
    {
        return pair<const Byte*, const Byte*>(&v[0], &v[0] + v.size());
    }
}

IceUtil::Mutex* globalMutex = 0;
Ice::LoggerPtr processLogger;

class Init
{
public:

    Init()
    {
        globalMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
    }
};

Init init;

}

StringSeq
Ice::argsToStringSeq(int argc, char* argv[])
{
    StringSeq result;
    for(int i = 0; i < argc; i++)
    {
        result.push_back(argv[i]);
    }
    return result;
}

#ifdef _WIN32

StringSeq
Ice::argsToStringSeq(int /*argc*/, wchar_t* argv[])
{
    //
    // Don't need to use a wide string converter argv is expected to
    // come from Windows API.
    //
    const IceUtil::StringConverterPtr converter = IceUtil::getProcessStringConverter();
    StringSeq args;
    for(int i=0; argv[i] != 0; i++)
    {
        args.push_back(IceUtil::wstringToString(argv[i], converter));
    }
    return args;
}

#endif

void
Ice::stringSeqToArgs(const StringSeq& args, int& argc, char* argv[])
{
    //
    // Shift all elements in argv which are present in args to the
    // beginning of argv. We record the original value of argc so
    // that we can know later if we've shifted the array.
    //
    const int argcOrig = argc;
    int i = 0;
    while(i < argc)
    {
        if(find(args.begin(), args.end(), argv[i]) == args.end())
        {
            for(int j = i; j < argc - 1; j++)
            {
                argv[j] = argv[j + 1];
            }
            --argc;
        }
        else
        {
            ++i;
        }
    }

    //
    // Make sure that argv[argc] == 0, the ISO C++ standard requires this.
    // We can only do this if we've shifted the array, otherwise argv[argc]
    // may point to an invalid address.
    //
    if(argv && argcOrig != argc)
    {
        argv[argc] = 0;
    }
}

PropertiesPtr
Ice::createProperties()
{
    return new PropertiesI(IceUtil::getProcessStringConverter());
}

PropertiesPtr
Ice::createProperties(StringSeq& args, const PropertiesPtr& defaults)
{
    return new PropertiesI(args, defaults, IceUtil::getProcessStringConverter());
}

PropertiesPtr
Ice::createProperties(int& argc, char* argv[], const PropertiesPtr& defaults)
{
    StringSeq args = argsToStringSeq(argc, argv);
    PropertiesPtr properties = createProperties(args, defaults);
    stringSeqToArgs(args, argc, argv);
    return properties;
}

Ice::ThreadHookPlugin::ThreadHookPlugin(const CommunicatorPtr& communicator, const ThreadNotificationPtr& threadHook)
{
    if(communicator == 0)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "Communicator cannot be null");
    }

    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    instance->setThreadHook(threadHook);
}

void
Ice::ThreadHookPlugin::initialize()
{
}

void
Ice::ThreadHookPlugin::destroy()
{
}

namespace
{

inline void checkIceVersion(Int version)
{
#ifndef ICE_IGNORE_VERSION

#   if ICE_INT_VERSION % 100 > 50
    //
    // Beta version: exact match required
    //
    if(ICE_INT_VERSION != version)
    {
        throw VersionMismatchException(__FILE__, __LINE__);
    }
#   else

    //
    // Major and minor version numbers must match.
    //
    if(ICE_INT_VERSION / 100 != version / 100)
    {
        throw VersionMismatchException(__FILE__, __LINE__);
    }

    //
    // Reject beta caller
    //
    if(version % 100 > 50)
    {
        throw VersionMismatchException(__FILE__, __LINE__);
    }

    //
    // The caller's patch level cannot be greater than library's patch level. (Patch level changes are
    // backward-compatible, but not forward-compatible.)
    //
    if(version % 100 > ICE_INT_VERSION % 100)
    {
        throw VersionMismatchException(__FILE__, __LINE__);
    }

#   endif
#endif
}

}

CommunicatorPtr
Ice::initialize(int& argc, char* argv[], const InitializationData& initializationData, Int version)
{
    checkIceVersion(version);

    InitializationData initData = initializationData;
    initData.properties = createProperties(argc, argv, initData.properties);

    CommunicatorI* communicatorI = new CommunicatorI(initData);
    CommunicatorPtr result = communicatorI; // For exception safety.
    communicatorI->finishSetup(argc, argv);
    return result;
}

CommunicatorPtr
Ice::initialize(StringSeq& args, const InitializationData& initializationData, Int version)
{
    CommunicatorPtr communicator;
    IceUtilInternal::ArgVector av(args);
    communicator = initialize(av.argc, av.argv, initializationData, version);
    args = argsToStringSeq(av.argc, av.argv);
    return communicator;
}

CommunicatorPtr
Ice::initialize(const InitializationData& initData, Int version)
{
    //
    // We can't simply call the other initialize() because this one does NOT read
    // the config file, while the other one always does.
    //
    checkIceVersion(version);

    CommunicatorI* communicatorI = new CommunicatorI(initData);
    CommunicatorPtr result = communicatorI; // For exception safety.
    int argc = 0;
    char* argv[] = { 0 };
    communicatorI->finishSetup(argc, argv);
    return result;
}

InputStreamPtr
Ice::createInputStream(const CommunicatorPtr& communicator, const vector<Byte>& bytes)
{
    return new InputStreamI(communicator, makePair(bytes), true);
}

InputStreamPtr
Ice::createInputStream(const CommunicatorPtr& communicator, const vector<Byte>& bytes, const EncodingVersion& v)
{
    return new InputStreamI(communicator, makePair(bytes), v, true);
}

InputStreamPtr
Ice::wrapInputStream(const CommunicatorPtr& communicator, const vector<Byte>& bytes)
{
    return new InputStreamI(communicator, makePair(bytes), false);
}

InputStreamPtr
Ice::wrapInputStream(const CommunicatorPtr& communicator, const vector<Byte>& bytes, const EncodingVersion& v)
{
    return new InputStreamI(communicator, makePair(bytes), v, false);
}

InputStreamPtr
Ice::createInputStream(const CommunicatorPtr& communicator, const pair<const Ice::Byte*, const Ice::Byte*>& bytes)
{
    return new InputStreamI(communicator, bytes, true);
}

InputStreamPtr
Ice::createInputStream(const CommunicatorPtr& communicator, const pair<const Ice::Byte*, const Ice::Byte*>& bytes,
                       const EncodingVersion& v)
{
    return new InputStreamI(communicator, bytes, v, true);
}

InputStreamPtr
Ice::wrapInputStream(const CommunicatorPtr& communicator, const pair<const Ice::Byte*, const Ice::Byte*>& bytes)
{
    return new InputStreamI(communicator, bytes, false);
}

InputStreamPtr
Ice::wrapInputStream(const CommunicatorPtr& communicator, const pair<const Ice::Byte*, const Ice::Byte*>& bytes,
                     const EncodingVersion& v)
{
    return new InputStreamI(communicator, bytes, v, false);
}

OutputStreamPtr
Ice::createOutputStream(const CommunicatorPtr& communicator)
{
    return new OutputStreamI(communicator);
}

OutputStreamPtr
Ice::createOutputStream(const CommunicatorPtr& communicator, const EncodingVersion& v)
{
    return new OutputStreamI(communicator, v);
}

LoggerPtr
Ice::getProcessLogger()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);

    if(processLogger == 0)
    {
       //
       // TODO: Would be nice to be able to use process name as prefix by default.
       //
       processLogger = new Ice::LoggerI("", "", true, IceUtil::getProcessStringConverter());
    }
    return processLogger;
}

void
Ice::setProcessLogger(const LoggerPtr& logger)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);
    processLogger = logger;
}

void
Ice::registerPluginFactory(const std::string& name, PLUGIN_FACTORY factory, bool loadOnInitialize)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);
    PluginManagerI::registerPluginFactory(name, factory, loadOnInitialize);
}

InstancePtr
IceInternal::getInstance(const CommunicatorPtr& communicator)
{
    CommunicatorI* p = dynamic_cast<CommunicatorI*>(communicator.get());
    assert(p);
    return p->_instance;
}

IceUtil::TimerPtr
IceInternal::getInstanceTimer(const CommunicatorPtr& communicator)
{
    CommunicatorI* p = dynamic_cast<CommunicatorI*>(communicator.get());
    assert(p);
    return p->_instance->timer();
}

#ifdef ICE_CPP11
Ice::DispatcherPtr
Ice::newDispatcher(const ::std::function<void (const DispatcherCallPtr&, const ConnectionPtr)>& cb)
{
    class Cpp11Dispatcher : public Dispatcher
    {
    public:

        Cpp11Dispatcher(const ::std::function<void (const DispatcherCallPtr&, const ConnectionPtr)>& cb) :
            _cb(cb)
        {
        }

        virtual void dispatch(const DispatcherCallPtr& call, const ConnectionPtr& conn)
        {
            _cb(call, conn);
        }

    private:
        const ::std::function<void (const DispatcherCallPtr&, const ConnectionPtr)> _cb;
    };

    return new Cpp11Dispatcher(cb);
}
#endif

#ifdef ICE_CPP11
Ice::BatchRequestInterceptorPtr
Ice::newBatchRequestInterceptor(const ::std::function<void (const BatchRequest&, int, int)>& cb)
{
    class Cpp11BatchRequestInterceptor : public BatchRequestInterceptor
    {
    public:

        Cpp11BatchRequestInterceptor(const ::std::function<void (const BatchRequest&, int, int)>& cb) :
            _cb(cb)
        {
        }

        virtual void enqueue(const BatchRequest& request, int count, int size)
        {
            _cb(request, count, size);
        }

    private:
        const ::std::function<void (const BatchRequest&, int, int)> _cb;
    };

    return new Cpp11BatchRequestInterceptor(cb);
}
#endif

Identity
Ice::stringToIdentity(const string& s)
{
    //
    // This method only accepts printable ascii. Since printable ascii is a subset
    // of all narrow string encodings, it is not necessary to convert the string
    // from the native string encoding. Any characters other than printable-ASCII
    // will cause an IllegalArgumentException. Note that it can contain Unicode
    // encoded in the escaped form which is the reason why we call fromUTF8 after
    // unespcaping the printable ASCII string.
    //

    Identity ident;

    //
    // Find unescaped separator; note that the string may contain an escaped
    // backslash before the separator.
    //
    string::size_type slash = string::npos, pos = 0;
    while((pos = s.find('/', pos)) != string::npos)
    {
        int escapes = 0;
        while(static_cast<int>(pos)- escapes > 0 && s[pos - escapes - 1] == '\\')
        {
            escapes++;
        }

        //
        // We ignore escaped escapes
        //
        if(escapes % 2 == 0)
        {
            if(slash == string::npos)
            {
                slash = pos;
            }
            else
            {
                //
                // Extra unescaped slash found.
                //
                IdentityParseException ex(__FILE__, __LINE__);
                ex.str = "unescaped backslash in identity `" + s + "'";
                throw ex;
            }
        }
        pos++;
    }

    if(slash == string::npos)
    {
        try
        {
            ident.name = IceUtilInternal::unescapeString(s, 0, s.size());
        }
        catch(const IceUtil::IllegalArgumentException& e)
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = "invalid identity name `" + s + "': " + e.reason();
            throw ex;
        }
    }
    else
    {
        try
        {
            ident.category = IceUtilInternal::unescapeString(s, 0, slash);
        }
        catch(const IceUtil::IllegalArgumentException& e)
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = "invalid category in identity `" + s + "': " + e.reason();
            throw ex;
        }
        if(slash + 1 < s.size())
        {
            try
            {
                ident.name = IceUtilInternal::unescapeString(s, slash + 1, s.size());
            }
            catch(const IceUtil::IllegalArgumentException& e)
            {
                IdentityParseException ex(__FILE__, __LINE__);
                ex.str = "invalid name in identity `" + s + "': " + e.reason();
                throw ex;
            }
        }
    }

    ident.name = UTF8ToNative(ident.name, IceUtil::getProcessStringConverter());
    ident.category = UTF8ToNative(ident.category, IceUtil::getProcessStringConverter());

    return ident;
}

string
Ice::identityToString(const Identity& ident)
{
    //
    // This method returns the stringified identity. The returned string only
    // contains printable ascii. It can contain UTF8 in the escaped form.
    //
    string name = nativeToUTF8(ident.name, IceUtil::getProcessStringConverter());
    string category = nativeToUTF8(ident.category, IceUtil::getProcessStringConverter());

    if(category.empty())
    {
        return IceUtilInternal::escapeString(name, "/");
    }
    else
    {
        return IceUtilInternal::escapeString(category, "/") + '/' + IceUtilInternal::escapeString(name, "/");
    }
}
