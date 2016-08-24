// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGridGUI.*;

import javax.swing.JTextField;

//
// A special field used to show/edit a list of strings separated by whitespace
//

public class ListTextField extends JTextField
{
    public ListTextField(int columns)
    {
        super(columns);
    }

    public void setList(java.util.List<String> list, final Utils.Resolver resolver)
    {
        Utils.Stringifier stringifier =  new Utils.Stringifier()
            {
                @Override
                public String toString(Object obj)
                {
                    return Utils.substitute((String)obj, resolver);
                }
            };

        setText(Utils.stringify(list, stringifier, " ").returnValue);
    }

    public java.util.LinkedList<String> getList()
    {
        String text = getText().trim();
        java.util.LinkedList<String> result = new java.util.LinkedList<>();

        while(text.length() > 0)
        {
            if(text.startsWith("\""))
            {
                int last = text.indexOf("\"", 1);
                if(last == -1)
                {
                    result.add(text.substring(1));
                    text = "";
                }
                else
                {
                    result.add(text.substring(1, last));
                    text = text.substring(last + 1).trim();
                }
            }
            else
            {
                String[] strings = text.split("\\s", 2);
                if(strings.length == 1)
                {
                    result.add(strings[0]);
                    text = "";
                }
                else
                {
                    result.add(strings[0]);
                    text = strings[1].trim();
                }
            }
        }
        return result;
    }
}
