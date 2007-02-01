// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Parser
{
    public Complex.Node
    parse(String buf)
        throws ParseError
    {
        _buf = buf;
        _pos = 0;
        _token = null;

        return start();
    }

    private Complex.Node
    start()
        throws ParseError
    {
        nextToken();
        Complex.Node node = expr();
        if(_token != null)
        {
            throw new ParseError("Extra garbage: " + _token);
        }
        return node;
    }

    private Complex.Node
    expr()
        throws ParseError
    {
        try
        {
            if(_token == null)
            {
                return null;
            }

            //
            // '(' expr ')'
            //
            if(_token.charAt(0) == '(')
            {
                nextToken();

                Complex.Node node = expr();
                if(_token.charAt(0) != ')')
                {
                    throw new ParseError("Expected ')'");
                }

                nextToken();
                return node;
            }

            //
            // expr | expr '+' expr | expr '*' expr
            //
            if(!Character.isDigit(_token.charAt(0)))
            {
                throw new ParseError("Expected number");
            }
            
            Complex.NumberNode number = new Complex.NumberNodeI(Integer.parseInt(_token));
            Complex.Node result = number;
            
            //
            // expr?
            //
            nextToken();
            if(_token != null)
            {
                //
                // expr '+' expr
                //
                if(_token.charAt(0) == '+')
                {
                    nextToken();
                    Complex.Node right = expr();
                    result = new Complex.AddNodeI(number, right);
                }
                
                //
                // expr '*' expr
                //
                else if(_token.charAt(0) == '*')
                {
                    nextToken();
                    Complex.Node right = expr();
                    result = new Complex.MultiplyNodeI(number, right);
                }
            }
            return result;
        }
        catch(NumberFormatException e)
        {
            ParseError ex = new ParseError("Error parsing number");
            ex.initCause(e);
            throw ex;
        }
    }

    private void
    nextToken()
    {
        //
        // Eat any whitespace.
        //
        while(_pos < _buf.length() && Character.isWhitespace(_buf.charAt(_pos)))
        {
            _pos++;
        }

        //
        // At the end-of-buffer?
        //
        if(_pos >= _buf.length())
        {
            _token = null;
            return;
        }

        StringBuffer buf = new StringBuffer();
        
        //
        // Get the next character
        //
        char c = _buf.charAt(_pos);
        
        //
        // '(', ')', '+' and '*' are tokens.
        //
        if(c == '(' || c == ')' || c == '+' || c == '*')
        {
            buf.append(c);
            ++_pos;
        }
        else
        {
            //
            // Otherwise it's a number.
            //
            while(_pos < _buf.length() && Character.isDigit(_buf.charAt(_pos)))
            {
                buf.append(_buf.charAt(_pos++));
            }
        }
        
        _token = buf.toString();
    }

    private String _buf;
    private int _pos;
    private String _token;
}
