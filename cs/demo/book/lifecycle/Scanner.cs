using System;
// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

class Scanner
{
    internal Scanner(Parser p)
    {
        _parser = p;
    }

    internal virtual Token nextToken()
    {
        string s = next();
        if(s == null)
        {
            return null;
        }
        if(s.Equals(";"))
        {
            return new Token(Token.TOK_SEMI);
        }
        else if(s.Equals("help"))
        {
            return new Token(Token.TOK_HELP);
        }
        else if(s.Equals("exit") || s.Equals("quit"))
        {
            return new Token(Token.TOK_EXIT);
        }
        else if(s.Equals("ls"))
        {
            return new Token(Token.TOK_LIST);
        }
        else if(s.Equals("lr"))
        {
            return new Token(Token.TOK_LIST_RECURSIVE);
        }
        else if(s.Equals("mkfile"))
        {
            return new Token(Token.TOK_CREATE_FILE);
        }
        else if(s.Equals("mkdir"))
        {
            return new Token(Token.TOK_CREATE_DIR);
        }
        else if(s.Equals("pwd"))
        {
            return new Token(Token.TOK_PWD);
        }
        else if(s.Equals("cd"))
        {
            return new Token(Token.TOK_CD);
        }
        else if(s.Equals("cat"))
        {
            return new Token(Token.TOK_CAT);
        }
        else if(s.Equals("write"))
        {
            return new Token(Token.TOK_WRITE);
        }
        else if(s.Equals("rm"))
        {
            return new Token(Token.TOK_RM);
        }
        else
        {
            return new Token(Token.TOK_STRING, s);
        }
    }

    private class EndOfInput : Exception
    {
    }

    private char @get()
    {
        //
        // If there is an character in the unget buffer, return it.
        //
        if(_unget)
        {
            _unget = false;
            return _ungetChar;
        }

        //
        // No current buffer?
        //
        if(_buf == null)
        {
            _buf = _parser.Input;
            _pos = 0;
            if(_buf == null)
            {
                throw new EndOfInput();
            }
        }

        //
        // At the end-of-buffer?
        //
        while(_pos >= _buf.Length)
        {
            _buf = null;
            _pos = 0;
            return '\n';
        }

        return _buf[_pos++];
    }

    //
    // unget only works with one character.
    //
    private void  unget(char c)
    {
        Debug.Assert(!_unget);
        _unget = true;
        _ungetChar = c;
    }

    private string next()
    {
        //
        // Eat any whitespace.
        //
        char c;
        try
        {
            do
            {
                c = @get();
            }
            while(char.IsWhiteSpace(c) && c != '\n');
        }
        catch(EndOfInput)
        {
            return null;
        }

        System.Text.StringBuilder buf = new System.Text.StringBuilder();

        if(c == ';' || c == '\n')
        {
            buf.Append(';');
        }
        else if(c == '\'')
        {
            try
            {
                while(true)
                {
                    c = @get();
                    if(c == '\'')
                    {
                        break;
                    }
                    else
                    {
                        buf.Append(c);
                    }
                }
            }
            catch(EndOfInput)
            {
                _parser.warning("EOF in string");
            }
        }
        else if(c == '\"')
        {
            try
            {
                while(true)
                {
                    c = @get();
                    if(c == '\"')
                    {
                        break;
                    }
                    else if(c == '\\')
                    {
                        try
                        {
                            char next = @get();
                            switch(next)
                            {
                                case '\\':
                                case '"':
                                {
                                    buf.Append(next);
                                    break;
                                }

                                case 'n':
                                {
                                    buf.Append('\n');
                                    break;
                                }

                                case 'r':
                                {
                                    buf.Append('\r');
                                    break;
                                }

                                case 't':
                                {
                                    buf.Append('\t');
                                    break;
                                }


                                case 'f':
                                {
                                    buf.Append('\f');
                                    break;
                                }

                                default:
                                {
                                    buf.Append(c);
                                    unget(next);
                                }
                                break;
                            }
                        }
                        catch(EndOfInput)
                        {
                            buf.Append(c);
                        }
                    }
                    else
                    {
                        buf.Append(c);
                    }
                }
            }
            catch(EndOfInput)
            {
                _parser.warning("EOF in string");
            }
        }
        else
        {
            //
            // Otherwise it's a string.
            //
            try
            {
                do
                {
                    buf.Append(c);
                    c = @get();
                }
                while(!char.IsWhiteSpace(c) && c != ';' && c != '\n');

                unget(c);
            }
            catch(EndOfInput)
            {
            }
        }

        return buf.ToString();
    }

    private Parser _parser;
    private bool _unget = false;
    private char _ungetChar;
    private string _buf = null;
    private int _pos;
}
