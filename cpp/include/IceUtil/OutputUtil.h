// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UTIL_OUTPUT_UTIL_H
#define ICE_UTIL_OUTPUT_UTIL_H

#include <IceUtil/Config.h>
#include <fstream>
#include <stack>

namespace IceUtil
{

// ----------------------------------------------------------------------
// OutputBase
// ----------------------------------------------------------------------

//
// Technically it's not necessary to have print() & nl() as virtual
// since the opeator<< functions are specific to each OutputBase
// derivative. However, since it's possible to call print() & nl()
// manually I've decided to leave them as virtual.
//

class ICE_UTIL_API OutputBase : public ::IceUtil::noncopyable
{
public:

    OutputBase();
    OutputBase(std::ostream&);
    OutputBase(const char*);

    virtual ~OutputBase() { }

    void setIndent(int); // What is the indent level?
    void setUseTab(bool); // Should we output tabs?

    void open(const char*); // Open output stream.

    virtual void print(const char*); // Print a string.

    void inc(); // Increment indentation level.
    void dec(); // Decrement indentation level.

    void useCurrentPosAsIndent(); // Save the current position as indentation.
    void zeroIndent(); // Use zero identation.
    void restoreIndent(); // Restore indentation.

    virtual void nl(); // Print newline.
    void sp(); // Print separator.

    bool operator!() const; // Check whether the output state is ok.

protected:

    std::ofstream _fout;
    std::ostream& _out;
    int _pos;
    int _indent;
    int _indentSize;
    std::stack<int> _indentSave;
    bool _useTab;
    bool _separator;
};

class ICE_UTIL_API NextLine { };
extern ICE_UTIL_API NextLine nl;

class ICE_UTIL_API Separator { };
extern ICE_UTIL_API Separator sp;

// ----------------------------------------------------------------------
// Output
// ----------------------------------------------------------------------

class ICE_UTIL_API Output : public OutputBase
{
public:

    Output();
    Output(std::ostream&);
    Output(const char*);

    void setBeginBlock(const char *); // what do we use at block starts?
    void setEndBlock(const char *);   // what do we use the block end?

    void sb(); // Start a block.
    void eb(); // End a block.

private:

    std::string _blockStart;
    std::string _blockEnd;
};

template<typename T>
Output&
operator<<(Output& out, const T& val)
{
    std::ostringstream s;
    s << val;
    out.print(s.str().c_str());
    return out;
}

template<>
inline Output&
operator<<(Output& o, const NextLine&)
{
    o.nl();
    return o;
}

template<>
inline Output&
operator<<(Output& o, const Separator&)
{
    o.sp();
    return o;
}

class ICE_UTIL_API StartBlock { };
extern ICE_UTIL_API StartBlock sb;

template<>
inline Output&
operator<<(Output& o, const StartBlock&)
{
    o.sb();
    return o;
}

class ICE_UTIL_API EndBlock { };
extern ICE_UTIL_API EndBlock eb;

template<>
inline Output&
operator<<(Output& o, const EndBlock&)
{
    o.eb();
    return o;
}

ICE_UTIL_API Output& operator<<(Output&, std::ios_base& (*)(std::ios_base&));

// ----------------------------------------------------------------------
// XMLOutput
// ----------------------------------------------------------------------

class ICE_UTIL_API XMLOutput : public OutputBase
{
public:

    XMLOutput();
    XMLOutput(std::ostream&);
    XMLOutput(const char*);

    void setSGML(bool);
    virtual void print(const char*); // Print a string.

    virtual void nl(); // Print newline.

    void se(const std::string&); // Start an element.
    void ee(); // End an element.

private:

    std::stack<std::string> _elementStack;
    bool _printed;
    bool _sgml;
};

template<typename T>
XMLOutput&
operator<<(XMLOutput& out, const T& val)
{
    std::ostringstream s;
    s << val;
    out.print(s.str().c_str());
    return out;
}

template<>
inline XMLOutput&
operator<<(XMLOutput& o, const NextLine&)
{
    o.nl();
    return o;
}

template<>
inline XMLOutput&
operator<<(XMLOutput& o, const Separator&)
{
    o.sp();
    return o;
}

class ICE_UTIL_API StartElement
{
public:

    StartElement(const std::string& name)
	: _name(name)
    {
    }
    
    ~StartElement()
    {
    }

    const std::string& getName() const { return _name; }

private:

    const std::string _name;
};
typedef StartElement se;

template<>
inline XMLOutput&
operator<<(XMLOutput& o, const StartElement& e)
{
    o.se(e.getName());
    return o;
}

class ICE_UTIL_API EndElement { };
extern ICE_UTIL_API EndElement ee;

template<>
inline XMLOutput&
operator<<(XMLOutput& o, const EndElement&)
{
    o.ee();
    return o;
}

ICE_UTIL_API XMLOutput& operator<<(XMLOutput&, std::ios_base& (*)(std::ios_base&));

}

#endif
