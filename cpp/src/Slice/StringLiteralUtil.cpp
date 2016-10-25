// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/Util.h>

#include <IceUtil/InputUtil.h>
#include <IceUtil/StringConverter.h>

#include <ostream>
#include <iomanip>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

class StringLiteralGenerator
{
public:

    StringLiteralGenerator(const string&, const string&, EscapeMode, unsigned char);

    string escapeASCIIChar(char) const;
    string escapeCodePoint(unsigned int) const;
    string flushU8Buffer(vector<unsigned char>&) const;

private:

    const string _nonPrintableEscaped;
    const string _printableEscaped;
    const EscapeMode _escapeMode;
    const unsigned char _cutOff;
};

StringLiteralGenerator::StringLiteralGenerator(const string& nonPrintableEscaped,
                                               const string& printableEscaped,
                                               EscapeMode escapeMode,
                                               unsigned char cutOff) :
    _nonPrintableEscaped(nonPrintableEscaped),
    _printableEscaped(printableEscaped + "\\\""),
    _escapeMode(escapeMode),
    _cutOff(cutOff)
{
}

string
StringLiteralGenerator::escapeASCIIChar(char c) const
{
    assert(static_cast<unsigned char>(c) < 128);

    string result;

    if(_nonPrintableEscaped.find(c) != string::npos)
    {
        switch(c)
        {
            case '\a':
            {
                result = "\\a";
                break;
            }
            case '\b':
            {
                result = "\\b";
                break;
            }
            case '\f':
            {
                result = "\\f";
                break;
            }
            case '\n':
            {
                result = "\\n";
                break;
            }
            case '\r':
            {
                result = "\\r";
                break;
            }
            case '\t':
            {
                result = "\\t";
                break;
            }
            case '\v':
            {
                result = "\\v";
                break;
            }
            case '\0':
            {
                result = "\\0";
                break;
            }
            case '\x20':
            {
                result = "\\s";
                break;
            }
            case '\x1b':
            {
                result = "\\e";
                break;
            }
            default:
            {
                // The caller cannot add additional non-printable ASCII characters!
                assert(0);
            }
        }
    }
    else if(_printableEscaped.find(c) != string::npos)
    {
        result = '\\';
        result += c;
    }
    else if(c >= 32 && c <= 126)
    {
        // Other printable ASCII
        result = c;
    }
    else
    {
        // Other non-printable ASCII character
        ostringstream os;
        if((static_cast<unsigned char>(c) < _cutOff) || (_escapeMode == Octal))
        {
            os << "\\" << oct << setfill('0') << setw(3) << static_cast<unsigned int>(c & 0xFF);
        }
        else
        {
            os << "\\u" << hex << setfill('0') << setw(4) << static_cast<unsigned int>(c & 0xFF);
        }
        result = os.str();
    }
    return result;
}

string
StringLiteralGenerator::escapeCodePoint(unsigned int codePoint) const
{
    if(codePoint < 128)
    {
        return escapeASCIIChar(static_cast<char>(codePoint));
    }
    else if(_escapeMode == Octal)
    {
        vector<unsigned int> u32buffer;
        u32buffer.push_back(codePoint);
        vector<unsigned char> u8buffer = fromUTF32(u32buffer);

        ostringstream os;
        for(vector<unsigned char>::const_iterator q = u8buffer.begin(); q != u8buffer.end(); ++q)
        {
            os << "\\" << setfill('0') << setw(3) << oct << static_cast<unsigned int>(*q);
        }
        return os.str();
    }
    else
    {
        ostringstream os;
        if(codePoint < _cutOff)
        {
            //
            // Output octal escape
            //
            os << "\\" << setfill('0') << setw(3) << oct << codePoint;
        }
        else if(codePoint <= 0xFFFF)
        {
            os << "\\u" << setfill('0') << setw(4) << hex << codePoint;
        }
        else if(_escapeMode == ShortUCN)
        {
            //
            // Convert to surrogate pair
            //
            unsigned int highSurrogate = ((codePoint - 0x10000) / 0x400) + 0xD800;
            unsigned int lowSurrogate = ((codePoint - 0x10000) % 0x400) + 0xDC00;
            os << "\\u" << setfill('0') << setw(4) << hex << highSurrogate;
            os << "\\u" << setfill('0') << setw(4) << hex << lowSurrogate;
        }
        else if(_escapeMode == EC6UCN)
        {
            os << "\\u{" << hex << codePoint << "}";
        }
        else
        {
            os << "\\U" << setfill('0') << setw(8) << hex << codePoint;
        }
        return os.str();
    }
}

string
StringLiteralGenerator::flushU8Buffer(vector<unsigned char>& u8buffer) const
{
    if(u8buffer.empty())
    {
        return "";
    }
    else
    {
        ostringstream os;
        vector<unsigned int> u32buffer = toUTF32(u8buffer);

        for(vector<unsigned int>::const_iterator p = u32buffer.begin(); p != u32buffer.end(); ++p)
        {
            os << escapeCodePoint(*p);
        }
        u8buffer.clear();
        return os.str();
    }
}

}

string
Slice::toStringLiteral(const string& value,
                       const string& nonPrintableEscaped,
                       const string& printableEscaped,
                       EscapeMode escapeMode,
                       unsigned char cutOff)
{
    StringLiteralGenerator generator(nonPrintableEscaped, printableEscaped, escapeMode, cutOff);

    ostringstream os;

    if(escapeMode != Octal)
    {
        vector<unsigned char> u8buffer; // Buffer to convert multibyte characters

        for(size_t i = 0; i < value.size(); ++i)
        {
            char c = value[i];

            if(static_cast<unsigned char>(c) >= 128)
            {
                // New UTF-8 byte
                u8buffer.push_back(static_cast<unsigned char>(c));
            }
            else
            {
                //
                // First write any outstanding UTF-8 -encoded characters
                //
                os << generator.flushU8Buffer(u8buffer);

                if(c == '\\')
                {
                    if(i + 1 == value.size())
                    {
                        // trailing backslash, add a second one
                        os << "\\\\";
                    }
                    else
                    {
                        c = value[++i];

                        if(c == '\\')
                        {
                            os << "\\\\";
                        }
                        else if(c == 'u' || c == 'U')
                        {
                            size_t sz = c == 'U' ? 8 : 4;
                            string codePointStr = value.substr(i + 1, sz);
                            assert(codePointStr.size() == sz);

                            IceUtil::Int64 v = IceUtilInternal::strToInt64(codePointStr.c_str(), 0, 16);
                            if(v < 128)
                            {
                                // ASCII character that may need to escaped in languages such as Java
                                os << generator.escapeASCIIChar(static_cast<char>(v));
                            }
                            else if(escapeMode == UCN || c == 'u')
                            {
                                // keep this escape as is
                                os << "\\" << c << codePointStr;
                            }
                            else
                            {
                                os << generator.escapeCodePoint(static_cast<unsigned int>(v));
                            }
                            i += sz;
                        }
                        else
                        {
                            // unescaped backslash: escape it!
                            os << "\\\\";
                            os << generator.escapeASCIIChar(c);
                        }
                    }
                }
                else
                {
                    os << generator.escapeASCIIChar(c);
                }
            }
        }

        //
        // Write any outstanding UTF-8 -encoded characters
        //
        os << generator.flushU8Buffer(u8buffer);
    }
    else
    {
        assert(escapeMode == Octal);

        for(size_t i = 0; i < value.size(); ++i)
        {
            char c = value[i];

            if(static_cast<unsigned char>(c) >= 128)
            {
                // Write octal escape
                os << "\\" << setfill('0') << setw(3) << oct << static_cast<unsigned int>(c & 0xFF);
            }
            else if(c == '\\')
            {
                if(i + 1 == value.size())
                {
                    // trailing backslash, add a second one
                    os << "\\\\";
                }
                else
                {
                    c = value[++i];

                    if(c == '\\')
                    {
                        os << "\\\\";
                    }
                    else if(c == 'u' || c == 'U')
                    {
                        //
                        // Convert code point to UTF-8 bytes and write the escaped bytes
                        //
                        size_t sz = c == 'U' ? 8 : 4;
                        string codePointStr = value.substr(i + 1, sz);
                        assert(codePointStr.size() == sz);

                        IceUtil::Int64 v = IceUtilInternal::strToInt64(codePointStr.c_str(), 0, 16);
                        os << generator.escapeCodePoint(static_cast<unsigned int>(v));
                        i += sz;
                    }
                    else
                    {
                        // unescaped backslash
                        os << "\\\\";
                        os << generator.escapeASCIIChar(c);
                    }
                }
            }
            else
            {
                os << generator.escapeASCIIChar(c);
            }
        }
    }
    return os.str();
}
