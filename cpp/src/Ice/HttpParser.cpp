// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Config.h>
#include <Ice/HttpParser.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::WebSocketException::WebSocketException(const string& r) :
    reason(r)
{
}

IceInternal::HttpParser::HttpParser() :
    _type(TypeUnknown),
    _versionMajor(0),
    _versionMinor(0),
    _status(0),
    _state(StateInit)
{
}

const Ice::Byte*
IceInternal::HttpParser::isCompleteMessage(const Ice::Byte* begin, const Ice::Byte* end) const
{
    const Ice::Byte* p = begin;

    //
    // Skip any leading CR-LF characters.
    //
    while(p < end)
    {
        char ch = static_cast<char>(*p);
        if(ch != '\r' && ch != '\n')
        {
            break;
        }
        ++p;
    }

    //
    // Look for adjacent CR-LF/CR-LF or LF/LF.
    //
    bool seenFirst = false;
    while(p < end)
    {
        char ch = static_cast<char>(*p++);
        if(ch == '\n')
        {
            if(seenFirst)
            {
                return p;
            }
            else
            {
                seenFirst = true;
            }
        }
        else if(ch != '\r')
        {
            seenFirst = false;
        }
    }

    return 0;
}

bool
IceInternal::HttpParser::parse(const Ice::Byte* begin, const Ice::Byte* end)
{
    const Ice::Byte* p = begin;
    const Ice::Byte* start = 0;
    const string::value_type CR = '\r';
    const string::value_type LF = '\n';

    if(_state == StateComplete)
    {
        _state = StateInit;
    }

    while(p != end && _state != StateComplete)
    {
        char c = static_cast<char>(*p);

        switch(_state)
        {
        case StateInit:
        {
            _method.clear();
            _uri.clear();
            _versionMajor = -1;
            _versionMinor = -1;
            _status = -1;
            _reason.clear();
            _headers.clear();
            _state = StateType;
            continue;
        }
        case StateType:
        {
            if(c == CR || c == LF)
            {
                break;
            }
            else if(c == 'H')
            {
                //
                // Could be the start of "HTTP/1.1" or "HEAD".
                //
                _state = StateTypeCheck;
                break;
            }
            else
            {
                _state = StateRequest;
                continue;
            }
        }
        case StateTypeCheck:
        {
            if(c == 'T') // Continuing "H_T_TP/1.1"
            {
                _state = StateResponse;
            }
            else if(c == 'E') // Expecting "HEAD"
            {
                _state = StateRequest;
                _method.push_back('H');
                _method.push_back('E');
            }
            else
            {
                throw WebSocketException("malformed request or response");
            }
            break;
        }
        case StateRequest:
        {
            _type = TypeRequest;
            _state = StateRequestMethod;
            continue;
        }
        case StateRequestMethod:
        {
            if(c == ' ' || c == CR || c == LF)
            {
                _state = StateRequestMethodSP;
                continue;
            }
            _method.push_back(c);
            break;
        }
        case StateRequestMethodSP:
        {
            if(c == ' ')
            {
                break;
            }
            else if(c == CR || c == LF)
            {
                throw WebSocketException("malformed request");
            }
            _state = StateRequestURI;
            continue;
        }
        case StateRequestURI:
        {
            if(c == ' ' || c == CR || c == LF)
            {
                _state = StateRequestURISP;
                continue;
            }
            _uri.push_back(c);
            break;
        }
        case StateRequestURISP:
        {
            if(c == ' ')
            {
                break;
            }
            else if(c == CR || c == LF)
            {
                throw WebSocketException("malformed request");
            }
            _state = StateVersion;
            continue;
        }
        case StateRequestLF:
        {
            if(c != LF)
            {
                throw WebSocketException("malformed request");
            }
            _state = StateHeaderFieldStart;
            break;
        }
        case StateHeaderFieldStart:
        {
            //
            // We've already seen a LF to reach this state.
            //
            // Another CR or LF indicates the end of the header fields.
            //
            if(c == CR)
            {
                _state = StateHeaderFieldEndLF;
                break;
            }
            else if(c == LF)
            {
                _state = StateComplete;
                break;
            }
            else if(c == ' ')
            {
                //
                // Could be a continuation line.
                //
                _state = StateHeaderFieldContStart;
                break;
            }

            _state = StateHeaderFieldNameStart;
            continue;
        }
        case StateHeaderFieldContStart:
        {
            if(c == ' ')
            {
                break;
            }

            _state = StateHeaderFieldCont;
            start = p;
            continue;
        }
        case StateHeaderFieldCont:
        {
            if(c == CR || c == LF)
            {
                if(p > start)
                {
                    if(_headerName.empty())
                    {
                        throw WebSocketException("malformed header");
                    }
                    HeaderFields::iterator q = _headers.find(_headerName);
                    assert(q != _headers.end());
                    q->second.second = q->second.second + " " + string(start, p);
                    _state = c == CR ? StateHeaderFieldLF : StateHeaderFieldStart;
                }
                else
                {
                    //
                    // Could mark the end of the header fields.
                    //
                    _state = c == CR ? StateHeaderFieldEndLF : StateComplete;
                }
            }

            break;
        }
        case StateHeaderFieldNameStart:
        {
            assert(c != ' ');
            start = p;
            _headerName.clear();
            _state = StateHeaderFieldName;
            continue;
        }
        case StateHeaderFieldName:
        {
            if(c == ' ' || c == ':')
            {
                _state = StateHeaderFieldNameEnd;
                continue;
            }
            else if(c == CR || c == LF)
            {
                throw WebSocketException("malformed header");
            }
            break;
        }
        case StateHeaderFieldNameEnd:
        {
            if(_headerName.empty())
            {
                _headerName = IceUtilInternal::toLower(string(start, p));
                HeaderFields::iterator q = _headers.find(_headerName);
                //
                // Add a placeholder entry if necessary.
                //
                if(q == _headers.end())
                {
                    _headers[_headerName] = make_pair(string(start, p), "");
                }
            }

            if(c == ' ')
            {
                break;
            }
            else if(c != ':' || p == start)
            {
                throw WebSocketException("malformed header");
            }

            _state = StateHeaderFieldValueStart;
            break;
        }
        case StateHeaderFieldValueStart:
        {
            if(c == ' ')
            {
                break;
            }

            //
            // Check for "Name:\r\n"
            //
            if(c == CR)
            {
                _state = StateHeaderFieldLF;
                break;
            }
            else if(c == LF)
            {
                _state = StateHeaderFieldStart;
                break;
            }

            start = p;
            _state = StateHeaderFieldValue;
            continue;
        }
        case StateHeaderFieldValue:
        {
            if(c == CR || c == LF)
            {
                _state = StateHeaderFieldValueEnd;
                continue;
            }
            break;
        }
        case StateHeaderFieldValueEnd:
        {
            assert(c == CR || c == LF);
            if(p > start)
            {
                HeaderFields::iterator q = _headers.find(_headerName);
                if(q == _headers.end())
                {
                    throw WebSocketException("malformed header");
                }
                else if(q->second.second.empty())
                {
                    q->second.second = string(start, p);
                }
                else
                {
                    q->second.second = q->second.second + ", " + string(start, p);
                }
            }

            if(c == CR)
            {
                _state = StateHeaderFieldLF;
            }
            else
            {
                _state = StateHeaderFieldStart;
            }
            break;
        }
        case StateHeaderFieldLF:
        {
            if(c != LF)
            {
                throw WebSocketException("malformed header");
            }
            _state = StateHeaderFieldStart;
            break;
        }
        case StateHeaderFieldEndLF:
        {
            if(c != LF)
            {
                throw WebSocketException("malformed header");
            }
            _state = StateComplete;
            break;
        }
        case StateVersion:
        {
            if(c != 'H')
            {
                throw WebSocketException("malformed version");
            }
            _state = StateVersionH;
            break;
        }
        case StateVersionH:
        {
            if(c != 'T')
            {
                throw WebSocketException("malformed version");
            }
            _state = StateVersionHT;
            break;
        }
        case StateVersionHT:
        {
            if(c != 'T')
            {
                throw WebSocketException("malformed version");
            }
            _state = StateVersionHTT;
            break;
        }
        case StateVersionHTT:
        {
            if(c != 'P')
            {
                throw WebSocketException("malformed version");
            }
            _state = StateVersionHTTP;
            break;
        }
        case StateVersionHTTP:
        {
            if(c != '/')
            {
                throw WebSocketException("malformed version");
            }
            _state = StateVersionMajor;
            break;
        }
        case StateVersionMajor:
        {
            if(c == '.')
            {
                if(_versionMajor == -1)
                {
                    throw WebSocketException("malformed version");
                }
                _state = StateVersionMinor;
                break;
            }
            else if(c < '0' || c > '9')
            {
                throw WebSocketException("malformed version");
            }
            if(_versionMajor == -1)
            {
                _versionMajor = 0;
            }
            _versionMajor *= 10;
            _versionMajor += (c - '0');
            break;
        }
        case StateVersionMinor:
        {
            if(c == CR)
            {
                if(_versionMinor == -1 || _type != TypeRequest)
                {
                    throw WebSocketException("malformed version");
                }
                _state = StateRequestLF;
                break;
            }
            else if(c == LF)
            {
                if(_versionMinor == -1 || _type != TypeRequest)
                {
                    throw WebSocketException("malformed version");
                }
                _state = StateHeaderFieldStart;
                break;
            }
            else if(c == ' ')
            {
                if(_versionMinor == -1 || _type != TypeResponse)
                {
                    throw WebSocketException("malformed version");
                }
                _state = StateResponseVersionSP;
                break;
            }
            else if(c < '0' || c > '9')
            {
                throw WebSocketException("malformed version");
            }
            if(_versionMinor == -1)
            {
                _versionMinor = 0;
            }
            _versionMinor *= 10;
            _versionMinor += (c - '0');
            break;
        }
        case StateResponse:
        {
            _type = TypeResponse;
            _state = StateVersionHT;
            continue;
        }
        case StateResponseVersionSP:
        {
            if(c == ' ')
            {
                break;
            }

            _state = StateResponseStatus;
            continue;
        }
        case StateResponseStatus:
        {
            // TODO: Is reason string optional?
            if(c == CR)
            {
                if(_status == -1)
                {
                    throw WebSocketException("malformed response status");
                }
                _state = StateResponseLF;
                break;
            }
            else if(c == LF)
            {
                if(_status == -1)
                {
                    throw WebSocketException("malformed response status");
                }
                _state = StateHeaderFieldStart;
                break;
            }
            else if(c == ' ')
            {
                if(_status == -1)
                {
                    throw WebSocketException("malformed response status");
                }
                _state = StateResponseReasonStart;
                break;
            }
            else if(c < '0' || c > '9')
            {
                throw WebSocketException("malformed response status");
            }
            if(_status == -1)
            {
                _status = 0;
            }
            _status *= 10;
            _status += (c - '0');
            break;
        }
        case StateResponseReasonStart:
        {
            //
            // Skip leading spaces.
            //
            if(c == ' ')
            {
                break;
            }

            _state = StateResponseReason;
            start = p;
            continue;
        }
        case StateResponseReason:
        {
            if(c == CR || c == LF)
            {
                if(p > start)
                {
                    _reason = string(start, p);
                }
                _state = c == CR ? StateResponseLF : StateHeaderFieldStart;
            }

            break;
        }
        case StateResponseLF:
        {
            if(c != LF)
            {
                throw WebSocketException("malformed status line");
            }
            _state = StateHeaderFieldStart;
            break;
        }
        case StateComplete:
        {
            assert(false); // Shouldn't reach
        }
        }

        ++p;
    }

    return _state == StateComplete;
}

HttpParser::Type
IceInternal::HttpParser::type() const
{
    return _type;
}

string
IceInternal::HttpParser::method() const
{
    assert(_type == TypeRequest);
    return _method;
}

string
IceInternal::HttpParser::uri() const
{
    assert(_type == TypeRequest);
    return _uri;
}

int
IceInternal::HttpParser::versionMajor() const
{
    return _versionMajor;
}

int
IceInternal::HttpParser::versionMinor() const
{
    return _versionMinor;
}

int
IceInternal::HttpParser::status() const
{
    return _status;
}

string
IceInternal::HttpParser::reason() const
{
    return _reason;
}

bool
IceInternal::HttpParser::getHeader(const string& name, string& value, bool toLower) const
{
    HeaderFields::const_iterator q = _headers.find(IceUtilInternal::toLower(name));
    if(q != _headers.end())
    {
        value = IceUtilInternal::trim(q->second.second);
        if(toLower)
        {
            value = IceUtilInternal::toLower(value);
        }
        return true;
    }

    return false;
}

map<string, string>
IceInternal::HttpParser::getHeaders() const
{
    map<string, string> headers;
    for(HeaderFields::const_iterator q = _headers.begin(); q != _headers.end(); ++q)
    {
        headers.insert(make_pair(q->second.first, IceUtilInternal::trim(q->second.second)));
    }
    return headers;
}
