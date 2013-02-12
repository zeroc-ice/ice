// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <IceUtil/IceUtil.h>
#ifdef ICE_OS_WINRT
#   include <Ice/Initialize.h>
#   include <Ice/Logger.h>
#   include <Ice/LocalException.h>
#endif

#include <cstdlib>

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

#ifndef ICE_OS_WINRT

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

namespace Test
{

class MainHelper : public std::streambuf
{
public:

    MainHelper()
    {
        setp(&data[0], &data[sizeof(data) - 1]);
    }

    virtual void serverReady() = 0;

    virtual void shutdown() = 0;

    virtual void waitForCompleted() = 0;

    virtual bool redirect() = 0;

    virtual void print(const std::string& msg) = 0;

    virtual void
    flush()
    {
    }

    virtual void
    newLine()
    {
        print("\n");
    }

private:

    //
    // streambuf redirection implementation
    //

    int sync()
    {
        std::streamsize n = pptr() - pbase();
        print(std::string(pbase(), static_cast<int>(n)));
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

    char data[1024];
};

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

    virtual ~TestFailedException() throw()
    {
    }

    virtual ::std::string ice_name() const
    {
        return "::TestFailedException";
    }

    virtual TestFailedException* ice_clone() const
    {
        return new TestFailedException(*this);
    }

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
      _declspec(dllexport) void dllTestShutdown(); \
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

