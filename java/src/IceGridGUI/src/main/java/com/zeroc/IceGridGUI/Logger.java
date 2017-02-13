// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;

public class Logger extends com.zeroc.Ice.LoggerI
{
    public
    Logger(JFrame mainFrame)
    {
        super("IceGrid GUI", "");

        _mainFrame = mainFrame;
    }

    @Override
    public void
    warning(final String message)
    {
        //
        // Ignore spurious selector wake up warnings in solaris.
        //
        if(OS_IS_SOLARIS && message.equals("spurious selector wake up"))
        {
            return;
        }
        SwingUtilities.invokeLater(() ->
            {
                JOptionPane.showMessageDialog(_mainFrame, message, "Warning - IceGrid GUI Logger",
                                              JOptionPane.WARNING_MESSAGE);
            });
    }

    @Override
    public void
    error(final String message)
    {
        SwingUtilities.invokeLater(() ->
            {
                JOptionPane.showMessageDialog(_mainFrame, message, "Error - IceGrid GUI Logger",
                                              JOptionPane.ERROR_MESSAGE);
            });
    }

    private final JFrame _mainFrame;
    
    private static final boolean OS_IS_SOLARIS = System.getProperty("os.name").equals("SunOS"); 
}
