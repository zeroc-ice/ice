#include <ConfluenceOutput.h>
#include <iostream>
#include <sstream>
#include <string.h>

using namespace std;
using namespace IceUtilInternal;

namespace Confluence
{

    EndElement ee;

}

// ----------------------------------------------------------------------
// ConfluenceOutput
// ----------------------------------------------------------------------

Confluence::ConfluenceOutput::ConfluenceOutput() :
    OutputBase(),
    _se(false),
    _text(false),
    _escape(false),
    _listMarkers("")
{
}

Confluence::ConfluenceOutput::ConfluenceOutput(ostream& os) :
    OutputBase(os),
    _se(false),
    _text(false),
    _escape(false),
    _listMarkers("")
{
}

Confluence::ConfluenceOutput::ConfluenceOutput(const char* s) :
    OutputBase(s),
    _se(false),
    _text(false),
    _escape(false),
    _listMarkers("")
{
}

void
Confluence::ConfluenceOutput::print(const char* s)
{
    if(_se)
    {
        _se = false;
    }
    _text = true;
    
    if(_escape)
    {
        string escaped = escape(s);
        OutputBase::print(escaped.c_str());
    }
    else
    {
        OutputBase::print(s);
    }
}

void
Confluence::ConfluenceOutput::newline()
{
    if(_se)
    {
        _se = false;
    }
    OutputBase::newline();
}

void
Confluence::ConfluenceOutput::startElement(const string& element)
{
    string escaped;
    if (_escape)
    {
        escaped = escape(element);
    }
    else
    {
        escaped = element;
    }
    
    const char *tagname;
    string::size_type tagpos = element.find_first_of(" ");
    tagname = element.substr(0, tagpos).c_str();
    
    
    if (!strcmp(tagname, "p")) 
    {
        _out << "\n";
    } else if (!strcmp(tagname, "dl")) 
    {
        _out << "\n";
    } else if (!strcmp(tagname, "dt")) 
    {
        _out << "+";
    } else if (!strcmp(tagname, "dd")) 
    {
        _out << "* ";
    } else if (!strcmp(tagname, "table")) 
    {
        _out << "\n";
    } else if (!strcmp(tagname, "tbody")) 
    {
        _out << "\n";
    } else if (!strcmp(tagname, "tr")) 
    {
        _out << "\n";
    } else if (!strcmp(tagname, "td")) 
    {
        _out << "|";
    } else if (!strcmp(tagname, "th")) 
    {
        _out << "||";
    } else if (!strcmp(tagname, "div")) 
    {
        _out << "{panel}";
    } else if (!strcmp(tagname, "span")) 
    {
        _out << "{panel}";
    } else if (!strcmp(tagname, "ol")) 
    {
        _listMarkers.append("#");
        _out << "\n";
    } else if (!strcmp(tagname, "ul")) 
    {
        _listMarkers.append("*");
        _out << "\n";
    } else if (!strcmp(tagname, "li")) 
    {
        _out << _listMarkers << " ";
    } else if (!strcmp(tagname, "hr")) 
    {
        _out << "----";
    } else if (!strcmp(tagname, "h1")) 
    {
        _out << "h1. ";
    } else if (!strcmp(tagname, "h2")) 
    {
        _out << "h2. ";
    } else if (!strcmp(tagname, "h3")) 
    {
        _out << "h3. ";
    } else if (!strcmp(tagname, "h4")) 
    {
        _out << "h4. ";
    } else if (!strcmp(tagname, "h5")) 
    {
        _out << "h5. ";
    } else if (!strcmp(tagname, "h6")) 
    {
        _out << "h6. ";
    } else {
        _out << "{" << escaped << "}";
    }
    
    _se = true;
    _text = false;
    
    string::size_type pos = element.find_first_of(" \t");
    if(pos == string::npos)
    {
        _elementStack.push(element);
    }
    else
    {
        _elementStack.push(element.substr(0, pos));
    }
    
    ++_pos; // TODO: ???
    inc();
    _separator = false;
}

void
Confluence::ConfluenceOutput::endElement()
{
    string element = _elementStack.top();
    _elementStack.pop();
    
    
    string escaped;
    if (_escape)
    {
        escaped = escape(element);
    }
    else
    {
        escaped = element;
    }
    
    const char *tagname;
    string::size_type tagpos = element.find_first_of(" ");
    tagname = element.substr(0, tagpos).c_str();
    
    if (!strcmp(tagname, "p")) 
    {
        _out << "\n";
    } else if (!strcmp(tagname, "dl")) 
    {
        _out << "\n";
    } else if (!strcmp(tagname, "dt")) 
    {
        _out << "+\n";
    } else if (!strcmp(tagname, "dd")) 
    {
        _out << "\n\n";
    } else if (!strcmp(tagname, "table")) 
    {
        _out << "\n";
    } else if (!strcmp(tagname, "tbody")) 
    {
        _out << "";
    } else if (!strcmp(tagname, "tr")) 
    {
        _out << "|\n";
    } else if (!strcmp(tagname, "td")) 
    {
        _out << "";
    } else if (!strcmp(tagname, "th")) 
    {
        _out << "";
    } else if (!strcmp(tagname, "div")) 
    {
        _out << "{panel}";
    } else if (!strcmp(tagname, "span")) 
    {
        _out << "{panel}";
    } else if (!strcmp(tagname, "ol")) 
    {
        _listMarkers.erase(_listMarkers.size()-1);
        _out << "\n";
    } else if (!strcmp(tagname, "ul")) 
    {
        _listMarkers.erase(_listMarkers.size()-1);
        _out << "\n";
    } else if (!strcmp(tagname, "li")) 
    {
        _out << "\n";
    } else if (!strcmp(tagname, "hr")) 
    {
        _out << "\n\n";
    } else if (!strcmp(tagname, "h1")) 
    {
        _out << "\n\n";
    } else if (!strcmp(tagname, "h2")) 
    {
        _out << "\n\n";
    } else if (!strcmp(tagname, "h3")) 
    {
        _out << "\n\n";
    } else if (!strcmp(tagname, "h4")) 
    {
        _out << "\n\n";
    } else if (!strcmp(tagname, "h5")) 
    {
        _out << "\n\n";
    } else if (!strcmp(tagname, "h6")) 
    {
        _out << "\n\n";
    } else {
        _out << "{" << escaped << "}";
    }
    
    dec();
    --_pos; // TODO: ???
    
    _se = false;
    _text = false;
}

string
Confluence::ConfluenceOutput::getLinkMarkup(const std::string& url, const std::string& text, const std::string& anchor, const std::string& tip)
{
    ostringstream oss;
    oss << "[";
    if (!text.empty()) {
        oss << text << "|";
    }
    oss << url;
    if (!anchor.empty())
    {
        oss << "#" << anchor;
    }
    if (!tip.empty())
    {
        oss << "|" << tip;
    }
    oss << "]";
    return oss.str();
}

string
Confluence::ConfluenceOutput::getImageMarkup(const string& url, const string& title)
{
    ostringstream oss;
    oss << "!" << url;
    if (!title.empty())
    {
        oss << "|" << title;
    }
    oss << "!";
    return oss.str(); //leak?
}

void
Confluence::ConfluenceOutput::attr(const string& name, const string& value)
{
    //
    // Precondition: Attributes can only be attached to elements.
    //
    assert(_se);
    _out << " " << name << "=\"" << escape(value) << "\"";
}

void
Confluence::ConfluenceOutput::startEscapes()
{
    _escape = true;
}

void
Confluence::ConfluenceOutput::endEscapes()
{
    _escape = false;
}

string
Confluence::ConfluenceOutput::currentElement() const
{
    if(_elementStack.size() > 0)
    {
        return _elementStack.top();
    }
    else
    {
        return string();
    }
}

string
Confluence::ConfluenceOutput::escape(const string& input) const
{
    string v = input;
    
    //
    // Find out whether there is a reserved character to avoid
    // conversion if not necessary.
    //
    const string allReserved = "<>'\"&{}";
    if(v.find_first_of(allReserved) != string::npos)
    {
        //
        // First convert all & to &amp;
        //
        size_t pos = 0;
        while((pos = v.find_first_of('&', pos)) != string::npos)
        {
            v.insert(pos+1, "amp;");
            pos += 4;
        }
        
        //
        // Next convert remaining reserved characters.
        //
        const string reserved = "<>'\"{}";
        pos = 0;
        while((pos = v.find_first_of(reserved, pos)) != string::npos)
        {
            string replace;
            switch(v[pos])
            {
                case '>':
                    replace = "&gt;";
                    break;
                    
                case '<':
                    replace = "&lt;";
                    break;
                    
                case '\'':
                    replace = "&apos;";
                    break;
                    
                case '"':
                    replace = "&quot;";
                    break;
                    
                case '{':
                    replace = "\\{";
                    break;
                    
                case '}':
                    replace = "\\}";
                    break;
                    
                default:
                    assert(false);
            }
            
            v.erase(pos, 1);
            v.insert(pos, replace);
            pos += replace.size();
        }
    }
    return v;
}

Confluence::ConfluenceOutput&
Confluence::operator<<(ConfluenceOutput& out, ios_base& (*val)(ios_base&))
{
    ostringstream s;
    s << val;
    out.print(s.str().c_str());
    return out;
}

Confluence::StartElement::StartElement(const string& name) :
_name(name)
{
}

const string&
Confluence::StartElement::getName() const
{
    return _name;
}

Confluence::Attribute::Attribute(const string& name, const string& value) :
    _name(name),
    _value(value)
{
}

const string&
Confluence::Attribute::getName() const
{
    return _name;
}

const string&
Confluence::Attribute::getValue() const
{
    return _value;
}
