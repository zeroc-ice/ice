// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef SLICE_OUTPUT_UTIL_H
#define SLICE_OUTPUT_UTIL_H

#include <IceUtil/Config.h>
#include <fstream>
#include <stack>

#ifdef WIN32
#   ifdef SLICE_API_EXPORTS
#       define SLICE_API __declspec(dllexport)
#   else
#       define SLICE_API __declspec(dllimport)
#   endif
#else
#   define SLICE_API /**/
#endif

namespace Slice
{

class SLICE_API NextLine { };
class SLICE_API StartBlock { };
class SLICE_API EndBlock { };
class SLICE_API Separator { };

extern SLICE_API NextLine nl;
extern SLICE_API StartBlock sb;
extern SLICE_API EndBlock eb;
extern SLICE_API Separator sp;

// ----------------------------------------------------------------------
// Indent
// ----------------------------------------------------------------------

class SLICE_API Output : public ::IceUtil::noncopyable
{
public:

    Output();
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

    std::ofstream _out;
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

SLICE_API Output& operator<<(Output&, std::ios_base& (*)(std::ios_base&));

}

#endif
