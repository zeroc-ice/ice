// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef OUTPUT_UTIL_H
#define OUTPUT_UTIL_H

#include <fstream>
#include <sstream>
#include <stack>

namespace Slice
{

class ICE_API NextLine { };
class ICE_API StartBlock { };
class ICE_API EndBlock { };
class ICE_API Separator { };

extern ICE_API NextLine nl;
extern ICE_API StartBlock sb;
extern ICE_API EndBlock eb;
extern ICE_API Separator sp;

// ----------------------------------------------------------------------
// Indent
// ----------------------------------------------------------------------

class ICE_API Output : ::__Ice::noncopyable
{
public:

    Output();
    Output(const char*);

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

    std::ofstream out_;
    int pos_;
    int indent_;
    std::stack<int> indentSave_;
    bool separator_;
};

template<typename T>
Output& operator<<(Output& out, const T& val)
{
    std::ostringstream s;
    s << val;
    out.print(s.str().c_str());
    return out;
}

template<typename T>
Output& operator<<(Output& out, T val)
{
    std::ostringstream s;
    s << val;
    out.print(s.str().c_str());
    return out;
}

ICE_API Output& operator<<(Output&, const NextLine&);
ICE_API Output& operator<<(Output&, const StartBlock&);
ICE_API Output& operator<<(Output&, const EndBlock&);
ICE_API Output& operator<<(Output&, const Separator&);

}

#endif
