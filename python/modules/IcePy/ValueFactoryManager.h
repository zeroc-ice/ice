//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPY_VALUE_FACTORY_MANAGER_H
#define ICEPY_VALUE_FACTORY_MANAGER_H

#include <Config.h>
#include <Ice/ValueFactory.h>

#include <mutex>

namespace IcePy
{
    extern PyTypeObject ValueFactoryManagerType;

    bool initValueFactoryManager(PyObject*);

    class ValueFactory
    {
    public:
        virtual std::shared_ptr<Ice::Value> create(std::string_view) = 0;
    };
    using ValueFactoryPtr = std::shared_ptr<ValueFactory>;

    class FactoryWrapper final : public ValueFactory
    {
    public:
        FactoryWrapper(PyObject*);
        ~FactoryWrapper();

        std::shared_ptr<Ice::Value> create(std::string_view) final;

        PyObject* getValueFactory() const;

    protected:
        PyObject* _valueFactory;
    };

    using FactoryWrapperPtr = std::shared_ptr<FactoryWrapper>;

    class DefaultValueFactory final : public ValueFactory
    {
    public:
        std::shared_ptr<Ice::Value> create(std::string_view) final;

        void setDelegate(const ValueFactoryPtr&);
        ValueFactoryPtr getDelegate() const { return _delegate; }

        PyObject* getValueFactory() const;

        void destroy();

    private:
        ValueFactoryPtr _delegate;
    };

    using DefaultValueFactoryPtr = std::shared_ptr<DefaultValueFactory>;

    class ValueFactoryManager final : public Ice::ValueFactoryManager
    {
    public:
        static std::shared_ptr<ValueFactoryManager> create();

        ~ValueFactoryManager();

        void add(Ice::ValueFactoryFunc, std::string_view) final;
        Ice::ValueFactoryFunc find(std::string_view) const noexcept final;

        void add(PyObject*, std::string_view);
        PyObject* findValueFactory(std::string_view) const;

        PyObject* getObject() const;

        void destroy();

    private:
        typedef std::map<std::string, ValueFactoryPtr, std::less<>> FactoryMap;

        ValueFactoryManager();
        ValueFactoryPtr findCore(std::string_view) const noexcept;

        PyObject* _self;
        FactoryMap _factories;
        DefaultValueFactoryPtr _defaultFactory;

        mutable std::mutex _mutex;
    };

    using ValueFactoryManagerPtr = std::shared_ptr<ValueFactoryManager>;
}

#endif
