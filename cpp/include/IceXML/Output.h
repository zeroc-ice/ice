// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_XML_OUTPUT_H
#define ICE_XML_OUTPUT_H

#include <IceUtil/Config.h>
#include <fstream>
#include <stack>

#ifdef WIN32
#   ifdef ICE_XML_API_EXPORTS
#       define ICE_XML_API __declspec(dllexport)
#   else
#       define ICE_XML_API __declspec(dllimport)
#   endif
#else
#   define ICE_XML_API /**/
#endif

namespace IceXML
{

class ICE_XML_API NextLine { };
class ICE_XML_API StartBlock { };
class ICE_XML_API EndBlock { };
class ICE_XML_API Separator { };

extern ICE_XML_API NextLine nl;
extern ICE_XML_API StartBlock sb;
extern ICE_XML_API EndBlock eb;
extern ICE_XML_API Separator sp;

// ----------------------------------------------------------------------
// Indent
// ----------------------------------------------------------------------

class ICE_XML_API Output : public ::IceUtil::noncopyable
{
public:

    Output();
    Output(std::ostream&);
    Output(const char*);

    void setBeginBlock(const char *); // what do we use at block starts?
    void setEndBlock(const char *);   // what do we use the block end?
    void setIndent(int);              // what is the indent level?
    void setUseTab(bool);             // should we output tabs?

    void open(const char*); // Open output stream

    void print(const char*); // Print a string

    void inc(); // Increment indentation level
    void dec(); // Decrement indentation level

    void useCurrentPosAsIndent(); // Save the current position as indentation
    void zeroIndent(); // Use zero identation
    void restoreIndent(); // Restore indentation

    void nl(); // Print newline
    void sb(); // Start a block
    void eb(); // End a block
    void sp(); // Print separator

    bool operator!() const; // Check whether the output state is ok

private:

    std::ofstream _fout;
    std::ostream& _out;
    int _pos;
    int _indent;
    std::stack<int> _indentSave;
    bool _separator;

    std::string _blockStart;
    std::string _blockEnd;
    bool _useTab;
    int  _indentSize;
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
operator<<(Output& o, const StartBlock&)
{
    o.sb();
    return o;
}

template<>
inline Output&
operator<<(Output& o, const EndBlock&)
{
    o.eb();
    return o;
}

template<>
inline Output&
operator<<(Output& o, const Separator&)
{
    o.sp();
    return o;
}

ICE_XML_API Output& operator<<(Output&, std::ios_base& (*)(std::ios_base&));

}

#endif
