// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IceXML/Parser.h>
#include <expat.h>
#include <list>
#include <fstream>

using namespace std;
using namespace IceXML;

//
// ParserException
//
IceXML::ParserException::ParserException(const string& reason) :
    IceUtil::Exception(), _reason(reason)
{
}

IceXML::ParserException::ParserException(const char* file, int line, const string& reason) :
    IceUtil::Exception(file, line), _reason(reason)
{
}

string
IceXML::ParserException::ice_name() const
{
    return "IceXML::ParserException";
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

IceUtil::Exception*
IceXML::ParserException::ice_clone() const
{
    return new ParserException(*this);
}

void
IceXML::ParserException::ice_throw() const
{
    throw *this;
}

//
// Node
//
IceXML::Node::Node(const NodePtr& parent, const string& name, const string& value) :
    _parent(parent), _name(name), _value(value)
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
IceXML::Node::getAttribute(const string& name) const
{
    return string();
}

bool
IceXML::Node::addChild(const NodePtr& child)
{
    return false;
}

//
// Element
//
IceXML::Element::Element(const NodePtr& parent, const string& name, const Attributes& attributes) :
    Node(parent, name, ""), _attributes(attributes)
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

//
// Text
//
IceXML::Text::Text(const NodePtr& parent, const string& value) :
    Node(parent, "", value)
{
}

IceXML::Text::~Text()
{
}

//
// Document
//
IceXML::Document::Document() :
    Node(0, "", "")
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

//
// Handler
//
IceXML::Handler::~Handler()
{
}

void
IceXML::Handler::error(const string& msg, int line, int col)
{
    ostringstream out;
    out << "XML error at input line " << line << ", column " << col << ":" << endl << msg;
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

    virtual void startElement(const string&, const Attributes&);
    virtual void endElement(const string&);
    virtual void characters(const string&);

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
IceXML::DocumentBuilder::startElement(const string& name, const Attributes& attributes)
{
    NodePtr parent = _nodeStack.front();

    Element* element = new Element(parent, name, attributes);
    bool b = parent->addChild(element);
    assert(b);

    _nodeStack.push_front(element);
}

void
IceXML::DocumentBuilder::endElement(const string& name)
{
    assert(!_nodeStack.empty());
    _nodeStack.pop_front();
}

void
IceXML::DocumentBuilder::characters(const string& data)
{
    NodePtr parent = _nodeStack.front();
    TextPtr text = new Text(parent, data);
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
static void
startElementHandler(void* data, const XML_Char* name, const XML_Char** attr)
{
    Handler* handler = static_cast<Handler*>(data);

    Attributes attributes;
    for(int i = 0; attr[i]; i += 2)
    {
        attributes[attr[i]] = attr[i + 1];
    }

    handler->startElement(name, attributes);
}

static void
endElementHandler(void* data, const XML_Char* name)
{
    Handler* handler = static_cast<Handler*>(data);
    handler->endElement(name);
}

static void
characterDataHandler(void* data, const XML_Char* s, int len)
{
    Handler* handler = static_cast<Handler*>(data);

    string str(s, len);
    handler->characters(str);
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
IceXML::Parser::parse(const string& file, Handler& handler)
{
    ifstream in(file.c_str());
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
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, &handler);
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
            if(XML_Parse(parser, buff, in.gcount(), isFinal) != 1)
            {
                handler.error(XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser),
                              XML_GetCurrentColumnNumber(parser));
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
