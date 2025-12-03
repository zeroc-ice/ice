// Copyright (c) ZeroC, Inc.

#ifndef ICE_DEFAULT_SLICE_LOADER_H
#define ICE_DEFAULT_SLICE_LOADER_H

#include "SliceLoader.h"
#include "UserException.h"
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <type_traits>

#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4251) // class ... needs to have dll-interface to be used by clients of class ...
#endif

namespace IceInternal
{
    class DefaultSliceLoader;
    using DefaultSliceLoaderPtr = std::shared_ptr<DefaultSliceLoader>;

    /// A singleton SliceLoader that the generated code uses to register factories for classes and exceptions.
    class ICE_API DefaultSliceLoader final : public Ice::SliceLoader
    {
    public:
        /// Returns the singleton instance of DefaultSliceLoader.
        /// @return The single instance of DefaultSliceLoader.
        static DefaultSliceLoaderPtr instance();

        ~DefaultSliceLoader() final;

        [[nodiscard]] Ice::ValuePtr newClassInstance(std::string_view typeId) const final;
        [[nodiscard]] std::exception_ptr newExceptionInstance(std::string_view typeId) const final;

        template<class T, std::enable_if_t<std::is_base_of_v<Ice::Value, T>, bool> = true> void addClass(int compactId)
        {
            std::lock_guard lock{_mutex};
            auto p = _classFactories.find(T::ice_staticId());
            if (p == _classFactories.end())
            {
                _classFactories[T::ice_staticId()] = {[] { return std::make_shared<T>(); }, 1};
            }
            else
            {
                p->second.second++;
            }

            if (compactId != -1)
            {
                std::string compactIdStr{std::to_string(compactId)};
                p = _classFactories.find(compactIdStr);
                if (p == _classFactories.end())
                {
                    _classFactories[compactIdStr] = {[] { return std::make_shared<T>(); }, 1};
                }
                else
                {
                    p->second.second++;
                }
            }
        }

        void removeClass(std::string_view typeId, int compactId)
        {
            std::lock_guard lock{_mutex};
            auto p = _classFactories.find(typeId);
            if (p != _classFactories.end())
            {
                if (--p->second.second == 0)
                {
                    _classFactories.erase(p);
                }
            }

            if (compactId != -1)
            {
                std::string compactIdStr{std::to_string(compactId)};
                p = _classFactories.find(compactIdStr);
                if (p != _classFactories.end())
                {
                    if (--p->second.second == 0)
                    {
                        _classFactories.erase(p);
                    }
                }
            }
        }

        template<class T, std::enable_if_t<std::is_base_of_v<Ice::UserException, T>, bool> = true> void addException()
        {
            std::lock_guard lock{_mutex};
            auto p = _exceptionFactories.find(T::ice_staticId());
            if (p == _exceptionFactories.end())
            {
                _exceptionFactories[T::ice_staticId()] = {[] { return std::make_exception_ptr(T{}); }, 1};
            }
            else
            {
                p->second.second++;
            }
        }

        void removeException(std::string_view typeId)
        {
            std::lock_guard lock{_mutex};
            auto p = _exceptionFactories.find(typeId);
            if (p != _exceptionFactories.end())
            {
                if (--p->second.second == 0)
                {
                    _exceptionFactories.erase(p);
                }
            }
        }

    private:
        using ClassFactory = std::function<Ice::ValuePtr()>;
        using ExceptionFactory = std::function<std::exception_ptr()>;

        DefaultSliceLoader() = default;

        mutable std::mutex _mutex;

        std::map<std::string, std::pair<ClassFactory, int>, std::less<>> _classFactories;
        std::map<std::string, std::pair<ExceptionFactory, int>, std::less<>> _exceptionFactories;
    };

    template<class T> class ClassInit
    {
    public:
        explicit ClassInit(int compactId = -1) noexcept : _typeId(T::ice_staticId()), _compactId(compactId)
        {
            DefaultSliceLoader::instance()->addClass<T>(compactId);
        }

        ~ClassInit() { DefaultSliceLoader::instance()->removeClass(_typeId, _compactId); }

    private:
        const char* _typeId;
        int _compactId;
    };

    template<class T> class ExceptionInit
    {
    public:
        ExceptionInit() noexcept : _typeId(T::ice_staticId()) { DefaultSliceLoader::instance()->addException<T>(); }
        ~ExceptionInit() { DefaultSliceLoader::instance()->removeException(_typeId); }

    private:
        const char* _typeId;
    };
}

#if defined(_MSC_VER)
#    pragma warning(pop)
#endif

#endif
