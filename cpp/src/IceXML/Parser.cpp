// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceXML/Parser.h>
#include <IceUtil/FileUtil.h>
#include <expat.h>
#include <list>
#include <fstream>

using namespace std;
using namespace IceXML;

//
// ParserException
//
IceXML::ParserException::ParserException(const string& reason) :
    _reason(reason)
{
}

IceXML::ParserException::ParserException(const char* file, int line, const string& reason) :
    IceUtil::ExceptionHelper<ParserException>(file, line), _reason(reason)
{
}

#ifndef ICE_CPP11_COMPILER
IceXML::ParserException::~ParserException() throw()
{
}
#endif

string
IceXML::ParserException::ice_id() const
{
    return "::IceXML::ParserException";
}

void
IceXML::ParserException::ice_print(std::ostream& out) const
{
    Exception::ice_print(out);
    if(!_reason.empty())
    {
        out << "\n" << _reason;
    }
    else
    {
        out << ":\nXML parser exception";
    }
}

#ifndef ICE_CPP11_MAPPING
IceXML::ParserException*
IceXML::ParserException::ice_clone() const
{
    return new ParserException(*this);
}
#endif

string
IceXML::ParserException::reason() const
{
    return _reason;
}

//
// Node
//
IceXML::Node::Node(const NodePtr& parent, const string& name, const string& value, int line, int column) :
    _parent(parent), _name(name), _value(value), _line(line), _column(column)
{
}

IceXML::Node::~Node()
{
}

IceXML::NodePtr
IceXML::Node::getParent() const
{
    return _parent;
}

string
IceXML::Node::getName() const
{
    return _name;
}

string
IceXML::Node::getValue() const
{
    return _value;
}

IceXML::NodeList
IceXML::Node::getChildren() const
{
    return NodeList();
}

IceXML::Attributes
IceXML::Node::getAttributes() const
{
    return Attributes();
}

string
IceXML::Node::getAttribute(const string&) const
{
    return string();
}

bool
IceXML::Node::addChild(const NodePtr&)
{
    return false;
}

void
IceXML::Node::destroy()
{
    _parent = 0;
}

int
IceXML::Node::getLine() const
{
    return _line;
}

int
IceXML::Node::getColumn() const
{
    return _column;
}

//
// Element
//
IceXML::Element::Element(const NodePtr& parent, const string& name, const Attributes& attributes, int line,
                         int column) :
    Node(parent, name, "", line, column), _attributes(attributes)
{
}

IceXML::Element::~Element()
{
}

IceXML::NodeList
IceXML::Element::getChildren() const
{
    return _children;
}

IceXML::Attributes
IceXML::Element::getAttributes() const
{
    return _attributes;
}

string
IceXML::Element::getAttribute(const string& name) const
{
    Attributes::const_iterator p = _attributes.find(name);
    if(p != _attributes.end())
    {
        return p->second;
    }
    return string();
}

bool
IceXML::Element::addChild(const NodePtr& child)
{
    _children.push_back(child);
    return true;
}

void
IceXML::Element::destroy()
{
    Node::destroy();
    for(NodeList::iterator p = _children.begin(); p != _children.end(); ++p)
    {
        (*p)->destroy();
    }
}

//
// Text
//
IceXML::Text::Text(const NodePtr& parent, const string& value, int line, int column) :
    Node(parent, "", value, line, column)
{
}

IceXML::Text::~Text()
{
}

//
// Document
//
IceXML::Document::Document() :
    Node(0, "", "", 0, 0)
{
}

IceXML::Document::~Document()
{
}

IceXML::NodeList
IceXML::Document::getChildren() const
{
    return _children;
}

bool
IceXML::Document::addChild(const NodePtr& child)
{
    _children.push_back(child);
    return true;
}

void
IceXML::Document::destroy()
{
    Node::destroy();
    for(NodeList::iterator p = _children.begin(); p != _children.end(); ++p)
    {
        (*p)->destroy();
    }
}

//
// Handler
//
IceXML::Handler::~Handler()
{
}

void
IceXML::Handler::error(const string& msg, int line, int column)
{
    ostringstream out;
    out << "XML error at input line " << line << ", column " << column << ":" << endl << msg;
    throw ParserException(__FILE__, __LINE__, out.str());
}

//
// DocumentBuilder
//
namespace IceXML
{

class DocumentBuilder : public Handler
{
public:

    DocumentBuilder();

    virtual void startElement(const string&, const Attributes&, int, int);
    virtual void endElement(const string&, int, int);
    virtual void characters(const string&, int, int);

    DocumentPtr getDocument() const;

private:

    list<NodePtr> _nodeStack;
    DocumentPtr _document;
};

}

IceXML::DocumentBuilder::DocumentBuilder()
{
    _document = new Document;
    _nodeStack.push_front(_document);
}

void
IceXML::DocumentBuilder::startElement(const string& name, const Attributes& attributes, int line, int column)
{
    NodePtr parent = _nodeStack.front();

    Element* element = new Element(parent, name, attributes, line, column);
#ifdef NDEBUG
    parent->addChild(element);
#else
    assert(parent->addChild(element));
#endif

    _nodeStack.push_front(element);
}

void
IceXML::DocumentBuilder::endElement(const string&, int, int)
{
    assert(!_nodeStack.empty());
    _nodeStack.pop_front();
}

void
IceXML::DocumentBuilder::characters(const string& data, int line, int column)
{
    NodePtr parent = _nodeStack.front();
    TextPtr text = new Text(parent, data, line, column);
    parent->addChild(text);
}

DocumentPtr
IceXML::DocumentBuilder::getDocument() const
{
    return _document;
}

//
// expat callbacks
//
struct CallbackData
{
    XML_Parser parser;
    Handler* handler;
};

extern "C"
{

static void
startElementHandler(void* data, const XML_Char* name, const XML_Char** attr)
{
    CallbackData* cb = static_cast<CallbackData*>(data);

    Attributes attributes;
    for(int i = 0; attr[i]; i += 2)
    {
        attributes[attr[i]] = attr[i + 1];
    }

    int line = static_cast<int>(XML_GetCurrentLineNumber(cb->parser));
    int column = static_cast<int>(XML_GetCurrentColumnNumber(cb->parser));
    cb->handler->startElement(name, attributes, line, column);
}

static void
endElementHandler(void* data, const XML_Char* name)
{
    CallbackData* cb = static_cast<CallbackData*>(data);
    int line = static_cast<int>(XML_GetCurrentLineNumber(cb->parser));
    int column = static_cast<int>(XML_GetCurrentColumnNumber(cb->parser));
    cb->handler->endElement(name, line, column);
}

static void
characterDataHandler(void* data, const XML_Char* s, int len)
{
    CallbackData* cb = static_cast<CallbackData*>(data);

    string str(s, len);
    int line = static_cast<int>(XML_GetCurrentLineNumber(cb->parser));
    int column = static_cast<int>(XML_GetCurrentColumnNumber(cb->parser));
    cb->handler->characters(str, line, column);
}

}

//
// Parser
//
IceXML::DocumentPtr
IceXML::Parser::parse(const string& file)
{
    DocumentBuilder builder;
    parse(file, builder);
    return builder.getDocument();
}

IceXML::DocumentPtr
IceXML::Parser::parse(istream& in)
{
    DocumentBuilder builder;
    parse(in, builder);
    return builder.getDocument();
}

void
IceXML::Parser::parse(const string& file, Handler& handler) // The given filename must be UTF-8 encoded
{
    ifstream in(IceUtilInternal::streamFilename(file).c_str());
    if(!in.good())
    {
        ostringstream out;
        out << "unable to open file `" << file << "'";
        throw ParserException(__FILE__, __LINE__, out.str());
    }
    parse(in, handler);
}

void
IceXML::Parser::parse(istream& in, Handler& handler)
{
    XML_Parser parser = XML_ParserCreate(ICE_NULLPTR);
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
        while(!isFinal)
        {
            in.read(buff, 1024);
            if(in.gcount() < 1024)
            {
                isFinal = 1;
            }
            if(XML_Parse(parser, buff, static_cast<int>(in.gcount()), isFinal) != 1)
            {
                handler.error(XML_ErrorString(XML_GetErrorCode(parser)),
                              static_cast<int>(XML_GetCurrentLineNumber(parser)),
                              static_cast<int>(XML_GetCurrentColumnNumber(parser)));
                return;
            }
        }
    }
    catch(...)
    {
        XML_ParserFree(parser);
        throw;
    }

    XML_ParserFree(parser);
}
