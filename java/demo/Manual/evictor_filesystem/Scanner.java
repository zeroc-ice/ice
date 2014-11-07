// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class Scanner
{
    Scanner(Parser p)
    {
        _parser = p;
    }

    Token
    nextToken()
    {
        String s = next();
        if(s == null)
        {
            return null;
        }

        if(s.equals(";"))
        {
            return new Token(Token.TOK_SEMI);
        }
        else if(s.equals("help"))
        {
            return new Token(Token.TOK_HELP);
        }
        else if(s.equals("exit") || s.equals("quit"))
        {
            return new Token(Token.TOK_EXIT);
        }
        else if(s.equals("ls"))
        {
            return new Token(Token.TOK_LIST);
        }
        else if(s.equals("lr"))
        {
            return new Token(Token.TOK_LIST_RECURSIVE);
        }
        else if(s.equals("mkfile"))
        {
            return new Token(Token.TOK_CREATE_FILE);
        }
        else if(s.equals("mkdir"))
        {
            return new Token(Token.TOK_CREATE_DIR);
        }
        else if(s.equals("pwd"))
        {
            return new Token(Token.TOK_PWD);
        }
        else if(s.equals("cd"))
        {
            return new Token(Token.TOK_CD);
        }
        else if(s.equals("cat"))
        {
            return new Token(Token.TOK_CAT);
        }
        else if(s.equals("write"))
        {
            return new Token(Token.TOK_WRITE);
        }
        else if(s.equals("rm"))
        {
            return new Token(Token.TOK_RM);
        }
        else
        {
            return new Token(Token.TOK_STRING, s);
        }
    }

    static private class EndOfInput extends Exception
    {
    }

    private char
    get()
        throws EndOfInput
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
            _buf = _parser.getInput();
            _pos = 0;
            if(_buf == null)
            {
                throw new EndOfInput();
            }
        }

        //
        // At the end-of-buffer?
        //
        while(_pos >= _buf.length())
        {
            _buf = null;
            _pos = 0;
            return '\n';
        }

        return _buf.charAt(_pos++);
    }

    //
    // unget only works with one character.
    //
    private void
    unget(char c)
    {
        assert(!_unget);
        _unget = true;
        _ungetChar = c;
    }

    private String
    next()
    {
        //
        // Eat any whitespace.
        //
        char c;
        try
        {
            do
            {
                c = get();
            }
            while(Character.isWhitespace(c) && c != '\n');
        }
        catch(EndOfInput ignore)
        {
            return null;
        }

        StringBuilder buf = new StringBuilder(128);

        if(c == ';' || c == '\n')
        {
            buf.append(';');
        }
        else if(c == '\'')
        {
            try
            {
                while(true)
                {
                    c = get();
                    if(c == '\'')
                    {
                        break;
                    }
                    else
                    {
                        buf.append(c);
                    }
                }
            }
            catch(EndOfInput e)
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
                    c = get();
                    if(c == '\"')
                    {
                        break;
                    }
                    else if(c == '\\')
                    {
                        try
                        {
                            char next = get();
                            switch(next)
                            {
                                case '\\':
                                case '"':
                                {
                                    buf.append(next);
                                    break;
                                }

                                case 'n':
                                {
                                    buf.append('\n');
                                    break;
                                }

                                case 'r':
                                {
                                    buf.append('\r');
                                    break;
                                }

                                case 't':
                                {
                                    buf.append('\t');
                                    break;
                                }

                                case 'f':
                                {
                                    buf.append('\f');
                                    break;
                                }

                                default:
                                {
                                    buf.append(c);
                                    unget(next);
                                }
                            }
                        }
                        catch(EndOfInput e)
                        {
                            buf.append(c);
                        }
                    }
                    else
                    {
                        buf.append(c);
                    }
                }
            }
            catch(EndOfInput e)
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
                    buf.append(c);
                    c = get();
                }
                while(!Character.isWhitespace(c) && c != ';' && c != '\n');

                unget(c);
            }
            catch(EndOfInput ignore)
            {
            }
        }

        return buf.toString();
    }

    private Parser _parser;
    private boolean _unget = false;
    private char _ungetChar;
    private String _buf = null;
    private int _pos;
}
