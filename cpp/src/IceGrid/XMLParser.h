// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_XML_PARSER_H
#define ICEGRID_XML_PARSER_H

#include "Ice/LocalException.h"

#include <map>

namespace IceGrid
{
    class XMLParserException final : public Ice::LocalException
    {
    public:
        using Ice::LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    using XMLAttributes = std::map<std::string, std::string>;

    class XMLHandler
    {
    public:
        virtual ~XMLHandler() = 0;
        virtual void startElement(const std::string&, const XMLAttributes&, int, int) = 0;
        virtual void endElement(const std::string&, int, int) = 0;
        virtual void characters(const std::string&, int, int) = 0;
        virtual void error(const std::string&, int, int) = 0;
    };

    class XMLParser
    {
    public:
        static void parse(const std::string&, XMLHandler&);
        static void parse(std::istream&, XMLHandler&);
    };
}

#endif
