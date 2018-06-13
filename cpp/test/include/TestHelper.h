// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <cassert>
#include <cstdlib>
#include <string>
#include <iostream>

#include <IceUtil/Config.h>

#if defined(_MSC_VER) && !defined(TEST_API_EXPORTS)
#   pragma comment(lib, ICE_LIBNAME("testcommon"))
#endif

#include <Ice/CommunicatorF.h>
#include <Ice/ProxyF.h>
#include <Ice/Initialize.h>
#include <Ice/Logger.h>
#include <Ice/LocalException.h>

#include <IceUtil/IceUtil.h>

#ifndef TEST_API
#   if defined(ICE_STATIC_LIBS)
#       define TEST_API /**/
#   elif defined(TEST_API_EXPORTS)
#       define TEST_API ICE_DECLSPEC_EXPORT
#   else
#       define TEST_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace Test
{

class TEST_API ControllerHelper
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:

    virtual bool redirect() const = 0;

    virtual std::string getOutput() const = 0;
    virtual std::string loggerPrefix() const = 0;
    virtual void join() = 0;
    virtual void print(const std::string&) = 0;
    virtual void serverReady() = 0;
    virtual void shutdown() = 0;

    virtual int waitSuccess(int) const = 0;
    virtual void waitReady(int) const = 0;
};
ICE_DEFINE_PTR(ControllerHelperPtr, ControllerHelper);

#if defined(ICE_OS_UWP) || (TARGET_OS_IPHONE != 0)

//
// streambuf redirection implementation
//
class StreamHelper : public std::streambuf
{
public:

    StreamHelper(ControllerHelper* controllerHelper, bool redirect) : _controllerHelper(controllerHelper)
    {
        setp(&data[0], &data[sizeof(data) - 1]);
        if(redirect)
        {
            _previousLogger = Ice::getProcessLogger();
            Ice::setProcessLogger(Ice::getProcessLogger()->cloneWithPrefix(_controllerHelper->loggerPrefix()));

            _previousCoutBuffer = std::cout.rdbuf();
            std::cout.rdbuf(this);

            _previousCerrBuffer = std::cerr.rdbuf();
            std::cerr.rdbuf(this);
        }
    }

    ~StreamHelper()
    {
        if(_previousLogger)
        {
            Ice::setProcessLogger(_previousLogger);
            std::cout.rdbuf(_previousCoutBuffer);
            std::cerr.rdbuf(_previousCerrBuffer);
        }
    }

    virtual void flush()
    {
    }

    virtual void newLine()
    {
        _controllerHelper->print("\n");
    }

private:

    int sync()
    {
        assert(_controllerHelper);
        std::streamsize n = pptr() - pbase();
        _controllerHelper->print(std::string(pbase(), static_cast<int>(n)));
        pbump(-static_cast<int>(pptr() - pbase()));
        return 0;
    }

    int overflow(int ch)
    {
        sync();
        if(ch != EOF)
        {
            assert(pptr() != epptr());
            sputc(ch);
        }
        return 0;
    }

    int sputc(char c)
    {
        if(c == '\n')
        {
            pubsync();
        }
        return std::streambuf::sputc(c);
    }

    ControllerHelper* _controllerHelper;
    char data[1024];
    Ice::LoggerPtr _previousLogger;
    std::streambuf* _previousCoutBuffer;
    std::streambuf* _previousCerrBuffer;
};
#endif

class TEST_API TestHelper
{
public:

    TestHelper(bool registerPlugins = true);
    virtual ~TestHelper();

    void setControllerHelper(ControllerHelper*);

    std::string getTestEndpoint(const std::string&);
    std::string getTestEndpoint(int num = 0, const std::string& prot = "");
    static std::string getTestEndpoint(const Ice::PropertiesPtr& properties, int num = 0, const std::string& prot = "");

    std::string getTestHost();
    static std::string getTestHost(const Ice::PropertiesPtr&);

    std::string getTestProtocol();
    static std::string getTestProtocol(const Ice::PropertiesPtr&);

    int getTestPort(int port = 0);
    static int getTestPort(const Ice::PropertiesPtr&, int port = 0);

    static Ice::PropertiesPtr
    createTestProperties(int&, char*[]);

    Ice::CommunicatorPtr
    initialize(int& argc, char* argv[], const Ice::PropertiesPtr& properties = ICE_NULLPTR);

    Ice::CommunicatorPtr initialize(int& argc, char* argv[], Ice::InitializationData initData);

    Ice::CommunicatorPtr communicator() const;

    void serverReady();
    void shutdown();

    void shutdownOnInterrupt();

    virtual void run(int argc, char* argv[]) = 0;

private:

    ControllerHelper* _controllerHelper;
    Ice::CommunicatorPtr _communicator;
#if !defined(ICE_OS_UWP) && (!defined(__APPLE__) || TARGET_OS_IPHONE == 0)
    IceUtil::CtrlCHandler* _ctrlCHandler;
#endif
};

#if defined(ICE_OS_UWP) || (TARGET_OS_IPHONE != 0)

class TestFailedException
{
};

void
inline testFailed(const char* expr, const char* file, unsigned int line)
{
    std::cout << "failed!" << std::endl;
    std::cout << file << ':' << line << ": assertion `" << expr << "' failed" << std::endl;
    throw TestFailedException();
}

#else

void
inline testFailed(const char* expr, const char* file, unsigned int line)
{
    std::cout << "failed!" << std::endl;
    std::cout << file << ':' << line << ": assertion `" << expr << "' failed" << std::endl;
    abort();
}

template<typename T>
int
runTest(int argc, char* argv[])
{
    int status = 0;
    try
    {
        T helper;
        helper.run(argc, argv);
    }
    catch(const std::exception& ex)
    {
        std::cerr << "error: " << ex.what() << std::endl;
        status = 1;
    }
    return status;
}
#endif

}

#if defined(ICE_OS_UWP) || (TARGET_OS_IPHONE != 0)
#   define DEFINE_TEST(HELPER)                                \
    extern "C"                                                \
    {                                                         \
        ICE_DECLSPEC_EXPORT Test::TestHelper* createHelper()  \
        {                                                     \
            return new HELPER();                              \
        }                                                     \
    }

#else

#   define DEFINE_TEST(HELPER)                          \
    int main(int argc, char** argv)                     \
    {                                                   \
        return Test::runTest<HELPER>(argc, argv);       \
    }

#endif

#define test(ex) ((ex) ? ((void)0) : Test::testFailed(#ex, __FILE__, __LINE__))

#endif
