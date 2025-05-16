// Copyright (c) ZeroC, Inc.

#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include "Ice/CommunicatorF.h"
#include "Ice/Config.h"
#include "Ice/CtrlCHandler.h"
#include "Ice/Demangle.h"
#include "Ice/Initialize.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Logger.h"
#include "Ice/ProxyF.h"

#if defined(_MSC_VER) && !defined(TEST_API_EXPORTS) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("testcommon"))
#endif

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>
#include <typeinfo>

#ifndef TEST_API
#    if defined(TEST_API_EXPORTS)
#        define TEST_API ICE_DECLSPEC_EXPORT
#    else
#        define TEST_API ICE_DECLSPEC_IMPORT
#    endif
#endif

namespace Test
{
    class TEST_API ControllerHelper
    {
    public:
        [[nodiscard]] virtual std::string loggerPrefix() const = 0;
        virtual void print(const std::string&) = 0;

        virtual void serverReady() = 0;
        virtual void communicatorInitialized(const Ice::CommunicatorPtr&) = 0;
    };
    using ControllerHelperPtr = std::shared_ptr<ControllerHelper>;

#if TARGET_OS_IPHONE != 0

    //
    // streambuf redirection implementation
    //
    class StreamHelper : public std::streambuf
    {
    public:
        StreamHelper();
        ~StreamHelper();

        void setControllerHelper(ControllerHelper*);

        virtual void flush();
        virtual void newLine();

    private:
        virtual int sync();
        virtual int overflow(int);
        virtual int sputc(char);

        std::mutex _mutex;
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

        [[nodiscard]] std::string getTestEndpoint(const std::string&) const;
        [[nodiscard]] std::string getTestEndpoint(int num = 0, const std::string& prot = "") const;
        static std::string
        getTestEndpoint(const Ice::PropertiesPtr& properties, int num = 0, const std::string& prot = "");

        [[nodiscard]] std::string getTestHost() const;
        static std::string getTestHost(const Ice::PropertiesPtr&);

        [[nodiscard]] std::string getTestProtocol() const;
        static std::string getTestProtocol(const Ice::PropertiesPtr&);

        [[nodiscard]] int getTestPort(int port = 0) const;
        static int getTestPort(const Ice::PropertiesPtr&, int port = 0);

        static Ice::PropertiesPtr createTestProperties(int&, char*[]);

        Ice::CommunicatorPtr initialize(int& argc, char* argv[], const Ice::PropertiesPtr& properties = nullptr);
        Ice::CommunicatorPtr initialize(Ice::InitializationData);
        Ice::CommunicatorPtr initialize(int&, char*[], Ice::InitializationData);

        [[nodiscard]] Ice::CommunicatorPtr communicator() const;

        void serverReady();
        void shutdown();

        void shutdownOnInterrupt();

        virtual void run(int argc, char* argv[]) = 0;

    private:
        ControllerHelper* _controllerHelper{nullptr};
        Ice::CommunicatorPtr _communicator;
        bool _registerPlugins{true};
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
        Ice::CtrlCHandler* _ctrlCHandler{nullptr};
#endif
    };

#if TARGET_OS_IPHONE != 0

    class TestFailedException
    {
    };

    void inline testFailed(const char* expr, const char* file, unsigned int line)
    {
        std::cout << "failed!" << std::endl;
        std::cout << file << ':' << line << ": assertion `" << expr << "' failed" << std::endl;
        throw TestFailedException();
    }

#else

    void inline testFailed(const char* expr, const char* file, unsigned int line)
    {
        std::cout << "failed!" << std::endl;
        std::cout << file << ':' << line << ": assertion `" << expr << "' failed" << std::endl;
        abort();
    }

    template<typename T> int runTest(int argc, char* argv[])
    {
        int status = 0;
        try
        {
            T helper;
            helper.run(argc, argv);
        }
        catch (const Ice::LocalException& ex)
        {
            std::cerr << "error: " << ex << std::endl;
            status = 1;
        }
        catch (const std::exception& ex)
        {
            std::cerr << "error: " << IceInternal::demangle(typeid(ex).name()) << ' ' << ex.what() << std::endl;
            status = 1;
        }
        return status;
    }
#endif
}

namespace Ice
{
    /// Installs the plug-in factory for the current transport into @p initData, if needed.
    /// Relies on @p initData's properties to determine the transport.
    /// @param initData The initialization data to modify.
    TEST_API void installTransport(InitializationData& initData);
}

#if TARGET_OS_IPHONE != 0
#    define DEFINE_TEST(HELPER)                                                                                        \
        extern "C"                                                                                                     \
        {                                                                                                              \
            ICE_DECLSPEC_EXPORT Test::TestHelper* createHelper() { return new HELPER(); }                              \
        }

#else

#    define DEFINE_TEST(HELPER)                                                                                        \
        int main(int argc, char** argv) { return Test::runTest<HELPER>(argc, argv); }

#endif

#define test(ex) ((ex) ? ((void)0) : ::Test::testFailed(#ex, __FILE__, __LINE__))

#endif
