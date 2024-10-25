//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DATASTORM_FORWARDER_MANAGER_H
#define DATASTORM_FORWARDER_MANAGER_H

#include "DataStorm/Config.h"
#include "Ice/Ice.h"

#include <functional>
#include <mutex>

namespace DataStormI
{
    class ForwarderManager : public Ice::BlobjectAsync
    {
    public:
        using Response = std::function<void(bool, const Ice::ByteSeq&)>;
        using Exception = std::function<void(std::exception_ptr)>;

        ForwarderManager(const Ice::ObjectAdapterPtr&, const std::string&);

        template<typename Prx, std::enable_if_t<std::is_base_of<Ice::ObjectPrx, Prx>::value, bool> = true>
        Prx add(std::function<void(Ice::ByteSeq, Response, Exception, const Ice::Current&)> forwarder)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            const Ice::Identity id = {std::to_string(_nextId++), _category};
            _forwarders.emplace(id.name, std::move(forwarder));
            return _adapter->createProxy<Prx>(id);
        }

        template<typename Prx, std::enable_if_t<std::is_base_of<Ice::ObjectPrx, Prx>::value, bool> = true>
        Prx add(std::function<void(Ice::ByteSeq, const Ice::Current&)> forwarder)
        {
            return add<Prx>(
                [forwarder = std::move(forwarder)](
                    Ice::ByteSeq inParams,
                    Response response,
                    Exception exception,
                    const Ice::Current& current)
                {
                    try
                    {
                        forwarder(std::move(inParams), current);
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
        virtual void ice_invokeAsync(
            Ice::ByteSeq,
            std::function<void(bool, const Ice::ByteSeq&)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&);

        const Ice::ObjectAdapterPtr _adapter;
        const std::string _category;

        std::mutex _mutex;
        std::map<std::string, std::function<void(Ice::ByteSeq, Response, Exception, const Ice::Current&)>> _forwarders;
        unsigned int _nextId;
    };
}
#endif
