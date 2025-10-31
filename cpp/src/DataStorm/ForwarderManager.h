// Copyright (c) ZeroC, Inc.

#ifndef DATASTORM_FORWARDER_MANAGER_H
#define DATASTORM_FORWARDER_MANAGER_H

#include "DataStorm/Config.h"
#include "Ice/Ice.h"

#include <functional>
#include <mutex>

namespace DataStormI
{
    class ForwarderManager final : public Ice::BlobjectAsync
    {
    public:
        using Response = std::function<void(bool, const Ice::ByteSeq&)>;
        using Exception = std::function<void(std::exception_ptr)>;

        ForwarderManager(Ice::ObjectAdapterPtr, std::string);

        template<typename Prx, std::enable_if_t<std::is_base_of_v<Ice::ObjectPrx, Prx>, bool> = true>
        Prx add(std::function<void(Ice::ByteSeq, const Response&, const Exception&, const Ice::Current&)> forwarder)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            const Ice::Identity id{.name = std::to_string(_nextId++), .category = _category};
            _forwarders.emplace(id.name, std::move(forwarder));
            Prx prx{_adapter->createProxy<Prx>(std::move(id))};
            // disable invocation timeout for collocated forwarders
            return prx->ice_invocationTimeout(std::chrono::milliseconds::zero());
        }

        template<typename Prx, std::enable_if_t<std::is_base_of_v<Ice::ObjectPrx, Prx>, bool> = true>
        Prx add(std::function<void(const Ice::ByteSeq&, const Ice::Current&)> forwarder)
        {
            return add<Prx>(
                [forwarder = std::move(forwarder)](
                    const Ice::ByteSeq& inParams,
                    const Response& response,
                    const Exception& exception,
                    const Ice::Current& current)
                {
                    try
                    {
                        forwarder(inParams, current);
                        response(true, {});
                    }
                    catch (...)
                    {
                        exception(std::current_exception());
                    }
                });
        }

        void remove(const Ice::Identity&);

        void destroy();

    private:
        void ice_invokeAsync(
            Ice::ByteSeq,
            std::function<void(bool, const Ice::ByteSeq&)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final;

        const Ice::ObjectAdapterPtr _adapter;
        const std::string _category;

        std::mutex _mutex;
        std::map<std::string, std::function<void(Ice::ByteSeq, const Response&, const Exception&, const Ice::Current&)>>
            _forwarders;
        unsigned int _nextId{0};
    };
}
#endif
