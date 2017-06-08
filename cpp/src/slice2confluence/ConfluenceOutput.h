// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CONFLUENCE_OUTPUT
#define CONFLUENCE_OUTPUT

#include <IceUtil/OutputUtil.h>
#include <list>

namespace Confluence
{

// ----------------------------------------------------------------------
// ConfluenceOutput
// ----------------------------------------------------------------------

class ConfluenceOutput : public IceUtilInternal::OutputBase
{
public:

    ConfluenceOutput();
    ConfluenceOutput(std::ostream&);
    ConfluenceOutput(const char*);

    virtual ~ConfluenceOutput(){};

    virtual void print(const std::string&); // Print a string.

    virtual void newline(); // Print newline.

    void startElement(const std::string&); // Start an element.
    void endElement(); // End an element.
    void attr(const std::string&, const std::string&); // Add an attribute to an element.

    std::string convertCommentHTML(std::string comment);
    std::string escapeComment(std::string comment);

    std::string getAnchorMarkup(const std::string&, const std::string& = "");
    std::string getLinkMarkup(const std::string&, const std::string& = "", const std::string& = "", const std::string& = "");
    std::string getImageMarkup(const std::string&, const std::string& = "");
    std::string getNavMarkup(const std::string&, const std::string&);

    void startEscapes();
    void endEscapes();

    std::string currentElement() const;

    /**
     * Wrap sections in these markers to prevent them from being confluence-escaped.
     * The regular confluence-escaping process will remove these markers.
     */
    const static std::string TEMP_ESCAPER_START; // wrap sections
    const static std::string TEMP_ESCAPER_END; // wrap sections

    /**
     * Gets the start and end positions of all TEMP_ESCAPED sections of the given string.
     */
    std::list<std::pair<unsigned int,unsigned int> > getMarkerLimits(const std::string&);

    std::string removeMarkers(std::string);

private:

    std::string escape(const ::std::string&) const;

    std::stack<std::string> _elementStack;

    bool _se;
    bool _text;

    bool _escape;

    std::string _listMarkers;
    std::string _commentListMarkers;
};

template<typename T>
inline ConfluenceOutput&
operator<<(ConfluenceOutput& out, const T& val)
{
    std::ostringstream s;
    s << val;
    out.print(s.str().c_str());
    return out;
}

template<>
inline ConfluenceOutput&
operator<<(ConfluenceOutput& o, const IceUtilInternal::NextLine&)
{
    o.newline();
    return o;
}

template<>
inline ConfluenceOutput&
operator<<(ConfluenceOutput& o, const IceUtilInternal::Separator&)
{
    o.separator();
    return o;
}

class EndElement
{
};
extern EndElement ee;

template<>
inline ConfluenceOutput&
operator<<(ConfluenceOutput& o, const EndElement&)
{
    o.endElement();
    return o;
}

class StartElement
{
public:

    StartElement(const std::string&);

    const std::string& getName() const;

private:

    const std::string _name;
};

typedef StartElement se;

template<>
inline ConfluenceOutput&
operator<<(ConfluenceOutput& o, const StartElement& e)
{
    o.startElement(e.getName());
    return o;
}

class Attribute
{
public:

    Attribute(const ::std::string&, const ::std::string&);

    const ::std::string& getName() const;
    const ::std::string& getValue() const;

private:

    const ::std::string _name;
    const ::std::string _value;
};

typedef Attribute attr;

template<>
inline ConfluenceOutput&
operator<<(ConfluenceOutput& o, const Attribute& e)
{
    o.attr(e.getName(), e.getValue());
    return o;
}

class StartEscapes
{
};
extern StartEscapes startEscapes;

class EndEscapes
{
};
extern EndEscapes endEscapes;

template<>
inline ConfluenceOutput&
operator<<(ConfluenceOutput& o, const StartEscapes&)
{
    o.startEscapes();
    return o;
}

template<>
inline ConfluenceOutput&
operator<<(ConfluenceOutput& o, const EndEscapes&)
{
    o.endEscapes();
    return o;
}

ConfluenceOutput& operator<<(ConfluenceOutput&, std::ios_base& (*)(std::ios_base&));

}

#endif
