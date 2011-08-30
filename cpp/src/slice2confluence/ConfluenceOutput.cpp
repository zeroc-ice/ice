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
    _listMarkers(""),
    _commentListMarkers("")
{
}

Confluence::ConfluenceOutput::ConfluenceOutput(ostream& os) :
    OutputBase(os),
    _se(false),
    _text(false),
    _escape(false),
    _listMarkers(""),
    _commentListMarkers("")
{
}

Confluence::ConfluenceOutput::ConfluenceOutput(const char* s) :
    OutputBase(s),
    _se(false),
    _text(false),
    _escape(false),
    _listMarkers(""),
    _commentListMarkers("")
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

string 
Confluence::ConfluenceOutput::escapeComment(string comment)
{
    string escapeChars = "\\{}-*|[]"; //backslash ("\\") needs to be first because it gets added later
    for (string::iterator i = escapeChars.begin(); i < escapeChars.end(); ++i)
    {
        string c(1, *i);
        string replacement = "\\" + c;
        
        size_t pos = comment.find(c);
        while (pos != string::npos)
        {
            cout << "COMMENT: " << comment << endl;
            comment.replace(pos, c.size(), replacement);
            cout << "COMMENT AFTER: " << comment << endl;
            
            pos = comment.find(c, pos+replacement.size());
        }
    }
    return comment;
}

string 
Confluence::ConfluenceOutput::convertCommentHTML(string comment)
{
    escapeComment(comment);
    
    size_t tagStart = comment.find("<");
    while (tagStart != string::npos)
    {
        size_t tagEnd = comment.find(">", tagStart);
        string tag = comment.substr(tagStart + 1, tagEnd - (tagStart + 1));
        string replacement = "";
        bool isEndTag = tag[0] == '/';
        if (isEndTag) 
        {
            //strip slash character
            tag.erase(remove(tag.begin(), tag.end(), '/'), tag.end());
        }
        
        size_t spacepos = tag.find(" ");
        if (spacepos != string::npos)
        {
            //strip attributes from tag
            tag = tag.substr(0, spacepos);
        }
        
        if (!strcmp(tag.c_str(), "tt"))
        {
            if (!isEndTag)
            {
                replacement = "{{{";
            }
            else
            {
                replacement = "}}}";
            }
        }
        else if (!strcmp(tag.c_str(), "p"))
        {
            if (!isEndTag)
            {
                replacement = "\n\n";
            }
            else
            {
                replacement = "\n\n";
            }
        }
        else if (!strcmp(tag.c_str(), "ol"))
        {
            if (!isEndTag)
            {
                if (_commentListMarkers.empty())
                {
                    replacement = "\n";
                }
                _commentListMarkers.append("#");

            }
            else
            {
                _commentListMarkers.erase(_commentListMarkers.size()-1);
            }
        }
        else if (!strcmp(tag.c_str(), "ul"))
        {
            if (!isEndTag)
            {
                if (_commentListMarkers.empty())
                {
                    replacement = "\n";
                }
                _commentListMarkers.append("*");
            }
            else
            {
                _commentListMarkers.erase(_commentListMarkers.size()-1);
            }
        }
        else if (!strcmp(tag.c_str(), "li"))
        {
            if (!isEndTag)
            {
                ostringstream oss;
                oss << "\n" << _commentListMarkers << " ";
                replacement = oss.str();
            }
            //do nothing for end tag
        }
        else 
        {
            if (!isEndTag)
            {
                replacement = "{{";
            }
            else
            {
                replacement = "}}";
            }
        }
        
        //apply replacement
        comment.replace(tagStart, tagEnd + 1 - tagStart, replacement);
        
        tagStart = comment.find("<");
    }
    return comment;
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
    } 
    else if (!strcmp(tagname, "b")) 
    {
        _out << "*";
    } 
    else if (!strcmp(tagname, "panel")) 
    {
        _out << "{panel}";
    } 
    else if (!strcmp(tagname, "blockquote")) 
    {
        _out << "{section}{column:width=10px}{column} {column}";
    } 
    else if (!strcmp(tagname, "dl")) 
    {
        _out << "\n";
    } 
    else if (!strcmp(tagname, "dt")) 
    {
        _out << "";
    } 
    else if (!strcmp(tagname, "dd")) 
    {
        _out << "--- ";
    } 
    else if (!strcmp(tagname, "table")) 
    {
        _out << "{table}\n";
    } 
    else if (!strcmp(tagname, "tr")) 
    {
        _out << "{tr}\n";
    } 
    else if (!strcmp(tagname, "td")) 
    {
        _out << "{td}";
    } 
    else if (!strcmp(tagname, "th")) 
    {
        _out << "{th}";
    } 
    else if (!strcmp(tagname, "div")) 
    {
        _out << "{div}";
    } 
    else if (!strcmp(tagname, "span")) 
    {
        _out << "{span}";
    } 
    else if (!strcmp(tagname, "ol")) 
    {
        if (_listMarkers.empty())
        {
            _out << "\n";
        }
        _listMarkers.append("#");
    } 
    else if (!strcmp(tagname, "ul")) 
    {
        if (_listMarkers.empty())
        {
            _out << "\n";
        }
        _listMarkers.append("*");
    } 
    else if (!strcmp(tagname, "li")) 
    {
        _out << "\n" << _listMarkers << " ";
    } 
    else if (!strcmp(tagname, "hr")) 
    {
        _out << "----";
    } 
    else if (!strcmp(tagname, "h1")) 
    {
        _out << "\nh1. ";
    } 
    else if (!strcmp(tagname, "h2")) 
    {
        _out << "\nh2. ";
    } 
    else if (!strcmp(tagname, "h3")) 
    {
        _out << "\nh3. ";
    } 
    else if (!strcmp(tagname, "h4")) 
    {
        _out << "\nh4. ";
    } 
    else if (!strcmp(tagname, "h5")) 
    {
        _out << "\nh5. ";
    } 
    else if (!strcmp(tagname, "h6")) 
    {
        _out << "\nh6. ";
    } 
    else if (!strcmp(tagname, "tt")) 
    {
        _out << "{{";
    } 
    else 
    {
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
    }
    else if (!strcmp(tagname, "b")) 
    {
        _out << "*";
    } 
    else if (!strcmp(tagname, "panel")) 
    {
        _out << "{panel}\n";
    } 
    else if (!strcmp(tagname, "blockquote")) 
    {
        _out << "{column}{section}\n";
    } 
    else if (!strcmp(tagname, "dl")) 
    {
        _out << "\n";
    } 
    else if (!strcmp(tagname, "dt")) 
    {
        _out << " ";
    }
    else if (!strcmp(tagname, "dd")) 
    {
        _out << "\n";
    }
    else if (!strcmp(tagname, "table")) 
    {
        _out << "{table}\n";
    }
    else if (!strcmp(tagname, "tr")) 
    {
        _out << "{tr}\n";
    }
    else if (!strcmp(tagname, "td")) 
    {
        _out << "{td}\n";
    }
    else if (!strcmp(tagname, "th")) 
    {
        _out << "";
    }
    else if (!strcmp(tagname, "div")) 
    {
        _out << "{div}";
    }
    else if (!strcmp(tagname, "span")) 
    {
        _out << "{span}";
    }
    else if (!strcmp(tagname, "ol")) 
    {
        _listMarkers.erase(_listMarkers.size()-1);
        if (_listMarkers.empty())
        {
            _out << "\n";
        }
    }
    else if (!strcmp(tagname, "ul")) 
    {
        _listMarkers.erase(_listMarkers.size()-1);
        if (_listMarkers.empty())
        {
            _out << "\n";
        }
    }
    else if (!strcmp(tagname, "li")) 
    {
        //nothing to do
    }
    else if (!strcmp(tagname, "hr")) 
    {
        _out << "\n\n";
    }
    else if (!strcmp(tagname, "h1")) 
    {
        _out << "\n\n";
    }
    else if (!strcmp(tagname, "h2")) 
    {
        _out << "\n\n";
    }
    else if (!strcmp(tagname, "h3")) 
    {
        _out << "\n\n";
    }
    else if (!strcmp(tagname, "h4")) 
    {
        _out << "\n\n";
    }
    else if (!strcmp(tagname, "h5")) 
    {
        _out << "\n\n";
    }
    else if (!strcmp(tagname, "h6")) 
    {
        _out << "\n\n";
    }
    else if (!strcmp(tagname, "tt")) 
    {
        _out << "}}";
    }
    else
    {
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

string
Confluence::ConfluenceOutput::getAnchorMarkup(const std::string& anchor, const std::string& text)
{
    ostringstream oss;
    oss << "{anchor:" << anchor << "}";
    if (!text.empty())
    {
        oss << text << "\n";
    }
    return oss.str(); //leak?
}

string
Confluence::ConfluenceOutput::getNavMarkup(const std::string& prevLink, const std::string& nextLink)
{
    ostringstream oss;
    oss << "{znav:";
    if (!prevLink.empty())
    {
        oss << "prev=" << prevLink << "|";
    }
    if (!nextLink.empty())
    {
        oss << "next=" << nextLink;
    }
    oss << "}\n";
    oss << "{section}{section}\n";
    return oss.str();
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
