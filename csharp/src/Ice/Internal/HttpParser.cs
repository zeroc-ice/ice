// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Text;

namespace Ice.Internal;

public sealed class WebSocketException : System.Exception
{
    public WebSocketException(string message)
        : base(message, null)
    {
    }
}

internal sealed class HttpParser
{
    internal HttpParser()
    {
        _type = Type.Unknown;
        _versionMajor = 0;
        _versionMinor = 0;
        _status = 0;
        _state = State.Init;
    }

    internal enum Type
    {
        Unknown,
        Request,
        Response
    }

    internal int isCompleteMessage(ByteBuffer buf, int begin, int end)
    {
        byte[] raw = buf.rawBytes();
        int p = begin;

        //
        // Skip any leading CR-LF characters.
        //
        while (p < end)
        {
            byte ch = raw[p];
            if (ch != (byte)'\r' && ch != (byte)'\n')
            {
                break;
            }
            ++p;
        }

        //
        // Look for adjacent CR-LF/CR-LF or LF/LF.
        //
        bool seenFirst = false;
        while (p < end)
        {
            byte ch = raw[p++];
            if (ch == (byte)'\n')
            {
                if (seenFirst)
                {
                    return p;
                }
                else
                {
                    seenFirst = true;
                }
            }
            else if (ch != (byte)'\r')
            {
                seenFirst = false;
            }
        }

        return -1;
    }

    internal bool parse(ByteBuffer buf, int begin, int end)
    {
        byte[] raw = buf.rawBytes();
        int p = begin;
        int start = 0;
        const char CR = '\r';
        const char LF = '\n';

        if (_state == State.Complete)
        {
            _state = State.Init;
        }

        while (p != end && _state != State.Complete)
        {
            char c = (char)raw[p];

            switch (_state)
            {
                case State.Init:
                {
                    _method = new StringBuilder();
                    _uri = new StringBuilder();
                    _versionMajor = -1;
                    _versionMinor = -1;
                    _status = -1;
                    _reason = "";
                    _headers.Clear();
                    _state = State.Type;
                    continue;
                }
                case State.Type:
                {
                    if (c == CR || c == LF)
                    {
                        break;
                    }
                    else if (c == 'H')
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
                case State.TypeCheck:
                {
                    if (c == 'T') // Continuing "H_T_TP/1.1"
                    {
                        _state = State.Response;
                    }
                    else if (c == 'E') // Expecting "HEAD"
                    {
                        _state = State.Request;
                        _method.Append('H');
                        _method.Append('E');
                    }
                    else
                    {
                        throw new WebSocketException("malformed request or response");
                    }
                    break;
                }
                case State.Request:
                {
                    _type = Type.Request;
                    _state = State.RequestMethod;
                    continue;
                }
                case State.RequestMethod:
                {
                    if (c == ' ' || c == CR || c == LF)
                    {
                        _state = State.RequestMethodSP;
                        continue;
                    }
                    _method.Append(c);
                    break;
                }
                case State.RequestMethodSP:
                {
                    if (c == ' ')
                    {
                        break;
                    }
                    else if (c == CR || c == LF)
                    {
                        throw new WebSocketException("malformed request");
                    }
                    _state = State.RequestURI;
                    continue;
                }
                case State.RequestURI:
                {
                    if (c == ' ' || c == CR || c == LF)
                    {
                        _state = State.RequestURISP;
                        continue;
                    }
                    _uri.Append(c);
                    break;
                }
                case State.RequestURISP:
                {
                    if (c == ' ')
                    {
                        break;
                    }
                    else if (c == CR || c == LF)
                    {
                        throw new WebSocketException("malformed request");
                    }
                    _state = State.Version;
                    continue;
                }
                case State.RequestLF:
                {
                    if (c != LF)
                    {
                        throw new WebSocketException("malformed request");
                    }
                    _state = State.HeaderFieldStart;
                    break;
                }
                case State.HeaderFieldStart:
                {
                    //
                    // We've already seen a LF to reach this state.
                    //
                    // Another CR or LF indicates the end of the header fields.
                    //
                    if (c == CR)
                    {
                        _state = State.HeaderFieldEndLF;
                        break;
                    }
                    else if (c == LF)
                    {
                        _state = State.Complete;
                        break;
                    }
                    else if (c == ' ')
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
                case State.HeaderFieldContStart:
                {
                    if (c == ' ')
                    {
                        break;
                    }

                    _state = State.HeaderFieldCont;
                    start = p;
                    continue;
                }
                case State.HeaderFieldCont:
                {
                    if (c == CR || c == LF)
                    {
                        if (p > start)
                        {
                            if (_headerName.Length == 0)
                            {
                                throw new WebSocketException("malformed header");
                            }
                            Debug.Assert(_headers.ContainsKey(_headerName));
                            string s = _headers[_headerName];
                            StringBuilder newValue = new StringBuilder(s);
                            newValue.Append(' ');
                            for (int i = start; i < p; ++i)
                            {
                                newValue.Append((char)raw[i]);
                            }
                            _headers[_headerName] = newValue.ToString();
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
                case State.HeaderFieldNameStart:
                {
                    Debug.Assert(c != ' ');
                    start = p;
                    _headerName = "";
                    _state = State.HeaderFieldName;
                    continue;
                }
                case State.HeaderFieldName:
                {
                    if (c == ' ' || c == ':')
                    {
                        _state = State.HeaderFieldNameEnd;
                        continue;
                    }
                    else if (c == CR || c == LF)
                    {
                        throw new WebSocketException("malformed header");
                    }
                    break;
                }
                case State.HeaderFieldNameEnd:
                {
                    if (_headerName.Length == 0)
                    {
                        StringBuilder str = new StringBuilder();
                        for (int i = start; i < p; ++i)
                        {
                            str.Append((char)raw[i]);
                        }
                        _headerName = str.ToString().ToLowerInvariant();
                        //
                        // Add a placeholder entry if necessary.
                        //
                        if (!_headers.ContainsKey(_headerName))
                        {
                            _headers[_headerName] = "";
                            _headerNames[_headerName] = str.ToString();
                        }
                    }

                    if (c == ' ')
                    {
                        break;
                    }
                    else if (c != ':' || p == start)
                    {
                        throw new WebSocketException("malformed header");
                    }

                    _state = State.HeaderFieldValueStart;
                    break;
                }
                case State.HeaderFieldValueStart:
                {
                    if (c == ' ')
                    {
                        break;
                    }

                    //
                    // Check for "Name:\r\n"
                    //
                    if (c == CR)
                    {
                        _state = State.HeaderFieldLF;
                        break;
                    }
                    else if (c == LF)
                    {
                        _state = State.HeaderFieldStart;
                        break;
                    }

                    start = p;
                    _state = State.HeaderFieldValue;
                    continue;
                }
                case State.HeaderFieldValue:
                {
                    if (c == CR || c == LF)
                    {
                        _state = State.HeaderFieldValueEnd;
                        continue;
                    }
                    break;
                }
                case State.HeaderFieldValueEnd:
                {
                    Debug.Assert(c == CR || c == LF);
                    if (p > start)
                    {
                        StringBuilder str = new StringBuilder();
                        for (int i = start; i < p; ++i)
                        {
                            str.Append((char)raw[i]);
                        }
                        string s = null;
                        if (!_headers.TryGetValue(_headerName, out s) || s.Length == 0)
                        {
                            _headers[_headerName] = str.ToString();
                        }
                        else
                        {
                            _headers[_headerName] = s + ", " + str.ToString();
                        }
                    }

                    if (c == CR)
                    {
                        _state = State.HeaderFieldLF;
                    }
                    else
                    {
                        _state = State.HeaderFieldStart;
                    }
                    break;
                }
                case State.HeaderFieldLF:
                {
                    if (c != LF)
                    {
                        throw new WebSocketException("malformed header");
                    }
                    _state = State.HeaderFieldStart;
                    break;
                }
                case State.HeaderFieldEndLF:
                {
                    if (c != LF)
                    {
                        throw new WebSocketException("malformed header");
                    }
                    _state = State.Complete;
                    break;
                }
                case State.Version:
                {
                    if (c != 'H')
                    {
                        throw new WebSocketException("malformed version");
                    }
                    _state = State.VersionH;
                    break;
                }
                case State.VersionH:
                {
                    if (c != 'T')
                    {
                        throw new WebSocketException("malformed version");
                    }
                    _state = State.VersionHT;
                    break;
                }
                case State.VersionHT:
                {
                    if (c != 'T')
                    {
                        throw new WebSocketException("malformed version");
                    }
                    _state = State.VersionHTT;
                    break;
                }
                case State.VersionHTT:
                {
                    if (c != 'P')
                    {
                        throw new WebSocketException("malformed version");
                    }
                    _state = State.VersionHTTP;
                    break;
                }
                case State.VersionHTTP:
                {
                    if (c != '/')
                    {
                        throw new WebSocketException("malformed version");
                    }
                    _state = State.VersionMajor;
                    break;
                }
                case State.VersionMajor:
                {
                    if (c == '.')
                    {
                        if (_versionMajor == -1)
                        {
                            throw new WebSocketException("malformed version");
                        }
                        _state = State.VersionMinor;
                        break;
                    }
                    else if (c < '0' || c > '9')
                    {
                        throw new WebSocketException("malformed version");
                    }
                    if (_versionMajor == -1)
                    {
                        _versionMajor = 0;
                    }
                    _versionMajor *= 10;
                    _versionMajor += (int)(c - '0');
                    break;
                }
                case State.VersionMinor:
                {
                    if (c == CR)
                    {
                        if (_versionMinor == -1 || _type != Type.Request)
                        {
                            throw new WebSocketException("malformed version");
                        }
                        _state = State.RequestLF;
                        break;
                    }
                    else if (c == LF)
                    {
                        if (_versionMinor == -1 || _type != Type.Request)
                        {
                            throw new WebSocketException("malformed version");
                        }
                        _state = State.HeaderFieldStart;
                        break;
                    }
                    else if (c == ' ')
                    {
                        if (_versionMinor == -1 || _type != Type.Response)
                        {
                            throw new WebSocketException("malformed version");
                        }
                        _state = State.ResponseVersionSP;
                        break;
                    }
                    else if (c < '0' || c > '9')
                    {
                        throw new WebSocketException("malformed version");
                    }
                    if (_versionMinor == -1)
                    {
                        _versionMinor = 0;
                    }
                    _versionMinor *= 10;
                    _versionMinor += c - '0';
                    break;
                }
                case State.Response:
                {
                    _type = Type.Response;
                    _state = State.VersionHT;
                    continue;
                }
                case State.ResponseVersionSP:
                {
                    if (c == ' ')
                    {
                        break;
                    }

                    _state = State.ResponseStatus;
                    continue;
                }
                case State.ResponseStatus:
                {
                    // TODO: Is reason string optional?
                    if (c == CR)
                    {
                        if (_status == -1)
                        {
                            throw new WebSocketException("malformed response status");
                        }
                        _state = State.ResponseLF;
                        break;
                    }
                    else if (c == LF)
                    {
                        if (_status == -1)
                        {
                            throw new WebSocketException("malformed response status");
                        }
                        _state = State.HeaderFieldStart;
                        break;
                    }
                    else if (c == ' ')
                    {
                        if (_status == -1)
                        {
                            throw new WebSocketException("malformed response status");
                        }
                        _state = State.ResponseReasonStart;
                        break;
                    }
                    else if (c < '0' || c > '9')
                    {
                        throw new WebSocketException("malformed response status");
                    }
                    if (_status == -1)
                    {
                        _status = 0;
                    }
                    _status *= 10;
                    _status += c - '0';
                    break;
                }
                case State.ResponseReasonStart:
                {
                    //
                    // Skip leading spaces.
                    //
                    if (c == ' ')
                    {
                        break;
                    }

                    _state = State.ResponseReason;
                    start = p;
                    continue;
                }
                case State.ResponseReason:
                {
                    if (c == CR || c == LF)
                    {
                        if (p > start)
                        {
                            StringBuilder str = new StringBuilder();
                            for (int i = start; i < p; ++i)
                            {
                                str.Append((char)raw[i]);
                            }
                            _reason = str.ToString();
                        }
                        _state = c == CR ? State.ResponseLF : State.HeaderFieldStart;
                    }

                    break;
                }
                case State.ResponseLF:
                {
                    if (c != LF)
                    {
                        throw new WebSocketException("malformed status line");
                    }
                    _state = State.HeaderFieldStart;
                    break;
                }
                case State.Complete:
                {
                    Debug.Assert(false); // Shouldn't reach
                    break;
                }
            }

            ++p;
        }

        return _state == State.Complete;
    }

    internal Type type()
    {
        return _type;
    }

    internal string method()
    {
        Debug.Assert(_type == Type.Request);
        return _method.ToString();
    }

    internal string uri()
    {
        Debug.Assert(_type == Type.Request);
        return _uri.ToString();
    }

    internal int versionMajor()
    {
        return _versionMajor;
    }

    internal int versionMinor()
    {
        return _versionMinor;
    }

    internal int status()
    {
        return _status;
    }

    internal string reason()
    {
        return _reason;
    }

    internal string getHeader(string name, bool toLower)
    {
        string s = null;
        if (_headers.TryGetValue(name.ToLowerInvariant(), out s))
        {
            return toLower ? s.Trim().ToLowerInvariant() : s.Trim();
        }

        return null;
    }

    internal Dictionary<string, string> getHeaders()
    {
        Dictionary<string, string> dict = new Dictionary<string, string>();
        foreach (KeyValuePair<string, string> e in _headers)
        {
            dict[_headerNames[e.Key]] = e.Value.Trim();
        }
        return dict;
    }

    private Type _type;

    private StringBuilder _method = new StringBuilder();
    private StringBuilder _uri = new StringBuilder();

    private readonly Dictionary<string, string> _headers = new Dictionary<string, string>();
    private readonly Dictionary<string, string> _headerNames = new Dictionary<string, string>();
    private string _headerName = "";

    private int _versionMajor;
    private int _versionMinor;

    private int _status;
    private string _reason;

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
