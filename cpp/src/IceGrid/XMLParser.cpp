// Copyright (c) ZeroC, Inc.

#include "XMLParser.h"
#include "../Ice/FileUtil.h"

#include <expat.h>
#include <fstream>
#include <sstream>

using namespace std;
using namespace IceGrid;

//
// ParserException
//

const char*
IceGrid::XMLParserException::ice_id() const noexcept
{
    return "::IceGrid::XMLParserException";
}

//
// XMLHandler
//
IceGrid::XMLHandler::~XMLHandler() = default;

//
// expat callbacks
//
struct CallbackData
{
    XML_Parser parser;
    XMLHandler* handler;
};

extern "C"
{
    static void startElementHandler(void* data, const XML_Char* name, const XML_Char** attr)
    {
        auto* cb = static_cast<CallbackData*>(data);

        XMLAttributes attributes;
        for (int i = 0; attr[i]; i += 2)
        {
            attributes[attr[i]] = attr[i + 1];
        }

        int line = static_cast<int>(XML_GetCurrentLineNumber(cb->parser));
        int column = static_cast<int>(XML_GetCurrentColumnNumber(cb->parser));
        cb->handler->startElement(name, attributes, line, column);
    }

    static void endElementHandler(void* data, const XML_Char* name)
    {
        auto* cb = static_cast<CallbackData*>(data);
        int line = static_cast<int>(XML_GetCurrentLineNumber(cb->parser));
        int column = static_cast<int>(XML_GetCurrentColumnNumber(cb->parser));
        cb->handler->endElement(name, line, column);
    }

    static void characterDataHandler(void* data, const XML_Char* s, int len)
    {
        auto* cb = static_cast<CallbackData*>(data);

        string str(s, static_cast<size_t>(len));
        int line = static_cast<int>(XML_GetCurrentLineNumber(cb->parser));
        int column = static_cast<int>(XML_GetCurrentColumnNumber(cb->parser));
        cb->handler->characters(str, line, column);
    }
}

//
// XMLParser
//
void
IceGrid::XMLParser::parse(const string& file, XMLHandler& handler) // The given filename must be UTF-8 encoded
{
    ifstream in(IceInternal::streamFilename(file).c_str());
    if (!in.good())
    {
        ostringstream out;
        out << "unable to open file '" << file << "'";
        throw XMLParserException(__FILE__, __LINE__, out.str());
    }
    parse(in, handler);
}

void
IceGrid::XMLParser::parse(istream& in, XMLHandler& handler)
{
    XML_Parser parser = XML_ParserCreate(nullptr);
    CallbackData cb;
    cb.parser = parser;
    cb.handler = &handler;
    XML_SetUserData(parser, &cb);
    XML_SetElementHandler(parser, startElementHandler, endElementHandler);
    XML_SetCharacterDataHandler(parser, characterDataHandler);

    try
    {
        char buff[1024];
        int isFinal = 0;
        while (!isFinal)
        {
            in.read(buff, 1024);
            if (in.gcount() < 1024)
            {
                isFinal = 1;
            }
            if (XML_Parse(parser, buff, static_cast<int>(in.gcount()), isFinal) != 1)
            {
                handler.error(
                    XML_ErrorString(XML_GetErrorCode(parser)),
                    static_cast<int>(XML_GetCurrentLineNumber(parser)),
                    static_cast<int>(XML_GetCurrentColumnNumber(parser)));
                return;
            }
        }
    }
    catch (...)
    {
        XML_ParserFree(parser);
        throw;
    }

    XML_ParserFree(parser);
}
