// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;

namespace
{
    class Plugin : public Ice::Plugin
    {
    public:
        Plugin(Ice::CommunicatorPtr communicator) : _communicator(std::move(communicator)) {}

        void initialize() override { _initialized = true; }

        void destroy() override { _destroyed = true; }

        ~Plugin() override
        {
            test(_initialized);
            test(_destroyed);
        }

    private:
        const Ice::CommunicatorPtr _communicator;
        Ice::StringSeq _args;
        bool _initialized{false};
        bool _destroyed{false};
    };

    class CustomPluginException : public std::exception
    {
    public:
        CustomPluginException() noexcept = default;
        [[nodiscard]] const char* what() const noexcept override { return "CustomPluginException"; }
    };

    class PluginInitializeFail : public Ice::Plugin
    {
    public:
        PluginInitializeFail(Ice::CommunicatorPtr communicator) : _communicator(std::move(communicator)) {}

        void initialize() override { throw CustomPluginException(); }

        void destroy() override { test(false); }

    private:
        const Ice::CommunicatorPtr _communicator;
    };

    class BasePlugin;
    using BasePluginPtr = std::shared_ptr<BasePlugin>;

    class BasePlugin : public Ice::Plugin
    {
    public:
        BasePlugin(Ice::CommunicatorPtr communicator) : _communicator(std::move(communicator)) {}

        [[nodiscard]] bool isInitialized() const { return _initialized; }

        [[nodiscard]] bool isDestroyed() const { return _destroyed; }

    protected:
        const Ice::CommunicatorPtr _communicator;
        bool _initialized{false};
        bool _destroyed{false};
        BasePluginPtr _other;
    };

    class PluginOne : public BasePlugin
    {
    public:
        PluginOne(const Ice::CommunicatorPtr& communicator) : BasePlugin(communicator) {}

        void initialize() override
        {
            _other = dynamic_pointer_cast<BasePlugin>(_communicator->getPluginManager()->getPlugin("PluginTwo"));
            test(!_other->isInitialized());
            _initialized = true;
        }

        void destroy() override
        {
            _destroyed = true;
            test(_other->isDestroyed());
            _other = nullptr;
        }
    };

    class PluginTwo : public BasePlugin
    {
    public:
        PluginTwo(const Ice::CommunicatorPtr& communicator) : BasePlugin(communicator) {}

        void initialize() override
        {
            _initialized = true;
            _other = dynamic_pointer_cast<BasePlugin>(_communicator->getPluginManager()->getPlugin("PluginOne"));
            test(_other->isInitialized());
        }

        void destroy() override
        {
            _destroyed = true;
            test(!_other->isDestroyed());
            _other = nullptr;
        }
    };

    class PluginThree : public BasePlugin
    {
    public:
        PluginThree(const Ice::CommunicatorPtr& communicator) : BasePlugin(communicator) {}

        void initialize() override
        {
            _initialized = true;
            _other = dynamic_pointer_cast<BasePlugin>(_communicator->getPluginManager()->getPlugin("PluginTwo"));
            test(_other->isInitialized());
        }

        void destroy() override
        {
            _destroyed = true;
            test(!_other->isDestroyed());
            _other = nullptr;
        }
    };

    class BasePluginFail;
    using BasePluginFailPtr = std::shared_ptr<BasePluginFail>;

    class BasePluginFail : public Ice::Plugin
    {
    public:
        BasePluginFail(Ice::CommunicatorPtr communicator) : _communicator(std::move(communicator)) {}

        [[nodiscard]] bool isInitialized() const { return _initialized; }

        [[nodiscard]] bool isDestroyed() const { return _destroyed; }

    protected:
        const Ice::CommunicatorPtr _communicator;
        bool _initialized{false};
        bool _destroyed{false};
        BasePluginFailPtr _one;
        BasePluginFailPtr _two;
        BasePluginFailPtr _three;
    };

    class PluginOneFail : public BasePluginFail
    {
    public:
        PluginOneFail(const Ice::CommunicatorPtr& communicator) : BasePluginFail(communicator) {}

        void initialize() override
        {
            _two = dynamic_pointer_cast<BasePluginFail>(_communicator->getPluginManager()->getPlugin("PluginTwoFail"));
            test(!_two->isInitialized());
            _three =
                dynamic_pointer_cast<BasePluginFail>(_communicator->getPluginManager()->getPlugin("PluginThreeFail"));
            test(!_three->isInitialized());
            _initialized = true;
        }

        void destroy() override
        {
            test(_two->isDestroyed());
            //
            // Not destroyed because initialize fails.
            //
            test(!_three->isDestroyed());
            _destroyed = true;
            _two = nullptr;
            _three = nullptr;
        }

        ~PluginOneFail() override
        {
            test(_initialized);
            test(_destroyed);
        }
    };

    class PluginTwoFail : public BasePluginFail
    {
    public:
        PluginTwoFail(const Ice::CommunicatorPtr& communicator) : BasePluginFail(communicator) {}

        void initialize() override
        {
            _initialized = true;
            _one = dynamic_pointer_cast<BasePluginFail>(_communicator->getPluginManager()->getPlugin("PluginOneFail"));
            test(_one->isInitialized());
            _three =
                dynamic_pointer_cast<BasePluginFail>(_communicator->getPluginManager()->getPlugin("PluginThreeFail"));
            test(!_three->isInitialized());
        }

        void destroy() override
        {
            _destroyed = true;
            test(!_one->isDestroyed());
            _one = nullptr;
            _three = nullptr;
        }

        ~PluginTwoFail() override
        {
            test(_initialized);
            test(_destroyed);
        }
    };

    class PluginThreeFail : public BasePluginFail
    {
    public:
        PluginThreeFail(const Ice::CommunicatorPtr& communicator) : BasePluginFail(communicator) {}

        void initialize() override { throw CustomPluginException(); }

        void destroy() override { test(false); }

        ~PluginThreeFail() override
        {
            test(!_initialized);
            test(!_destroyed);
        }
    };
}

extern "C" ICE_DECLSPEC_EXPORT Ice::Plugin*
createPlugin(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new Plugin(communicator);
}

extern "C" ICE_DECLSPEC_EXPORT Ice::Plugin*
createPluginWithArgs(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq& args)
{
    test(args.size() == 3);
    test(args[0] == "C:\\Program Files\\");
    test(args[1] == "--DatabasePath");
    test(args[2] == "C:\\Program Files\\Application\\db");
    return new Plugin(communicator);
}

extern "C" ICE_DECLSPEC_EXPORT Ice::Plugin*
createPluginInitializeFail(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginInitializeFail(communicator);
}

extern "C" ICE_DECLSPEC_EXPORT Ice::Plugin*
createPluginOne(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginOne(communicator);
}

extern "C" ICE_DECLSPEC_EXPORT Ice::Plugin*
createPluginTwo(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginTwo(communicator);
}

extern "C" ICE_DECLSPEC_EXPORT Ice::Plugin*
createPluginThree(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginThree(communicator);
}

extern "C" ICE_DECLSPEC_EXPORT Ice::Plugin*
createPluginOneFail(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginOneFail(communicator);
}

extern "C" ICE_DECLSPEC_EXPORT Ice::Plugin*
createPluginTwoFail(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginTwoFail(communicator);
}

extern "C" ICE_DECLSPEC_EXPORT Ice::Plugin*
createPluginThreeFail(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginThreeFail(communicator);
}
