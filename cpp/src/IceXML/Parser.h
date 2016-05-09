// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_XML_PARSER_H
#define ICE_XML_PARSER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <IceUtil/Exception.h>

#include <vector>
#include <map>

#ifndef ICE_XML_API
#    if defined(ICE_STATIC_LIBS)
#       define ICE_XML_API /**/
#    elif defined(ICE_XML_API_EXPORTS)
#       define ICE_XML_API ICE_DECLSPEC_EXPORT
#    else
#       define ICE_XML_API ICE_DECLSPEC_IMPORT
#    endif
#endif

//
// Automatically link IceXML[D|++11|++11D].lib with Visual C++
//

#if !defined(ICE_BUILDING_ICE_XML) && defined(ICE_XML_API_EXPORTS)
#   define ICE_BUILDING_ICE_XML
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_XML)
#   pragma comment(lib, ICE_LIBNAME("IceXML"))
#endif

namespace IceXML
{

class ICE_XML_API ParserException : public IceUtil::Exception
{
public:

    ParserException(const std::string&);
    ParserException(const char*, int, const std::string&);
    virtual ~ParserException() ICE_NOEXCEPT;

    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;
#ifndef ICE_CPP11_MAPPING
    virtual ParserException* ice_clone() const;
#endif
    virtual void ice_throw() const;

    std::string reason() const;

private:

    std::string _reason;
    static const char* _name;
};

class Node;
typedef IceUtil::Handle< Node > NodePtr;

typedef std::vector<NodePtr> NodeList;

class Element;
typedef IceUtil::Handle< Element > ElementPtr;

class Text;
typedef IceUtil::Handle< Text > TextPtr;

class Document;
typedef IceUtil::Handle< Document > DocumentPtr;

typedef std::map<std::string, std::string> Attributes;

class ICE_XML_API Node : public IceUtil::Shared
{
public:

    virtual ~Node();

    virtual NodePtr getParent() const;
    virtual std::string getName() const;
    virtual std::string getValue() const;
    virtual NodeList getChildren() const;
    virtual Attributes getAttributes() const;
    virtual std::string getAttribute(const std::string&) const;

    virtual bool addChild(const NodePtr&);

    virtual void destroy();

    int getLine() const;
    int getColumn() const;

protected:

    Node(const NodePtr&, const std::string&, const std::string&, int, int);

    NodePtr _parent;
    std::string _name;
    std::string _value;
    int _line;
    int _column;
};

class ICE_XML_API Element : public Node
{
public:

    Element(const NodePtr&, const std::string&, const Attributes&, int, int);
    virtual ~Element();

    virtual NodeList getChildren() const;
    virtual Attributes getAttributes() const;
    virtual std::string getAttribute(const std::string&) const;

    virtual bool addChild(const NodePtr&);

    virtual void destroy();

private:

    NodeList _children;
    Attributes _attributes;
};

class ICE_XML_API Text : public Node
{
public:

    Text(const NodePtr&, const std::string&, int, int);
    virtual ~Text();
};

class ICE_XML_API Document : public Node
{
public:

    Document();
    virtual ~Document();

    virtual NodeList getChildren() const;

    virtual bool addChild(const NodePtr&);

    virtual void destroy();

private:

    NodeList _children;
};

class ICE_XML_API Handler
{
public:

    virtual ~Handler();

    virtual void startElement(const std::string&, const Attributes&, int, int) = 0;
    virtual void endElement(const std::string&, int, int) = 0;
    virtual void characters(const std::string&, int, int) = 0;
    virtual void error(const std::string&, int, int);
};

class ICE_XML_API Parser
{
public:

    static DocumentPtr parse(const std::string&); // The given filename must be UTF-8 encoded
    static DocumentPtr parse(std::istream&);

    static void parse(const std::string&, Handler&);
    static void parse(std::istream&, Handler&);
};

}

#endif
