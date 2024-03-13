//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Object.h"
#include "Ice/Incoming.h"
#include "Ice/LocalException.h"
#include "Ice/SlicedData.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace Ice
{
    const Current emptyCurrent = Current();
}

bool
Ice::Object::ice_isA(string s, const Current& current) const
{
    vector<string> allTypeIds = ice_ids(current); // sorted type IDs
    return ::std::binary_search(allTypeIds.begin(), allTypeIds.end(), s);
}

void
Ice::Object::ice_ping(const Current&) const
{
    // Nothing to do.
}

vector<string>
Ice::Object::ice_ids(const Current&) const
{
    static const vector<string> allTypeIds = {"::Ice::Object"};
    return allTypeIds;
}

string
Ice::Object::ice_id(const Current&) const
{
    return string{ice_staticId()};
}

string_view
Ice::Object::ice_staticId()
{
    static constexpr std::string_view typeId = "::Ice::Object";
    return typeId;
}

bool
Ice::Object::_iceD_ice_isA(Incoming& incoming)
{
    InputStream* istr = incoming.startReadParams();
    string iceP_id;
    istr->read(iceP_id, false);
    incoming.endReadParams();
    bool ret = ice_isA(std::move(iceP_id), incoming.current());
    OutputStream* ostr = incoming.startWriteParams();
    ostr->write(ret);
    incoming.endWriteParams();
    return true;
}

bool
Ice::Object::_iceD_ice_ping(Incoming& incoming)
{
    incoming.readEmptyParams();
    ice_ping(incoming.current());
    incoming.writeEmptyParams();
    return true;
}

bool
Ice::Object::_iceD_ice_ids(Incoming& incoming)
{
    incoming.readEmptyParams();
    vector<string> ret = ice_ids(incoming.current());
    OutputStream* ostr = incoming.startWriteParams();
    if (ret.empty())
    {
        ostr->write(ret);
    }
    else
    {
        ostr->write(&ret[0], &ret[0] + ret.size(), false);
    }
    incoming.endWriteParams();
    return true;
}

bool
Ice::Object::_iceD_ice_id(Incoming& incoming)
{
    incoming.readEmptyParams();
    string ret = ice_id(incoming.current());
    OutputStream* ostr = incoming.startWriteParams();
    ostr->write(ret, false);
    incoming.endWriteParams();
    return true;
}

bool
Ice::Object::_iceDispatch(Incoming& incoming)
{
    static constexpr string_view allOperations[] = {"ice_id", "ice_ids", "ice_isA", "ice_ping"};

    const Current& current = incoming.current();

    pair<const string_view*, const string_view*> r = equal_range(allOperations, allOperations + 4, current.operation);

    if (r.first == r.second)
    {
        throw OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }

    switch (r.first - allOperations)
    {
        case 0:
        {
            return _iceD_ice_id(incoming);
        }
        case 1:
        {
            return _iceD_ice_ids(incoming);
        }
        case 2:
        {
            return _iceD_ice_isA(incoming);
        }
        case 3:
        {
            return _iceD_ice_ping(incoming);
        }
        default:
        {
            assert(false);
            throw OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
        }
    }
}

namespace
{
    string operationModeToString(OperationMode mode)
    {
        switch (mode)
        {
            case OperationMode::Normal:
                return "::Ice::Normal";

            case OperationMode::Nonmutating:
                return "::Ice::Nonmutating";

            case OperationMode::Idempotent:
                return "::Ice::Idempotent";
        }
        //
        // This could not happen with C++11 strong type enums
        //
        assert(false);
        return "";
    }
}

void
Ice::Object::_iceCheckMode(OperationMode expected, OperationMode received)
{
    if (expected != received)
    {
        assert(expected != OperationMode::Nonmutating); // We never expect Nonmutating
        if (expected == OperationMode::Idempotent && received == OperationMode::Nonmutating)
        {
            //
            // Fine: typically an old client still using the deprecated nonmutating keyword
            //
        }
        else
        {
            std::ostringstream reason;
            reason << "unexpected operation mode. expected = " << operationModeToString(expected)
                   << " received = " << operationModeToString(received);
            throw Ice::MarshalException(__FILE__, __LINE__, reason.str());
        }
    }
}

bool
Ice::Blobject::_iceDispatch(Incoming& incoming)
{
    const Current& current = incoming.current();
    const byte* inEncaps;
    int32_t sz;
    incoming.readParamEncaps(inEncaps, sz);
    vector<byte> outEncaps;
    bool ok = ice_invoke(vector<byte>(inEncaps, inEncaps + sz), outEncaps, current);
    if (outEncaps.empty())
    {
        incoming.writeParamEncaps(0, 0, ok);
    }
    else
    {
        incoming.writeParamEncaps(&outEncaps[0], static_cast<int32_t>(outEncaps.size()), ok);
    }
    return true;
}

bool
Ice::BlobjectArray::_iceDispatch(Incoming& incoming)
{
    const Current& current = incoming.current();
    pair<const byte*, const byte*> inEncaps;
    int32_t sz;
    incoming.readParamEncaps(inEncaps.first, sz);
    inEncaps.second = inEncaps.first + sz;
    vector<byte> outEncaps;
    bool ok = ice_invoke(inEncaps, outEncaps, current);
    if (outEncaps.empty())
    {
        incoming.writeParamEncaps(0, 0, ok);
    }
    else
    {
        incoming.writeParamEncaps(&outEncaps[0], static_cast<int32_t>(outEncaps.size()), ok);
    }
    return true;
}

bool
Ice::BlobjectAsync::_iceDispatch(Incoming& incoming)
{
    const byte* inEncaps;
    int32_t sz;
    incoming.readParamEncaps(inEncaps, sz);
    auto incomingPtr = make_shared<Incoming>(std::move(incoming));
    try
    {
        ice_invokeAsync(
            vector<byte>(inEncaps, inEncaps + sz),
            [incomingPtr](bool ok, const vector<byte>& outEncaps)
            {
                if (outEncaps.empty())
                {
                    incomingPtr->writeParamEncaps(0, 0, ok);
                }
                else
                {
                    incomingPtr->writeParamEncaps(&outEncaps[0], static_cast<int32_t>(outEncaps.size()), ok);
                }
                incomingPtr->completed();
            },
            [incomingPtr](std::exception_ptr ex) { incomingPtr->completed(ex); },
            incomingPtr->current());
    }
    catch (...)
    {
        incomingPtr->failed(std::current_exception());
    }
    return false;
}

bool
Ice::BlobjectArrayAsync::_iceDispatch(Incoming& incoming)
{
    pair<const byte*, const byte*> inEncaps;
    int32_t sz;
    incoming.readParamEncaps(inEncaps.first, sz);
    inEncaps.second = inEncaps.first + sz;
    auto incomingPtr = make_shared<Incoming>(std::move(incoming));
    try
    {
        ice_invokeAsync(
            inEncaps,
            [incomingPtr](bool ok, const pair<const byte*, const byte*>& outE)
            {
                incomingPtr->writeParamEncaps(outE.first, static_cast<int32_t>(outE.second - outE.first), ok);
                incomingPtr->completed();
            },
            [incomingPtr](std::exception_ptr ex) { incomingPtr->completed(ex); },
            incomingPtr->current());
    }
    catch (...)
    {
        incomingPtr->failed(std::current_exception());
    }
    return false;
}
