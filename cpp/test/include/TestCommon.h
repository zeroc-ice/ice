// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <IceUtil/Config.h>

#if defined(_MSC_VER) && !defined(TEST_API_EXPORTS)
#   pragma comment(lib, ICE_LIBNAME("testcommon"))
#endif

#include <Ice/CommunicatorF.h>
#include <Ice/ProxyF.h>
#include <Ice/Initialize.h>

#if defined(ICE_OS_UWP) || (TARGET_OS_IPHONE != 0)
#   include <Ice/Initialize.h>
#   include <Ice/Logger.h>
#   include <Ice/LocalException.h>
#endif

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

void
inline print(const std::string& msg)
{
    std::cout << msg << std::flush;
}

void
inline println(const std::string& msg)
{
    std::cout << msg << std::endl;
}

TEST_API std::string getTestEndpoint(const Ice::CommunicatorPtr&, int, const std::string& = std::string());
TEST_API std::string getTestEndpoint(const Ice::PropertiesPtr&, int, const std::string& = std::string());
TEST_API std::string getTestProtocol(const Ice::PropertiesPtr&);
TEST_API std::string getTestHost(const Ice::PropertiesPtr&);
TEST_API int getTestPort(const Ice::PropertiesPtr&, int);
TEST_API Ice::InitializationData getTestInitData(int&, char*[]);

#if !defined(ICE_OS_UWP) && (TARGET_OS_IPHONE == 0)

void
inline testFailed(const char* expr, const char* file, unsigned int line)
{
    std::cout << "failed!" << std::endl;
    std::cout << file << ':' << line << ": assertion `" << expr << "' failed" << std::endl;
    abort();
}

#define DEFINE_TEST(name)
#define TEST_READY

#else

#include <TestHelper.h>

namespace Test
{

extern MainHelper* helper;

class MainHelperInit
{
public:

    MainHelperInit(MainHelper* r, const std::string& name, bool redirect)
    {
        helper = r;

        if(redirect)
        {
            _previousLogger = Ice::getProcessLogger();
            Ice::setProcessLogger(Ice::getProcessLogger()->cloneWithPrefix(name));

            _previousCoutBuffer = std::cout.rdbuf();
            std::cout.rdbuf(r);

            _previousCerrBuffer = std::cerr.rdbuf();
            std::cerr.rdbuf(r);
        }
    }

    ~MainHelperInit()
    {
        if(_previousLogger)
        {
            Ice::setProcessLogger(_previousLogger);
            std::cout.rdbuf(_previousCoutBuffer);
            std::cerr.rdbuf(_previousCerrBuffer);
        }
    }

private:

    Ice::LoggerPtr _previousLogger;
    std::streambuf* _previousCoutBuffer;
    std::streambuf* _previousCerrBuffer;
};

//
// Redefine main as Test::mainEntryPoint
//
#define main Test::mainEntryPoint
int mainEntryPoint(int, char**);

}

class TestFailedException : public ::Ice::LocalException
{
public:

    TestFailedException(const char* file, int line) :
        LocalException(file, line)
    {
    }

    TestFailedException(const char* file, int line, const ::std::string& r) :
        LocalException(file, line),
        reason(r)
    {
    }

#ifndef ICE_CPP11_COMPILER
    virtual ~TestFailedException() throw()
    {
    }
#endif

    virtual ::std::string ice_id() const
    {
        return "::TestFailedException";
    }

#ifdef ICE_CPP11_MAPPING
    virtual IceUtil::Exception* ice_cloneImpl() const
    {
        return new TestFailedException(*this);
    }
#else
    virtual TestFailedException* ice_clone() const
    {
        return new TestFailedException(*this);
    }
#endif

    virtual void ice_throw() const
    {
        throw *this;
    }

    ::std::string reason;
};

void
inline testFailed(const char* expr, const char* file, unsigned int line)
{
    std::cout << "failed!" << std::endl;
    std::cout << file << ':' << line << ": assertion `" << expr << "' failed" << std::endl;
    throw TestFailedException(__FILE__, __LINE__, "Test Failed");
}

#define DEFINE_TEST(name) \
   Test::MainHelper* Test::helper; \
   Ice::CommunicatorPtr communicatorInstance; \
   extern "C" { \
      ICE_DECLSPEC_EXPORT void dllTestShutdown(); \
      void dllTestShutdown() \
      { \
          try \
          { \
             communicatorInstance->destroy(); \
          } \
          catch(const Ice::LocalException&) \
          { \
          } \
      } \
      ICE_DECLSPEC_EXPORT int dllMain(int, char**, Test::MainHelper*); \
      int dllMain(int argc, char** argv, Test::MainHelper* helper) \
      { \
          Test::MainHelperInit init(helper, name, helper->redirect());  \
          return Test::mainEntryPoint(argc, argv); \
      } \
   }

#define TEST_READY Test::helper->serverReady(); \
    communicatorInstance = communicator;

#endif

#define test(ex) ((ex) ? ((void)0) : testFailed(#ex, __FILE__, __LINE__))

#endif
