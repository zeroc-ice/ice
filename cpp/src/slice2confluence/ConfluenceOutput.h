#include <IceUtil/OutputUtil.h>

#ifndef CONFLUENCE_OUTPUT
#define CONFLUENCE_OUTPUT

namespace Confluence
{

// ----------------------------------------------------------------------
// ConfluenceOutput
// ----------------------------------------------------------------------

class ICE_UTIL_API ConfluenceOutput : public IceUtilInternal::OutputBase
{
public:
    
    ConfluenceOutput();
    ConfluenceOutput(std::ostream&);
    ConfluenceOutput(const char*);
    
    virtual ~ConfluenceOutput(){};
    
    virtual void print(const char*); // Print a string.
    
    virtual void newline(); // Print newline.
    
    void startElement(const std::string&); // Start an element.
    void endElement(); // End an element.
    void attr(const std::string&, const std::string&); // Add an attribute to an element.
    
    std::string getLinkMarkup(const std::string&, const std::string& = "", const std::string& = "", const std::string& = "");
    std::string getImageMarkup(const std::string&, const std::string& = "");
    
    void startEscapes();
    void endEscapes();
    
    std::string currentElement() const;
    
private:
    
    std::string escape(const ::std::string&) const;
    
    std::stack<std::string> _elementStack;
    
    bool _se;
    bool _text;
    
    bool _escape;
    
    std::string _listMarkers;
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

class ICE_UTIL_API EndElement
{
};
extern ICE_UTIL_API EndElement ee;

template<>
inline ConfluenceOutput&
operator<<(ConfluenceOutput& o, const EndElement&)
{
    o.endElement();
    return o;
}

class ICE_UTIL_API StartElement
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

class ICE_UTIL_API Attribute
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

class ICE_UTIL_API StartEscapes
{
};
extern ICE_UTIL_API StartEscapes startEscapes;

class ICE_UTIL_API EndEscapes
{
};
extern ICE_UTIL_API EndEscapes endEscapes;

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

ICE_UTIL_API ConfluenceOutput& operator<<(ConfluenceOutput&, std::ios_base& (*)(std::ios_base&));

}

#endif
