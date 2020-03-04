//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_XML_PARSER_H
#define ICE_XML_PARSER_H

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

class ICE_XML_API ParserException final : public IceUtil::ExceptionHelper<ParserException>
{
public:

    ParserException(const std::string&);
    ParserException(const char*, int, const std::string&);

    std::string ice_id() const override;
    void ice_print(std::ostream&) const override;

    std::string reason() const;

private:

    std::string _reason;
    static const char* _name;
};

class Document;
class Element;
class Node;
class Text;

using NodeList = std::vector<std::shared_ptr<Node>>;
using Attributes = std::map<std::string, std::string>;

class ICE_XML_API Node
{
public:

    virtual ~Node() = default;

    virtual std::shared_ptr<Node> getParent() const;
    virtual std::string getName() const;
    virtual std::string getValue() const;
    virtual NodeList getChildren() const;
    virtual Attributes getAttributes() const;
    virtual std::string getAttribute(const std::string&) const;

    virtual bool addChild(const std::shared_ptr<Node>&);

    virtual void destroy();

    int getLine() const;
    int getColumn() const;

protected:

    Node(const std::shared_ptr<Node>&, const std::string&, const std::string&, int, int);

    std::shared_ptr<Node> _parent;
    std::string _name;
    std::string _value;
    int _line;
    int _column;
};

class ICE_XML_API Element final : public Node
{
public:

    Element(const std::shared_ptr<Node>&, const std::string&, const Attributes&, int, int);

    NodeList getChildren() const override;
    Attributes getAttributes() const override;
    std::string getAttribute(const std::string&) const override;

    bool addChild(const std::shared_ptr<Node>&) override;

    void destroy() override;

private:

    NodeList _children;
    Attributes _attributes;
};

class ICE_XML_API Text final : public Node
{
public:

    Text(const std::shared_ptr<Node>&, const std::string&, int, int);
};

class ICE_XML_API Document : public Node
{
public:

    Document();

    NodeList getChildren() const override;

    bool addChild(const std::shared_ptr<Node>&) override;

    void destroy() override;

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

    static std::shared_ptr<Document> parse(const std::string&); // The given filename must be UTF-8 encoded
    static std::shared_ptr<Document> parse(std::istream&);

    static void parse(const std::string&, Handler&);
    static void parse(std::istream&, Handler&);
};

}

#endif
