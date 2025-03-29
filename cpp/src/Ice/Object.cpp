// Copyright (c) ZeroC, Inc.

#include "Ice/Object.h"
#include "Ice/AsyncResponseHandler.h"
#include "Ice/InputStream.h"
#include "Ice/LocalExceptions.h"
#include "Ice/OutputStream.h"
#include "Ice/SlicedData.h"

#include <algorithm>
#include <array>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

bool
Ice::Object::ice_isA(
    string typeId, // NOLINT(performance-unnecessary-value-param)
    const Current& current) const
{
    vector<string> allTypeIds = ice_ids(current); // sorted type IDs
    return std::binary_search(allTypeIds.begin(), allTypeIds.end(), typeId);
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

const char*
Ice::Object::ice_staticId() noexcept
{
    return "::Ice::Object";
}

void
Ice::Object::_iceD_ice_isA(
    IncomingRequest& request,
    function<void(OutgoingResponse)> sendResponse) // NOLINT(performance-unnecessary-value-param)
{
    InputStream* istr = &request.inputStream();
    istr->startEncapsulation();
    string iceP_id;
    istr->read(iceP_id, false);
    istr->endEncapsulation();

    bool ret = ice_isA(std::move(iceP_id), request.current());
    sendResponse(makeOutgoingResponse([&](OutputStream* ostr) { ostr->write(ret); }, request.current()));
}

void
Ice::Object::_iceD_ice_ping(
    IncomingRequest& request,
    function<void(OutgoingResponse)> sendResponse) // NOLINT(performance-unnecessary-value-param)
{
    request.inputStream().skipEmptyEncapsulation();
    ice_ping(request.current());
    sendResponse(makeEmptyOutgoingResponse(request.current()));
}

void
Ice::Object::_iceD_ice_ids(
    IncomingRequest& request,
    function<void(OutgoingResponse)> sendResponse) // NOLINT(performance-unnecessary-value-param)
{
    request.inputStream().skipEmptyEncapsulation();
    vector<string> ret = ice_ids(request.current());

    sendResponse(makeOutgoingResponse(
        [&](OutputStream* ostr)
        {
            if (ret.empty())
            {
                ostr->write(ret);
            }
            else
            {
                ostr->write(&ret[0], &ret[0] + ret.size(), false);
            }
        },
        request.current()));
}

void
Ice::Object::_iceD_ice_id(
    IncomingRequest& request,
    function<void(OutgoingResponse)> sendResponse) // NOLINT(performance-unnecessary-value-param)
{
    request.inputStream().skipEmptyEncapsulation();
    string ret = ice_id(request.current());

    sendResponse(makeOutgoingResponse([&](OutputStream* ostr) { ostr->write(ret, false); }, request.current()));
}

void
Ice::Object::dispatch(IncomingRequest& request, std::function<void(OutgoingResponse)> sendResponse)
{
    static constexpr std::array<string_view, 4> allOperations{"ice_id", "ice_ids", "ice_isA", "ice_ping"};

    const Current& current = request.current();
    auto r = equal_range(allOperations.begin(), allOperations.end(), current.operation);

    if (r.first == r.second)
    {
        sendResponse(makeOutgoingResponse(make_exception_ptr(OperationNotExistException{__FILE__, __LINE__}), current));
        return;
    }

    switch (r.first - allOperations.begin())
    {
        case 0:
        {
            _iceD_ice_id(request, std::move(sendResponse));
            break;
        }
        case 1:
        {
            _iceD_ice_ids(request, std::move(sendResponse));
            break;
        }
        case 2:
        {
            _iceD_ice_isA(request, std::move(sendResponse));
            break;
        }
        case 3:
        {
            _iceD_ice_ping(request, std::move(sendResponse));
            break;
        }
        default:
        {
            assert(false);
            sendResponse(
                makeOutgoingResponse(make_exception_ptr(OperationNotExistException{__FILE__, __LINE__}), current));
        }
    }
}

void
Ice::Blobject::dispatch(IncomingRequest& request, std::function<void(OutgoingResponse)> sendResponse)
{
    const Current& current = request.current();
    const byte* inEncaps;
    int32_t sz;
    request.inputStream().readEncapsulation(inEncaps, sz);
    vector<byte> outEncaps;
    bool ok = ice_invoke(vector<byte>(inEncaps, inEncaps + sz), outEncaps, current);

    if (outEncaps.empty())
    {
        sendResponse(makeOutgoingResponse(ok, {nullptr, nullptr}, current));
    }
    else
    {
        sendResponse(makeOutgoingResponse(ok, {outEncaps.data(), outEncaps.data() + outEncaps.size()}, current));
    }
}

void
Ice::BlobjectArray::dispatch(IncomingRequest& request, std::function<void(OutgoingResponse)> sendResponse)
{
    const Current& current = request.current();
    pair<const byte*, const byte*> inEncaps;
    int32_t sz;
    request.inputStream().readEncapsulation(inEncaps.first, sz);
    inEncaps.second = inEncaps.first + sz;
    vector<byte> outEncaps;
    bool ok = ice_invoke(inEncaps, outEncaps, current);

    if (outEncaps.empty())
    {
        sendResponse(makeOutgoingResponse(ok, {nullptr, nullptr}, current));
    }
    else
    {
        sendResponse(makeOutgoingResponse(ok, {outEncaps.data(), outEncaps.data() + outEncaps.size()}, current));
    }
}

void
Ice::BlobjectAsync::dispatch(IncomingRequest& request, std::function<void(OutgoingResponse)> sendResponse)
{
    const byte* inEncaps;
    int32_t sz;
    request.inputStream().readEncapsulation(inEncaps, sz);
    auto responseHandler = make_shared<AsyncResponseHandler>(std::move(sendResponse), request.current());
    try
    {
        ice_invokeAsync(
            vector<byte>{inEncaps, inEncaps + sz},
            [responseHandler](bool ok, const vector<byte>& outEncaps)
            {
                if (outEncaps.empty())
                {
                    responseHandler->sendResponse(ok, {nullptr, nullptr});
                }
                else
                {
                    responseHandler->sendResponse(ok, {outEncaps.data(), outEncaps.data() + outEncaps.size()});
                }
            },
            [responseHandler](std::exception_ptr ex) { responseHandler->sendException(ex); },
            responseHandler->current());
    }
    catch (...)
    {
        responseHandler->sendException(std::current_exception());
    }
}

void
Ice::BlobjectArrayAsync::dispatch(IncomingRequest& request, std::function<void(OutgoingResponse)> sendResponse)
{
    pair<const byte*, const byte*> inEncaps;
    int32_t sz;
    request.inputStream().readEncapsulation(inEncaps.first, sz);
    inEncaps.second = inEncaps.first + sz;
    auto responseHandler = make_shared<AsyncResponseHandler>(std::move(sendResponse), request.current());
    try
    {
        ice_invokeAsync(
            inEncaps,
            [responseHandler](bool ok, pair<const byte*, const byte*> outEncaps)
            { responseHandler->sendResponse(ok, outEncaps); },
            [responseHandler](std::exception_ptr ex) { responseHandler->sendException(ex); },
            responseHandler->current());
    }
    catch (...)
    {
        responseHandler->sendException(std::current_exception());
    }
}
