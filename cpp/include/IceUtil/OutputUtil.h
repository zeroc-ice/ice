//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_OUTPUT_UTIL_H
#define ICE_UTIL_OUTPUT_UTIL_H

#include <IceUtil/Config.h>
#include <fstream>
#include <stack>
#include <vector>

namespace IceUtilInternal
{

ICE_API std::string int64ToString(IceUtil::Int64);

// ----------------------------------------------------------------------
// OutputBase
// ----------------------------------------------------------------------

//
// Technically it's not necessary to have print() & newline() as virtual
// since the opeator<< functions are specific to each OutputBase
// derivative. However, since it's possible to call print() & newline()
// manually I've decided to leave them as virtual.
//

class ICE_API OutputBase : private ::IceUtil::noncopyable
{
public:

    OutputBase();
    OutputBase(std::ostream&);
    OutputBase(const std::string&);
    virtual ~OutputBase();

    void setIndent(int); // What is the indent level?
    void setUseTab(bool); // Should we output tabs?

    void open(const std::string&); // Open output stream.
    void close(); // Close output stream.
    bool isOpen(); // Is a file stream open?

    virtual void print(const std::string&); // Print a string.

    void inc(); // Increment indentation level.
    void dec(); // Decrement indentation level.

    void useCurrentPosAsIndent(); // Save the current position as indentation.
    void zeroIndent(); // Use zero identation.
    void restoreIndent(); // Restore indentation.
    int currIndent(); // Return current indent value.

    virtual void newline(); // Print newline.
    void separator(); // Print separator.

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

class ICE_API NextLine
{
};
extern ICE_API NextLine nl;

class ICE_API Separator
{
};
extern ICE_API Separator sp;

// ----------------------------------------------------------------------
// Output
// ----------------------------------------------------------------------

class ICE_API Output : public OutputBase
{
public:

    Output(bool breakBeforeBlock = true, bool shortEmptyBlock = false);
    Output(std::ostream&, bool = true, bool = false);
    Output(const char*, bool = true, bool = false);

    virtual void print(const std::string&); // Print a string.

    void sb(); // Start a block.
    void eb(); // End a block.

    void spar(char = '('); // Start a paramater list.
    void epar(char = ')'); // End a paramater list.

private:

    std::string _blockStart;
    std::string _blockEnd;
    int _par; // If >= 0, we are writing a parameter list.
    const bool _breakBeforeBlock; // If true break before starting a new block.
    const bool _shortEmptyBlock; // If true, an empty block is written <sb><eb>.
    bool _emptyBlock;
};

template<typename T>
inline Output&
operator<<(Output& out, const T& val)
{
    std::ostringstream s;
    s << val;
    out.print(s.str());
    return out;
}

template<typename T>
inline Output&
operator<<(Output& out, const std::vector<T>& val)
{
    for(typename std::vector<T>::const_iterator p = val.begin(); p != val.end(); ++p)
    {
        out << *p;
    }
    return out;
}

template<>
inline Output&
operator<<(Output& o, const NextLine&)
{
    o.newline();
    return o;
}

template<>
inline Output&
operator<<(Output& o, const Separator&)
{
    o.separator();
    return o;
}

class ICE_API StartBlock
{
};
extern ICE_API StartBlock sb;

template<>
inline Output&
operator<<(Output& o, const StartBlock&)
{
    o.sb();
    return o;
}

class ICE_API EndBlock
{
};
extern ICE_API EndBlock eb;

template<>
inline Output&
operator<<(Output& o, const EndBlock&)
{
    o.eb();
    return o;
}

class ICE_API StartPar
{
};
extern ICE_API StartPar spar;

template<>
inline Output&
operator<<(Output& o, const StartPar&)
{
    o.spar();
    return o;
}

class ICE_API EndPar
{
};
extern ICE_API EndPar epar;

template<>
inline Output&
operator<<(Output& o, const EndPar&)
{
    o.epar();
    return o;
}

class ICE_API StartAbrk
{
};
extern ICE_API StartAbrk sabrk;

template<>
inline Output&
operator<<(Output& o, const StartAbrk&)
{
    o.spar('<');
    return o;
}

class ICE_API EndAbrk
{
};
extern ICE_API EndAbrk eabrk;

template<>
inline Output&
operator<<(Output& o, const EndAbrk&)
{
    o.epar('>');
    return o;
}

ICE_API Output& operator<<(Output&, std::ios_base& (*)(std::ios_base&));

// ----------------------------------------------------------------------
// XMLOutput
// ----------------------------------------------------------------------

class ICE_API XMLOutput : public OutputBase
{
public:

    XMLOutput();
    XMLOutput(std::ostream&);
    XMLOutput(const char*);

    virtual void print(const std::string&); // Print a string.

    virtual void newline(); // Print newline.

    void startElement(const std::string&); // Start an element.
    void endElement(); // End an element.
    void attr(const std::string&, const std::string&); // Add an attribute to an element.

    void startEscapes();
    void endEscapes();

    std::string currentElement() const;

private:

    ::std::string escape(const ::std::string&) const;

    std::stack<std::string> _elementStack;

    bool _se;
    bool _text;

    bool _escape;
};

template<typename T>
inline XMLOutput&
operator<<(XMLOutput& out, const T& val)
{
    std::ostringstream s;
    s << val;
    out.print(s.str());
    return out;
}

template<>
inline XMLOutput&
operator<<(XMLOutput& o, const NextLine&)
{
    o.newline();
    return o;
}

template<>
inline XMLOutput&
operator<<(XMLOutput& o, const Separator&)
{
    o.separator();
    return o;
}

class ICE_API EndElement
{
};
extern ICE_API EndElement ee;

template<>
inline XMLOutput&
operator<<(XMLOutput& o, const EndElement&)
{
    o.endElement();
    return o;
}

class ICE_API StartElement
{
public:

    StartElement(const std::string&);

    const std::string& getName() const;

private:

    const std::string _name;
};

typedef StartElement se;

template<>
inline XMLOutput&
operator<<(XMLOutput& o, const StartElement& e)
{
    o.startElement(e.getName());
    return o;
}

class ICE_API Attribute
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
inline XMLOutput&
operator<<(XMLOutput& o, const Attribute& e)
{
    o.attr(e.getName(), e.getValue());
    return o;
}

class ICE_API StartEscapes
{
};
extern ICE_API StartEscapes startEscapes;

class ICE_API EndEscapes
{
};
extern ICE_API EndEscapes endEscapes;

template<>
inline XMLOutput&
operator<<(XMLOutput& o, const StartEscapes&)
{
    o.startEscapes();
    return o;
}

template<>
inline XMLOutput&
operator<<(XMLOutput& o, const EndEscapes&)
{
    o.endEscapes();
    return o;
}

ICE_API XMLOutput& operator<<(XMLOutput&, std::ios_base& (*)(std::ios_base&));

}

#endif
