//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_HTTP_PARSER_H
#define ICE_HTTP_PARSER_H

#include "Ice/Config.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace IceInternal
{
    std::vector<unsigned char> calcSHA1(const std::vector<unsigned char>&);

    using HeaderFields = std::map<std::string, std::pair<std::string, std::string>>;

    class WebSocketException
    {
    public:
        WebSocketException(const std::string&);

        std::string reason;
    };

    class HttpParser
    {
    public:
        HttpParser();

        enum Type
        {
            TypeUnknown,
            TypeRequest,
            TypeResponse
        };

        const std::byte* isCompleteMessage(const std::byte*, const std::byte*) const;

        bool parse(const std::byte*, const std::byte*);

        Type type() const;

        std::string method() const;
        std::string uri() const;
        int versionMajor() const;
        int versionMinor() const;

        int status() const;
        std::string reason() const;

        bool getHeader(const std::string&, std::string&, bool) const;

        std::map<std::string, std::string> getHeaders() const;

    private:
        Type _type;

        std::string _method;
        std::string _uri;

        HeaderFields _headers;
        std::string _headerName;

        int _versionMajor;
        int _versionMinor;

        int _status;
        std::string _reason;

        enum State
        {
            StateInit,
            StateType,
            StateTypeCheck,
            StateRequest,
            StateRequestMethod,
            StateRequestMethodSP,
            StateRequestURI,
            StateRequestURISP,
            StateRequestLF,
            StateHeaderFieldStart,
            StateHeaderFieldContStart,
            StateHeaderFieldCont,
            StateHeaderFieldNameStart,
            StateHeaderFieldName,
            StateHeaderFieldNameEnd,
            StateHeaderFieldValueStart,
            StateHeaderFieldValue,
            StateHeaderFieldValueEnd,
            StateHeaderFieldLF,
            StateHeaderFieldEndLF,
            StateVersion,
            StateVersionH,
            StateVersionHT,
            StateVersionHTT,
            StateVersionHTTP,
            StateVersionMajor,
            StateVersionMinor,
            StateResponse,
            StateResponseVersionSP,
            StateResponseStatus,
            StateResponseReasonStart,
            StateResponseReason,
            StateResponseLF,
            StateComplete
        };
        State _state;
    };
    using HttpParserPtr = std::shared_ptr<HttpParser>;
}

#endif
