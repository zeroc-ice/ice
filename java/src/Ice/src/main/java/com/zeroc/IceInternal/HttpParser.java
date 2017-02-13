// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

final class HttpParser
{
    HttpParser()
    {
        _type = Type.Unknown;
        _versionMajor = 0;
        _versionMinor = 0;
        _status = 0;
        _state = State.Init;
    }

    private enum Type
    {
        Unknown,
        Request,
        Response
    }

    int isCompleteMessage(java.nio.ByteBuffer buf, int begin, int end)
    {
        int p = begin;

        //
        // Skip any leading CR-LF characters.
        //
        while(p < end)
        {
            byte ch = buf.get(p);
            if(ch != (byte)'\r' && ch != (byte)'\n')
            {
                break;
            }
            ++p;
        }

        //
        // Look for adjacent CR-LF/CR-LF or LF/LF.
        //
        boolean seenFirst = false;
        while(p < end)
        {
            byte ch = buf.get(p++);
            if(ch == (byte)'\n')
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
            else if(ch != (byte)'\r')
            {
                seenFirst = false;
            }
        }

        return -1;
    }

    boolean parse(java.nio.ByteBuffer buf, int begin, int end)
    {
        int p = begin;
        int start = 0;
        final char CR = '\r';
        final char LF = '\n';

        if(_state == State.Complete)
        {
            _state = State.Init;
        }

        while(p != end && _state != State.Complete)
        {
            char c = (char)buf.get(p);

            switch(_state)
            {
            case Init:
            {
                _method.setLength(0);
                _uri.setLength(0);
                _versionMajor = -1;
                _versionMinor = -1;
                _status = -1;
                _reason = "";
                _headers.clear();
                _state = State.Type;
                continue;
            }
            case Type:
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
                    _state = State.TypeCheck;
                    break;
                }
                else
                {
                    _state = State.Request;
                    continue;
                }
            }
            case TypeCheck:
            {
                if(c == 'T') // Continuing "H_T_TP/1.1"
                {
                    _state = State.Response;
                }
                else if(c == 'E') // Expecting "HEAD"
                {
                    _state = State.Request;
                    _method.append('H');
                    _method.append('E');
                }
                else
                {
                    throw new WebSocketException("malformed request or response");
                }
                break;
            }
            case Request:
            {
                _type = Type.Request;
                _state = State.RequestMethod;
                continue;
            }
            case RequestMethod:
            {
                if(c == ' ' || c == CR || c == LF)
                {
                    _state = State.RequestMethodSP;
                    continue;
                }
                _method.append(c);
                break;
            }
            case RequestMethodSP:
            {
                if(c == ' ')
                {
                    break;
                }
                else if(c == CR || c == LF)
                {
                    throw new WebSocketException("malformed request");
                }
                _state = State.RequestURI;
                continue;
            }
            case RequestURI:
            {
                if(c == ' ' || c == CR || c == LF)
                {
                    _state = State.RequestURISP;
                    continue;
                }
                _uri.append(c);
                break;
            }
            case RequestURISP:
            {
                if(c == ' ')
                {
                    break;
                }
                else if(c == CR || c == LF)
                {
                    throw new WebSocketException("malformed request");
                }
                _state = State.Version;
                continue;
            }
            case RequestLF:
            {
                if(c != LF)
                {
                    throw new WebSocketException("malformed request");
                }
                _state = State.HeaderFieldStart;
                break;
            }
            case HeaderFieldStart:
            {
                //
                // We've already seen a LF to reach this state.
                //
                // Another CR or LF indicates the end of the header fields.
                //
                if(c == CR)
                {
                    _state = State.HeaderFieldEndLF;
                    break;
                }
                else if(c == LF)
                {
                    _state = State.Complete;
                    break;
                }
                else if(c == ' ')
                {
                    //
                    // Could be a continuation line.
                    //
                    _state = State.HeaderFieldContStart;
                    break;
                }

                _state = State.HeaderFieldNameStart;
                continue;
            }
            case HeaderFieldContStart:
            {
                if(c == ' ')
                {
                    break;
                }

                _state = State.HeaderFieldCont;
                start = p;
                continue;
            }
            case HeaderFieldCont:
            {
                if(c == CR || c == LF)
                {
                    if(p > start)
                    {
                        if(_headerName.isEmpty())
                        {
                            throw new WebSocketException("malformed header");
                        }
                        String s = _headers.get(_headerName);
                        assert(s != null);
                        StringBuffer newValue = new StringBuffer(s);
                        newValue.append(' ');
                        for(int i = start; i < p; ++i)
                        {
                            newValue.append((char)buf.get(i));
                        }
                        _headers.put(_headerName, newValue.toString());
                        _state = c == CR ? State.HeaderFieldLF : State.HeaderFieldStart;
                    }
                    else
                    {
                        //
                        // Could mark the end of the header fields.
                        //
                        _state = c == CR ? State.HeaderFieldEndLF : State.Complete;
                    }
                }

                break;
            }
            case HeaderFieldNameStart:
            {
                assert(c != ' ');
                start = p;
                _headerName = "";
                _state = State.HeaderFieldName;
                continue;
            }
            case HeaderFieldName:
            {
                if(c == ' ' || c == ':')
                {
                    _state = State.HeaderFieldNameEnd;
                    continue;
                }
                else if(c == CR || c == LF)
                {
                    throw new WebSocketException("malformed header");
                }
                break;
            }
            case HeaderFieldNameEnd:
            {
                if(_headerName.isEmpty())
                {
                    StringBuffer str = new StringBuffer();
                    for(int i = start; i < p; ++i)
                    {
                        str.append((char)buf.get(i));
                    }
                    _headerName = str.toString().toLowerCase();
                    //
                    // Add a placeholder entry if necessary.
                    //
                    if(!_headers.containsKey(_headerName))
                    {
                        _headers.put(_headerName, "");
                        _headerNames.put(_headerName, str.toString());
                    }
                }

                if(c == ' ')
                {
                    break;
                }
                else if(c != ':' || p == start)
                {
                    throw new WebSocketException("malformed header");
                }

                _state = State.HeaderFieldValueStart;
                break;
            }
            case HeaderFieldValueStart:
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
                    _state = State.HeaderFieldLF;
                    break;
                }
                else if(c == LF)
                {
                    _state = State.HeaderFieldStart;
                    break;
                }

                start = p;
                _state = State.HeaderFieldValue;
                continue;
            }
            case HeaderFieldValue:
            {
                if(c == CR || c == LF)
                {
                    _state = State.HeaderFieldValueEnd;
                    continue;
                }
                break;
            }
            case HeaderFieldValueEnd:
            {
                assert(c == CR || c == LF);
                if(p > start)
                {
                    StringBuffer str = new StringBuffer();
                    for(int i = start; i < p; ++i)
                    {
                        str.append((char)buf.get(i));
                    }
                    String s = _headers.get(_headerName);
                    if(s == null || s.length() == 0)
                    {
                        _headers.put(_headerName, str.toString());
                    }
                    else
                    {
                        _headers.put(_headerName, s + ", " + str.toString());
                    }
                }

                if(c == CR)
                {
                    _state = State.HeaderFieldLF;
                }
                else
                {
                    _state = State.HeaderFieldStart;
                }
                break;
            }
            case HeaderFieldLF:
            {
                if(c != LF)
                {
                    throw new WebSocketException("malformed header");
                }
                _state = State.HeaderFieldStart;
                break;
            }
            case HeaderFieldEndLF:
            {
                if(c != LF)
                {
                    throw new WebSocketException("malformed header");
                }
                _state = State.Complete;
                break;
            }
            case Version:
            {
                if(c != 'H')
                {
                    throw new WebSocketException("malformed version");
                }
                _state = State.VersionH;
                break;
            }
            case VersionH:
            {
                if(c != 'T')
                {
                    throw new WebSocketException("malformed version");
                }
                _state = State.VersionHT;
                break;
            }
            case VersionHT:
            {
                if(c != 'T')
                {
                    throw new WebSocketException("malformed version");
                }
                _state = State.VersionHTT;
                break;
            }
            case VersionHTT:
            {
                if(c != 'P')
                {
                    throw new WebSocketException("malformed version");
                }
                _state = State.VersionHTTP;
                break;
            }
            case VersionHTTP:
            {
                if(c != '/')
                {
                    throw new WebSocketException("malformed version");
                }
                _state = State.VersionMajor;
                break;
            }
            case VersionMajor:
            {
                if(c == '.')
                {
                    if(_versionMajor == -1)
                    {
                        throw new WebSocketException("malformed version");
                    }
                    _state = State.VersionMinor;
                    break;
                }
                else if(c < '0' || c > '9')
                {
                    throw new WebSocketException("malformed version");
                }
                if(_versionMajor == -1)
                {
                    _versionMajor = 0;
                }
                _versionMajor *= 10;
                _versionMajor += c - '0';
                break;
            }
            case VersionMinor:
            {
                if(c == CR)
                {
                    if(_versionMinor == -1 || _type != Type.Request)
                    {
                        throw new WebSocketException("malformed version");
                    }
                    _state = State.RequestLF;
                    break;
                }
                else if(c == LF)
                {
                    if(_versionMinor == -1 || _type != Type.Request)
                    {
                        throw new WebSocketException("malformed version");
                    }
                    _state = State.HeaderFieldStart;
                    break;
                }
                else if(c == ' ')
                {
                    if(_versionMinor == -1 || _type != Type.Response)
                    {
                        throw new WebSocketException("malformed version");
                    }
                    _state = State.ResponseVersionSP;
                    break;
                }
                else if(c < '0' || c > '9')
                {
                    throw new WebSocketException("malformed version");
                }
                if(_versionMinor == -1)
                {
                    _versionMinor = 0;
                }
                _versionMinor *= 10;
                _versionMinor += c - '0';
                break;
            }
            case Response:
            {
                _type = Type.Response;
                _state = State.VersionHT;
                continue;
            }
            case ResponseVersionSP:
            {
                if(c == ' ')
                {
                    break;
                }

                _state = State.ResponseStatus;
                continue;
            }
            case ResponseStatus:
            {
                // TODO: Is reason string optional?
                if(c == CR)
                {
                    if(_status == -1)
                    {
                        throw new WebSocketException("malformed response status");
                    }
                    _state = State.ResponseLF;
                    break;
                }
                else if(c == LF)
                {
                    if(_status == -1)
                    {
                        throw new WebSocketException("malformed response status");
                    }
                    _state = State.HeaderFieldStart;
                    break;
                }
                else if(c == ' ')
                {
                    if(_status == -1)
                    {
                        throw new WebSocketException("malformed response status");
                    }
                    _state = State.ResponseReasonStart;
                    break;
                }
                else if(c < '0' || c > '9')
                {
                    throw new WebSocketException("malformed response status");
                }
                if(_status == -1)
                {
                    _status = 0;
                }
                _status *= 10;
                _status += c - '0';
                break;
            }
            case ResponseReasonStart:
            {
                //
                // Skip leading spaces.
                //
                if(c == ' ')
                {
                    break;
                }

                _state = State.ResponseReason;
                start = p;
                continue;
            }
            case ResponseReason:
            {
                if(c == CR || c == LF)
                {
                    if(p > start)
                    {
                        StringBuffer str = new StringBuffer();
                        for(int i = start; i < p; ++i)
                        {
                            str.append((char)buf.get(i));
                        }
                        _reason = str.toString();
                    }
                    _state = c == CR ? State.ResponseLF : State.HeaderFieldStart;
                }

                break;
            }
            case ResponseLF:
            {
                if(c != LF)
                {
                    throw new WebSocketException("malformed status line");
                }
                _state = State.HeaderFieldStart;
                break;
            }
            case Complete:
            {
                assert(false); // Shouldn't reach
            }
            }

            ++p;
        }

        return _state == State.Complete;
    }

    String uri()
    {
        assert(_type == Type.Request);
        return _uri.toString();
    }

    int versionMajor()
    {
        return _versionMajor;
    }

    int versionMinor()
    {
        return _versionMinor;
    }

    int status()
    {
        return _status;
    }

    String reason()
    {
        return _reason;
    }

    String getHeader(String name, boolean toLower)
    {
        String s = _headers.get(name.toLowerCase());
        if(s != null)
        {
            return toLower ? s.trim().toLowerCase() : s.trim();
        }

        return null;
    }

    java.util.Map<String, String> getHeaders()
    {
        java.util.Map<String, String> headers = new java.util.HashMap<>();
        for(java.util.Map.Entry<String, String> entry : _headers.entrySet())
        {
            headers.put(_headerNames.get(entry.getKey()), entry.getValue().trim()); // Return original header name.
        }
        return headers;
    }

    private Type _type;

    private StringBuffer _method = new StringBuffer();
    private StringBuffer _uri = new StringBuffer();

    private java.util.Map<String, String> _headers = new java.util.HashMap<>();
    private java.util.Map<String, String> _headerNames = new java.util.HashMap<>();
    private String _headerName = "";

    private int _versionMajor;
    private int _versionMinor;

    private int _status;
    private String _reason;

    private enum State
    {
        Init,
        Type,
        TypeCheck,
        Request,
        RequestMethod,
        RequestMethodSP,
        RequestURI,
        RequestURISP,
        RequestLF,
        HeaderFieldStart,
        HeaderFieldContStart,
        HeaderFieldCont,
        HeaderFieldNameStart,
        HeaderFieldName,
        HeaderFieldNameEnd,
        HeaderFieldValueStart,
        HeaderFieldValue,
        HeaderFieldValueEnd,
        HeaderFieldLF,
        HeaderFieldEndLF,
        Version,
        VersionH,
        VersionHT,
        VersionHTT,
        VersionHTTP,
        VersionMajor,
        VersionMinor,
        Response,
        ResponseVersionSP,
        ResponseStatus,
        ResponseReasonStart,
        ResponseReason,
        ResponseLF,
        Complete
    }

    private State _state;
}
