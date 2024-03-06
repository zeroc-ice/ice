//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Object.h>
#include <Ice/Incoming.h>
#include <Ice/IncomingAsync.h>
#include <Ice/IncomingRequest.h>
#include <Ice/LocalException.h>
#include <Ice/SlicedData.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace Ice
{
const Current emptyCurrent = Current();
}

Ice::Request::~Request()
{
    // Out of line to avoid weak vtable
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
    static const vector<string> allTypeIds = { "::Ice::Object" };
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
Ice::Object::_iceD_ice_isA(Incoming& inS, const Current& current)
{
    InputStream* istr = inS.startReadParams();
    string iceP_id;
    istr->read(iceP_id, false);
    inS.endReadParams();
    bool ret = ice_isA(std::move(iceP_id), current);
    OutputStream* ostr = inS.startWriteParams();
    ostr->write(ret);
    inS.endWriteParams();
    return true;
}

bool
Ice::Object::_iceD_ice_ping(Incoming& inS, const Current& current)
{
    inS.readEmptyParams();
    ice_ping(current);
    inS.writeEmptyParams();
    return true;
}

bool
Ice::Object::_iceD_ice_ids(Incoming& inS, const Current& current)
{
    inS.readEmptyParams();
    vector<string> ret = ice_ids(current);
    OutputStream* ostr = inS.startWriteParams();
    if(ret.empty())
    {
        ostr->write(ret);
    }
    else
    {
        ostr->write(&ret[0], &ret[0] + ret.size(), false);
    }
    inS.endWriteParams();
    return true;
}

bool
Ice::Object::_iceD_ice_id(Incoming& inS, const Current& current)
{
    inS.readEmptyParams();
    string ret = ice_id(current);
    OutputStream* ostr = inS.startWriteParams();
    ostr->write(ret, false);
    inS.endWriteParams();
    return true;
}

bool
Ice::Object::_iceDispatch(Incoming& in, const Current& current)
{
    static constexpr string_view allOperations[] =
    {
        "ice_id",
        "ice_ids",
        "ice_isA",
        "ice_ping"
    };

    pair<const string_view*, const string_view*> r = equal_range(allOperations, allOperations + 4, current.operation);

    if(r.first == r.second)
    {
        throw OperationNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }

    switch(r.first - allOperations)
    {
        case 0:
        {
            return _iceD_ice_id(in, current);
        }
        case 1:
        {
            return _iceD_ice_ids(in, current);
        }
        case 2:
        {
            return _iceD_ice_isA(in, current);
        }
        case 3:
        {
            return _iceD_ice_ping(in, current);
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

string
operationModeToString(OperationMode mode)
{
    switch(mode)
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
    if(expected != received)
    {
        assert(expected != OperationMode::Nonmutating); // We never expect Nonmutating
        if(expected == OperationMode::Idempotent && received == OperationMode::Nonmutating)
        {
            //
            // Fine: typically an old client still using the deprecated nonmutating keyword
            //
        }
        else
        {
            std::ostringstream reason;
            reason << "unexpected operation mode. expected = "
                   << operationModeToString(expected)
                   << " received = "
                   << operationModeToString(received);
            throw Ice::MarshalException(__FILE__, __LINE__, reason.str());
        }
    }
}

bool
Ice::Blobject::_iceDispatch(Incoming& in, const Current& current)
{
    const uint8_t* inEncaps;
    int32_t sz;
    in.readParamEncaps(inEncaps, sz);
    vector<uint8_t> outEncaps;
    bool ok = ice_invoke(vector<uint8_t>(inEncaps, inEncaps + sz), outEncaps, current);
    if(outEncaps.empty())
    {
        in.writeParamEncaps(0, 0, ok);
    }
    else
    {
        in.writeParamEncaps(&outEncaps[0], static_cast<int32_t>(outEncaps.size()), ok);
    }
    return true;
}

bool
Ice::BlobjectArray::_iceDispatch(Incoming& in, const Current& current)
{
    pair<const uint8_t*, const uint8_t*> inEncaps;
    int32_t sz;
    in.readParamEncaps(inEncaps.first, sz);
    inEncaps.second = inEncaps.first + sz;
    vector<uint8_t> outEncaps;
    bool ok = ice_invoke(inEncaps, outEncaps, current);
    if(outEncaps.empty())
    {
        in.writeParamEncaps(0, 0, ok);
    }
    else
    {
        in.writeParamEncaps(&outEncaps[0], static_cast<int32_t>(outEncaps.size()), ok);
    }
    return true;
}

bool
Ice::BlobjectAsync::_iceDispatch(Incoming& in, const Current& current)
{
    const uint8_t* inEncaps;
    int32_t sz;
    in.readParamEncaps(inEncaps, sz);
    auto async = IncomingAsync::create(in);
    ice_invokeAsync(vector<uint8_t>(inEncaps, inEncaps + sz),
                    [async](bool ok, const vector<uint8_t>& outEncaps)
                    {
                        if(outEncaps.empty())
                        {
                            async->writeParamEncaps(0, 0, ok);
                        }
                        else
                        {
                            async->writeParamEncaps(&outEncaps[0], static_cast<int32_t>(outEncaps.size()), ok);
                        }
                        async->completed();
                    },
                    async->exception(), current);
    return false;
}

bool
Ice::BlobjectArrayAsync::_iceDispatch(Incoming& in, const Current& current)
{
    pair<const uint8_t*, const uint8_t*> inEncaps;
    int32_t sz;
    in.readParamEncaps(inEncaps.first, sz);
    inEncaps.second = inEncaps.first + sz;
    auto async = IncomingAsync::create(in);
    ice_invokeAsync(inEncaps,
                    [async](bool ok, const pair<const uint8_t*, const uint8_t*>& outE)
                    {
                        async->writeParamEncaps(outE.first, static_cast<int32_t>(outE.second - outE.first), ok);
                        async->completed();
                    },
                    async->exception(), current);
    return false;
}
