// Copyright (c) ZeroC, Inc.

#include "Ice/IncomingRequest.h"
#include "Ice/InputStream.h"
#include "Ice/LocalExceptions.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

IncomingRequest::IncomingRequest(
    int32_t requestId,
    ConnectionPtr connection,
    ObjectAdapterPtr adapter,
    InputStream& inputStream,
    int32_t requestCount)
    : _inputStream(inputStream),
      _requestCount(requestCount)
{
    _current.adapter = std::move(adapter);
    _current.con = std::move(connection);
    _current.requestId = requestId;

    // Read everything else from the input stream.
    auto start = inputStream.i;
    inputStream.read(_current.id);

    vector<string> facetPath;
    inputStream.read(facetPath);
    if (!facetPath.empty())
    {
        if (facetPath.size() > 1)
        {
            throw MarshalException{__FILE__, __LINE__, "received facet path with more than one element"};
        }
        _current.facet = facetPath[0];
    }

    inputStream.read(_current.operation, false);

    uint8_t mode;
    inputStream.read(mode);
    _current.mode = static_cast<OperationMode>(mode);

    int32_t sz = inputStream.readSize();
    while (sz--)
    {
        string key;
        string value;
        inputStream.read(key);
        inputStream.read(value);
        _current.ctx.emplace(std::move(key), std::move(value));
    }

    int32_t encapsulationSize;
    inputStream.read(encapsulationSize);
    EncodingVersion encoding;
    inputStream.read(encoding.major);
    inputStream.read(encoding.minor);
    _current.encoding = encoding;

    // Rewind to the start of the encapsulation
    inputStream.i -= 6;

    _requestSize = static_cast<int32_t>(inputStream.i - start) + encapsulationSize;
}
