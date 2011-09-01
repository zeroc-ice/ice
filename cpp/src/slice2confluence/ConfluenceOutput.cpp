#include <ConfluenceOutput.h>
#include <iostream>
#include <sstream>
#include <utility>
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

const string Confluence::ConfluenceOutput::TEMP_ESCAPER_START = "$$$$$$$";
const string Confluence::ConfluenceOutput::TEMP_ESCAPER_END = "!!!!!!!";

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
    list< pair<unsigned int,unsigned int> > escaperLimits = getMarkerLimits(comment);
    string escapeChars = "\\{}-*|[]";
    
    //for each escape character
    for (string::iterator i = escapeChars.begin(); i < escapeChars.end(); ++i)
    {
        string c(1, *i);
        string replacement;
        
        if (c == "\\")
        {
            replacement = "&#92;";
        }
        else if (c == "{")
        {
            replacement = "&#123;";
        }
        else if (c == "}")
        {
            replacement = "&#125;";
        }
        else if (c == "-")
        {
            replacement = "&#45;";
        }
        else if (c == "*")
        {
            replacement = "&#42;";
        }
        else if (c == "|")
        {
            replacement = "&#124;";
        }
        else if (c == "[")
        {
            replacement = "&#91;";
        }
        else if (c == "]")
        {
            replacement = "&#93;";
        }
        
        size_t pos = comment.find(c);
        
        //for each position of a found escape character
        while (pos != string::npos)
        {
            pair<unsigned int,unsigned int> *region = NULL;
            
            //is this pos in an escaped section?
            for (list<pair<unsigned int,unsigned int> >::iterator i = escaperLimits.begin(); i != escaperLimits.end(); ++i)
            {
                if (pos >= i->first && pos <= i->second)
                {
                    region = &*i;
                    break;
                }
            }
            
            if (region == NULL)
            {
                comment.replace(pos, c.size(), replacement);
                pos = comment.find(c, pos+replacement.size());
            }
            else
            {
//                cout << "skipping ahead to: '" << comment.substr(region->second) << "'"<< endl;
                pos = comment.find(c, region->second+1);
            }
        }
    }
    
//    cout << "COMMENT: " << comment << endl;
    comment = removeMarkers(comment);
    size_t f = comment.find("This exception is raised if the ");
    if (f != string::npos)
    {
        cout << "AFTER: " << comment << endl;
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
                replacement = "{{";
            }
            else
            {
                replacement = "}}";
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
        else if (!strcmp(tag.c_str(), "dl"))
        {
            if (!isEndTag)
            {
                replacement = "\n";
            }
            else
            {
                replacement = "\n";
            }
        }
        else if (!strcmp(tag.c_str(), "dt"))
        {
            if (!isEndTag)
            {
                replacement = "";
            }
            else
            {
                replacement = " ";
            }
        }
        else if (!strcmp(tag.c_str(), "dd"))
        {
            if (!isEndTag)
            {
                replacement = "--- ";
            }
            else
            {
                replacement = "\n";
            }
        }
        else if (!strcmp(tag.c_str(), "em"))
        {
            if (!isEndTag)
            {
                replacement = "_";
            }
            else
            {
                replacement = "_";
            }
        }
        else 
        {
            replacement = "*{{UNRECOGNIZED MARKUP: " + tag + "}}*";
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

list< pair<unsigned int,unsigned int> > 
Confluence::ConfluenceOutput::getMarkerLimits(const string& str)
{
    list< pair<unsigned int,unsigned int> > pairs;
    
    size_t start = str.find(TEMP_ESCAPER_START); 
    size_t end;
    while (start != string::npos)
    {
        end = str.find(TEMP_ESCAPER_END, start+TEMP_ESCAPER_START.size());
        if (end != string::npos)
        {
            pair<unsigned int, unsigned int> p = make_pair((unsigned int)start, (unsigned int)end+TEMP_ESCAPER_END.size());
//            cout << "adding pair (" << p.first << ", " << p.second << ") for '" << str << "'"  << endl; 
            pairs.push_back(p);
            start = str.find(TEMP_ESCAPER_START, end+TEMP_ESCAPER_END.size());
        }
        else
        {
            cerr << "getEscaperLimits FOUND START OF ESCAPER WITH NO MATCHING END IN STRING:" << endl << str.substr(start) << endl;
            break;
        }
    }
    
    return pairs;
}

string 
Confluence::ConfluenceOutput::removeMarkers(string str)
{
    //remove starts
//    cout << "REMOVE STARTS FROM STR: " << str << endl;
    size_t start = str.find(TEMP_ESCAPER_START);
    while (start != string::npos)
    {
        str.erase(start, TEMP_ESCAPER_START.size());
        start = str.find(TEMP_ESCAPER_START, start);
    }
    
//    cout << "WITH STARTS REMOVED: " << str << endl;
    
    //remove ends
    size_t end = str.find(TEMP_ESCAPER_END);
    while (end != string::npos)
    {
        str.erase(end, TEMP_ESCAPER_END.size());
        end = str.find(TEMP_ESCAPER_END, end);
    }
    size_t f = str.find("This exception is raised if the ");
    if (f != string::npos)
    {
        cout << "WITH STOPS REMOVED?: " << str << endl;
    }
    return str;
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
