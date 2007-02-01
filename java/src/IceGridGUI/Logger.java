// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;

public class Logger extends Ice.LocalObjectImpl implements Ice.Logger
{
    public 
    Logger(JFrame mainFrame)
    {
        _mainFrame = mainFrame;
    }

    public void
    print(String message)
    {
        System.err.println(message);
    }

    public void
    trace(String category, String message)
    {
        StringBuffer s = new StringBuffer("[ ");
        s.append(_date.format(new java.util.Date()));
        s.append(' ');
        s.append(_prefix);
        s.append(category);
        s.append(": ");
        s.append(message);
        s.append(" ]");
        int idx = 0;
        while((idx = s.indexOf("\n", idx)) != -1)
        {
            s.insert(idx + 1, "  ");
            ++idx;
        }
        System.err.println(s.toString());
    }

    public void
    warning(final String message)
    {
        SwingUtilities.invokeLater(new Runnable() 
            {
                public void run()
                {
                    JOptionPane.showMessageDialog(
                        _mainFrame,
                        message,
                        "Warning - IceGrid Admin Logger",
                        JOptionPane.WARNING_MESSAGE);
                }
            });
    }

    public void
    error(final String message)
    {
        SwingUtilities.invokeLater(new Runnable() 
            {
                public void run()
                {
                    JOptionPane.showMessageDialog(
                        _mainFrame,
                        message,
                        "Error - IceGrid Admin Logger",
                        JOptionPane.ERROR_MESSAGE);
                }
            });
    }

    private final String _prefix = "IceGrid Admin: ";
    private final JFrame _mainFrame;
    private final java.text.SimpleDateFormat _date = new java.text.SimpleDateFormat("yy/MM/dd HH:mm:ss:SSS");
}