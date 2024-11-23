//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_XML_PARSER_H
#define ICE_XML_PARSER_H

#include "Ice/LocalException.h"

#include <map>
#include <vector>

#ifndef ICE_XML_API
#    if defined(ICE_XML_API_EXPORTS)
#        define ICE_XML_API ICE_DECLSPEC_EXPORT
#    else
#        define ICE_XML_API ICE_DECLSPEC_IMPORT
#    endif
#endif

//
// Automatically link IceXML[D].lib with Visual C++
//

#if !defined(ICE_BUILDING_ICE_XML) && defined(ICE_XML_API_EXPORTS)
#    define ICE_BUILDING_ICE_XML
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_XML)
#    pragma comment(lib, ICE_LIBNAME("IceXML"))
#endif

namespace IceXML
{
    class ICE_XML_API ParserException final : public Ice::LocalException
    {
    public:
        using Ice::LocalException::LocalException;

        const char* ice_id() const noexcept final;
    };

    using Attributes = std::map<std::string, std::string>;

    class ICE_XML_API Handler
    {
    public:
        virtual ~Handler();

        virtual void startElement(const std::string&, const Attributes&, int, int) = 0;
        virtual void endElement(const std::string&, int, int) = 0;
        virtual void characters(const std::string&, int, int) = 0;
        virtual void error(const std::string&, int, int) = 0;
    };

    class ICE_XML_API Parser
    {
    public:
        static void parse(const std::string&, Handler&);
        static void parse(std::istream&, Handler&);
    };
}

#endif
