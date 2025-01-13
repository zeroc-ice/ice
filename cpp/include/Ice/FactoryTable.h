//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_FACTORY_TABLE_H
#define ICE_FACTORY_TABLE_H

#include "Config.h"
#include "UserExceptionFactory.h"
#include "ValueFactory.h"

#include <cassert>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

namespace IceInternal
{
    class ICE_API FactoryTable final
    {
    public:
        FactoryTable() noexcept = default;

        FactoryTable(const FactoryTable&) = delete;
        FactoryTable& operator=(const FactoryTable&) = delete;

        void addExceptionFactory(std::string_view, Ice::UserExceptionFactory);
        [[nodiscard]] Ice::UserExceptionFactory getExceptionFactory(std::string_view) const;
        void removeExceptionFactory(std::string_view);

        void addValueFactory(std::string_view, Ice::ValueFactory);
        [[nodiscard]] Ice::ValueFactory getValueFactory(std::string_view) const;
        void removeValueFactory(std::string_view);

        void addTypeId(int, std::string_view);
        [[nodiscard]] std::string getTypeId(int) const;
        void removeTypeId(int);

    private:
        mutable std::mutex _mutex;

        using EFPair = std::pair<Ice::UserExceptionFactory, int>;
        using EFTable = std::map<std::string, EFPair, std::less<>>;
        EFTable _eft;

        using VFPair = std::pair<Ice::ValueFactory, int>;
        using VFTable = std::map<std::string, VFPair, std::less<>>;
        VFTable _vft;

        using TypeIdPair = std::pair<std::string, int>;
        using TypeIdTable = std::map<int, TypeIdPair>;
        TypeIdTable _typeIdTable;
    };

    extern ICE_API FactoryTable* factoryTable;

    template<class E> void defaultUserExceptionFactory([[maybe_unused]] std::string_view typeId)
    {
        assert(typeId == E::ice_staticId());
        throw E();
    }

    template<class V> Ice::ValuePtr defaultValueFactory([[maybe_unused]] std::string_view typeId)
    {
        assert(typeId == V::ice_staticId());
        return std::make_shared<V>();
    }

    class ICE_API FactoryTableInit
    {
    public:
        FactoryTableInit() noexcept;
        ~FactoryTableInit();
    };

    class ICE_API CompactIdInit
    {
    public:
        CompactIdInit(const char* typeId, int compactId) noexcept;
        ~CompactIdInit();

    private:
        const int _compactId;
    };

    template<class E> class DefaultUserExceptionFactoryInit
    {
    public:
        DefaultUserExceptionFactoryInit(const char* tId) noexcept : typeId(tId)
        {
            factoryTable->addExceptionFactory(typeId, defaultUserExceptionFactory<E>);
        }

        ~DefaultUserExceptionFactoryInit() { factoryTable->removeExceptionFactory(typeId); }

        const char* typeId;
    };

    template<class O> class DefaultValueFactoryInit
    {
    public:
        DefaultValueFactoryInit(const char* tId) noexcept : typeId(tId)
        {
            factoryTable->addValueFactory(typeId, defaultValueFactory<O>);
        }

        ~DefaultValueFactoryInit() { factoryTable->removeValueFactory(typeId); }

        const char* typeId;
    };
}

#endif
