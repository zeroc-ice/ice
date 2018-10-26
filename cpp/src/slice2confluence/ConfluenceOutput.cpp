// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/ConsoleUtil.h>
#include <ConfluenceOutput.h>
#include <iostream>
#include <sstream>
#include <algorithm>

#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wshadow"
#   pragma clang diagnostic ignored "-Wshadow-field"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wshadow"
#   pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

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
Confluence::ConfluenceOutput::print(const string& s)
{
    if(_se)
    {
        _se = false;
    }
    _text = true;

    if(_escape)
    {
        string escaped = escape(s);
        OutputBase::print(escaped);
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
    string escapeChars = "\\{}-_+*|[]";

    // For each escape character
    for(string::iterator i = escapeChars.begin(); i < escapeChars.end(); ++i)
    {
        string c(1, *i);
        string replacement;

        if(c == "\\")
        {
            replacement = "\\\\";
        }
        else if(c == "{")
        {
            replacement = "\\{";
        }
        else if(c == "}")
        {
            replacement = "\\}";
        }
        else if(c == "-")
        {
            replacement = "\\-";
        }
        else if(c == "*")
        {
            replacement = "\\*";
        }
        else if(c == "|")
        {
            replacement = "\\|";
        }
        else if(c == "_")
        {
            replacement = "\\_";
        }
        else if(c == "+")
        {
            replacement = "\\+";
        }
        else if(c == "[")
        {
            replacement = "\\[";
        }
        else if(c == "]")
        {
            replacement = "\\]";
        }

        size_t pos = comment.find(c);

        // For each position of a found escape character
        while(pos != string::npos)
        {
            pair<unsigned int,unsigned int> *region = ICE_NULLPTR;

            // Is this pos in an escaped section?
            for(list<pair<unsigned int,unsigned int> >::iterator i = escaperLimits.begin(); i != escaperLimits.end();
                ++i)
            {
                if(pos >= i->first && pos <= i->second)
                {
                    region = &*i;
                    break;
                }
            }

            if(region == ICE_NULLPTR)
            {
                comment.replace(pos, c.size(), replacement);
                pos = comment.find(c, pos + replacement.size());
            }
            else
            {
                // Skip ahead past the marked section
                pos = comment.find(c, region->second + 1);
            }
        }
    }
    comment = removeMarkers(comment);
    return comment;
}

string
Confluence::ConfluenceOutput::convertCommentHTML(string comment)
{
    comment = escapeComment(comment);

    bool italics = false;

    size_t tagStart = comment.find("<");
    while(tagStart != string::npos)
    {
        size_t tagEnd = comment.find(">", tagStart);
        string tag = comment.substr(tagStart + 1, tagEnd - (tagStart + 1));
        string replacement = "";
        bool isEndTag = tag[0] == '/';
        if(isEndTag)
        {
            // Strip slash character
            tag.erase(remove(tag.begin(), tag.end(), '/'), tag.end());
        }

        size_t spacepos = tag.find(" ");
        list<pair<string,string> > attributes;

        string rest;
        if(spacepos != string::npos)
        {
            // Get the rest and separate into attrs
            rest = tag.substr(spacepos);

            // Get just the tag
            tag = tag.substr(0, spacepos);

            size_t nextSpace = 0;
            size_t lastSpace = 0;
            do
            {
                lastSpace = nextSpace;
                nextSpace = rest.find(" ", lastSpace + 1); // Rest starts with a space

                string setting;
                if(nextSpace == string::npos)
                {
                    setting = rest.substr(lastSpace);
                }
                else {
                    setting = rest.substr(lastSpace, nextSpace - lastSpace);
                }

                size_t eqPos = setting.find("=");
                if(eqPos != string::npos)
                {
                    string aName = setting.substr(1, eqPos - 1);
                    string aVal = setting.substr(eqPos + 1);
                    // Remove quotes from val
                    size_t qPos = aVal.find("\"");
                    while(qPos != string::npos)
                    {
                        aVal.erase(qPos, 1);
                        qPos = aVal.find("\"");
                    }

                    pair<string,string> p = make_pair(aName, aVal);
                    attributes.push_back(p);
                }
                else
                {
                    // Bad attribute, ignore
                }
            } while(nextSpace != string::npos);
        }

        if(tag == "tt")
        {
            if(!isEndTag)
            {
                replacement = "{{";
            }
            else
            {
                replacement = "}}";
            }
        }
        else if(tag == "p")
        {
            // Special case: Some classes add markup
            for(list<pair<string,string> >::iterator i = attributes.begin(); i != attributes.end(); ++i)
            {
                if(i->first == "class" && i->second == "Note")
                {
                    italics = true;
                    break;
                }
                if(i->first == "class" && i->second == "Deprecated")
                {
                    italics = true;
                    break;
                }
            }

            if(!isEndTag)
            {
                if(italics)
                {
                    replacement = "\n\n_";
                }
                else
                {
                    replacement = "\n\n";
                }
            }
            else
            {
                if(italics)
                {
                    replacement = "_\n\n";
                    italics = false;
                }
                else
                {
                    replacement = "\n\n";
                }
            }
        }
        else if(tag == "ol")
        {
            if(!isEndTag)
            {
                if(_commentListMarkers.empty())
                {
                    replacement = "\n";
                }
                _commentListMarkers.append("#");

            }
            else
            {
                _commentListMarkers.erase(_commentListMarkers.size() - 1);
            }
        }
        else if(tag == "ul")
        {
            if(!isEndTag)
            {
                if(_commentListMarkers.empty())
                {
                    replacement = "\n";
                }
                _commentListMarkers.append("*");
            }
            else
            {
                _commentListMarkers.erase(_commentListMarkers.size() - 1);
            }
        }
        else if(tag == "li")
        {
            if(!isEndTag)
            {
                ostringstream oss;
                oss << "\n" << _commentListMarkers << " ";
                replacement = oss.str();
            }
            // Do nothing for end tag
        }
        else if(tag == "dl")
        {
            if(!isEndTag)
            {
                replacement = "\n";
            }
            else
            {
                replacement = "\n";
            }
        }
        else if(tag == "dt")
        {
            if(!isEndTag)
            {
                replacement = "";
            }
            else
            {
                replacement = " ";
            }
        }
        else if(tag == "dd")
        {
            if(!isEndTag)
            {
                replacement = "--- ";
            }
            else
            {
                replacement = "\n";
            }
        }
        else if(tag == "em")
        {
            if(!isEndTag)
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

        // Apply replacement
        if(tag == "p")
        {
            comment.erase(tagStart, tagEnd + 1 - tagStart);
            size_t displace = comment.find_first_not_of(" \n\r\t", tagStart); // Skip ahead over whitespace
            comment.insert(displace, replacement);
        }
        else
        {
            comment.replace(tagStart, tagEnd + 1 - tagStart, replacement); // Don't skip whitespace
        }

        // Special case: terminate <p> (and any italics) on double newline or end of comment
        size_t dnl = comment.find("\n\n", tagStart + replacement.size());
        tagStart = comment.find("<");

        if(italics)
        {
            if(tagStart == string::npos && dnl == string::npos)
            {
                // End italics before javadoc markup
                size_t atPos = comment.find("@", tagStart + replacement.size());
                if(atPos != string::npos)
                {
                    // Found markup. now move to the last non-whitespace char before the markup and end italics
                    string before = comment.substr(0, atPos);
                    size_t endLocation = before.find_last_not_of(" \n\r\t");
                    comment.insert(endLocation, "_");
                    italics = false;
                }
                else
                {
                    // No markup; end of comment
                    size_t endLocation = comment.find_last_not_of(" \n\r\t");
                    comment.insert(endLocation, "_");
                    italics = false;
                }
            }
            else if(dnl != string::npos && (tagStart == string::npos || dnl < tagStart))
            {
                string before = comment.substr(0, dnl);
                size_t endLocation = before.find_last_not_of(" \n\r\t");
                comment.insert(endLocation, "_");
                italics = false;
            }
        }

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
    if(_escape)
    {
        escaped = escape(element);
    }
    else
    {
        escaped = element;
    }

    string::size_type tagpos = element.find_first_of(" ");
    const string tagname = element.substr(0, tagpos).c_str();

    if(tagname == "p")
    {
        _out << "\n";
    }
    else if(tagname == "b")
    {
        _out << "*";
    }
    else if(tagname == "panel")
    {
        _out << "{panel}";
    }
    else if(tagname == "blockquote")
    {
        _out << "{section}{column:width=10px}{column} {column}";
    }
    else if(tagname == "dl")
    {
        _out << "\n";
    }
    else if(tagname == "dt")
    {
        _out << "";
    }
    else if(tagname == "dd")
    {
        _out << "--- ";
    }
    else if(tagname == "table")
    {
        _out << "{table}\n";
    }
    else if(tagname == "tr")
    {
        _out << "{tr}\n";
    }
    else if(tagname == "td")
    {
        _out << "{td}";
    }
    else if(tagname == "th")
    {
        _out << "{th}";
    }
    else if(tagname == "div")
    {
        _out << "{div}";
    }
    else if(tagname == "span")
    {
        _out << "{span}";
    }
    else if(tagname == "ol")
    {
        if(_listMarkers.empty())
        {
            _out << "\n";
        }
        _listMarkers.append("#");
    }
    else if(tagname == "ul")
    {
        if(_listMarkers.empty())
        {
            _out << "\n";
        }
        _listMarkers.append("*");
    }
    else if(tagname == "li")
    {
        _out << "\n" << _listMarkers << " ";
    }
    else if(tagname == "hr")
    {
        _out << "----";
    }
    else if(tagname == "h1")
    {
        _out << "\nh1. ";
    }
    else if(tagname == "h2")
    {
        _out << "\nh2. ";
    }
    else if(tagname == "h3")
    {
        _out << "\nh3. ";
    }
    else if(tagname == "h4")
    {
        _out << "\nh4. ";
    }
    else if(tagname == "h5")
    {
        _out << "\nh5. ";
    }
    else if(tagname == "h6")
    {
        _out << "\nh6. ";
    }
    else if(tagname == "tt")
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
    if(_escape)
    {
        escaped = escape(element);
    }
    else
    {
        escaped = element;
    }

    string::size_type tagpos = element.find_first_of(" ");
    const string tagname = element.substr(0, tagpos).c_str();

    if(tagname == "p")
    {
        _out << "\n";
    }
    else if(tagname == "b")
    {
        _out << "*";
    }
    else if(tagname == "panel")
    {
        _out << "{panel}\n";
    }
    else if(tagname == "blockquote")
    {
        _out << "{column}{section}\n";
    }
    else if(tagname == "dl")
    {
        _out << "\n";
    }
    else if(tagname == "dt")
    {
        _out << " ";
    }
    else if(tagname == "dd")
    {
        _out << "\n";
    }
    else if(tagname == "table")
    {
        _out << "{table}\n";
    }
    else if(tagname == "tr")
    {
        _out << "{tr}\n";
    }
    else if(tagname == "td")
    {
        _out << "{td}\n";
    }
    else if(tagname == "th")
    {
        _out << "";
    }
    else if(tagname == "div")
    {
        _out << "{div}";
    }
    else if(tagname == "span")
    {
        _out << "{span}";
    }
    else if(tagname == "ol")
    {
        _listMarkers.erase(_listMarkers.size() - 1);
        if(_listMarkers.empty())
        {
            _out << "\n";
        }
    }
    else if(tagname == "ul")
    {
        _listMarkers.erase(_listMarkers.size() - 1);
        if(_listMarkers.empty())
        {
            _out << "\n";
        }
    }
    else if(tagname == "li")
    {
        // Nothing to do
    }
    else if(tagname == "hr")
    {
        _out << "\n\n";
    }
    else if(tagname == "h1")
    {
        _out << "\n\n";
    }
    else if(tagname == "h2")
    {
        _out << "\n\n";
    }
    else if(tagname == "h3")
    {
        _out << "\n\n";
    }
    else if(tagname == "h4")
    {
        _out << "\n\n";
    }
    else if(tagname == "h5")
    {
        _out << "\n\n";
    }
    else if(tagname == "h6")
    {
        _out << "\n\n";
    }
    else if(tagname == "tt")
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
Confluence::ConfluenceOutput::getLinkMarkup(const std::string& url, const std::string& text, const std::string& anchor,
                                            const std::string& tip)
{
    ostringstream oss;
    oss << "[";
    if(!text.empty()) {
        oss << text << "|";
    }
    oss << url;
    if(!anchor.empty())
    {
        oss << "#" << anchor;
    }
    if(!tip.empty())
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
    if(!title.empty())
    {
        oss << "|" << title;
    }
    oss << "!";
    return oss.str();
}

string
Confluence::ConfluenceOutput::getAnchorMarkup(const std::string& anchor, const std::string& text)
{
    ostringstream oss;
    oss << "{anchor:" << anchor << "}";
    if(!text.empty())
    {
        oss << text << "\n";
    }
    return oss.str();
}

string
Confluence::ConfluenceOutput::getNavMarkup(const std::string& prevLink, const std::string& nextLink)
{
    ostringstream oss;
    oss << "{znav:";
    if(!prevLink.empty())
    {
        oss << "prev=" << prevLink << "|";
    }
    if(!nextLink.empty())
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
    while(start != string::npos)
    {
        end = str.find(TEMP_ESCAPER_END, start + TEMP_ESCAPER_START.size());
        if(end != string::npos)
        {
            pair<unsigned int, unsigned int> p =
                make_pair(static_cast<unsigned int>(start), static_cast<unsigned int>(end+TEMP_ESCAPER_END.size()));
            pairs.push_back(p);
            start = str.find(TEMP_ESCAPER_START, end+TEMP_ESCAPER_END.size());
        }
        else
        {
            consoleErr << "getMarkerLimits FOUND START OF ESCAPE MARKER WITH NO MATCHING END IN STRING:"
                       << endl << str.substr(start) << endl;
            break;
        }
    }

    return pairs;
}

string
Confluence::ConfluenceOutput::removeMarkers(string str)
{
    // Remove starts
    size_t start = str.find(TEMP_ESCAPER_START);
    while(start != string::npos)
    {
        str.erase(start, TEMP_ESCAPER_START.size());
        start = str.find(TEMP_ESCAPER_START, start);
    }

    // Remove ends
    size_t end = str.find(TEMP_ESCAPER_END);
    while(end != string::npos)
    {
        str.erase(end, TEMP_ESCAPER_END.size());
        end = str.find(TEMP_ESCAPER_END, end);
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
            v.insert(pos + 1, "amp;");
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
