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

namespace IceLang
{

class NextLine { };
class StartBlock { };
class EndBlock { };
class Separator { };

extern NextLine nl;
extern StartBlock sb;
extern EndBlock eb;
extern Separator sp;

// ----------------------------------------------------------------------
// Indent
// ----------------------------------------------------------------------

class Output : ::__Ice::noncopyable
{
public:

    Output();

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
    int indentSave_;
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

Output& operator<<(Output&, const NextLine&);
Output& operator<<(Output&, const StartBlock&);
Output& operator<<(Output&, const EndBlock&);
Output& operator<<(Output&, const Separator&);

}

#endif
