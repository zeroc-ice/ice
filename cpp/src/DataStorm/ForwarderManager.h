//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DATASTORM_FORWARDER_MANAGER_H
#define DATASTORM_FORWARDER_MANAGER_H

#include "DataStorm/Config.h"
#include "Ice/Ice.h"

#include <functional>
#include <optional>

namespace DataStormI
{

    class Instance;

    class ForwarderManager : public Ice::BlobjectAsync
    {
    public:
        using Response = std::function<void(bool, const Ice::ByteSeq&)>;
        using Exception = std::function<void(std::exception_ptr)>;

        ForwarderManager(const Ice::ObjectAdapterPtr&, const std::string&);
        Ice::ObjectPrx add(std::function<void(Ice::ByteSeq, Response, Exception, const Ice::Current&)>);
        Ice::ObjectPrx add(std::function<void(Ice::ByteSeq, const Ice::Current&)>);
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
