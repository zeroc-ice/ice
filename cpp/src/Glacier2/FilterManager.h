// Copyright (c) ZeroC, Inc.

#ifndef FILTER_MANAGER_H
#define FILTER_MANAGER_H

//
// It'd be better if we didn't have to include this everywhere, but
// this is the most expeditious approach for now.
//
#include "FilterT.h"
#include "Ice/ObjectAdapter.h"
#include "Instance.h"

namespace Glacier2
{
    class FilterManager
    {
    public:
        FilterManager(
            std::shared_ptr<Instance>,
            std::shared_ptr<StringSetI>,
            std::shared_ptr<StringSetI>,
            std::shared_ptr<IdentitySetI>);
        virtual ~FilterManager();

        void destroy();

        [[nodiscard]] std::shared_ptr<StringSetI> categories() const { return _categories; }

        [[nodiscard]] std::shared_ptr<StringSetI> adapterIds() const { return _adapters; }

        [[nodiscard]] std::shared_ptr<IdentitySetI> identities() const { return _identities; }

        [[nodiscard]] std::optional<StringSetPrx> categoriesPrx() const { return _categoriesPrx; }

        [[nodiscard]] std::optional<StringSetPrx> adapterIdsPrx() const { return _adapterIdsPrx; }

        [[nodiscard]] std::optional<IdentitySetPrx> identitiesPrx() const { return _identitiesPrx; }

        static std::shared_ptr<FilterManager> create(std::shared_ptr<Instance>, const std::string&, bool);

    private:
        std::optional<StringSetPrx> _categoriesPrx;
        std::optional<StringSetPrx> _adapterIdsPrx;
        std::optional<IdentitySetPrx> _identitiesPrx;

        const std::shared_ptr<StringSetI> _categories;
        const std::shared_ptr<StringSetI> _adapters;
        const std::shared_ptr<IdentitySetI> _identities;
        const std::shared_ptr<Instance> _instance;
    };
}

#endif /* FILTER_MANAGER_H */
