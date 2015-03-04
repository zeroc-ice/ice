// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI;

import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.Icon;
import javax.swing.JFrame;
import javax.swing.JOptionPane;

import javax.swing.SwingUtilities;
import javax.swing.UIManager;

import java.util.prefs.Preferences;

public class Main extends JFrame
{
    public static void main(final String[] args)
    {
        //
        // Initialize L&F in main thread before Swing is used
        //
        try
        {
            if(System.getProperty("os.name").startsWith("Mac OS")) // OS X L&F
            {
                System.setProperty("apple.laf.useScreenMenuBar", "true");
                System.setProperty("com.apple.mrj.application.apple.menu.about.name", "IceGrid Admin");
                UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
            }
            else if(System.getProperty("os.name").startsWith("Windows"))
            {
                UIManager.setLookAndFeel("com.jgoodies.looks.windows.WindowsLookAndFeel");
            }
            else  // JGoodies L&F
            {
                UIManager.setLookAndFeel("com.jgoodies.looks.plastic.PlasticLookAndFeel");
            }
        }
        catch(Exception e)
        {
            System.err.println(e.toString());
        }

        SwingUtilities.invokeLater(new Runnable()
            {
                public void run()
                {
                    try
                    {
                        //
                        // Create and set up the window.
                        //
                        new Main(args);
                    }
                    catch(Ice.LocalException e)
                    {
                        JOptionPane.showMessageDialog(null,
                                                      e.toString(),
                                                      "Initialization failed",
                                                      JOptionPane.ERROR_MESSAGE);
                        System.exit(1);
                    }
                }
            });
    }

    Main(String[] args)
    {
        super("IceGrid Admin");
        javax.swing.ImageIcon icon = Utils.getIcon("/icons/16x16/grid.png");
        if(icon != null)
        {
            setIconImage(icon.getImage());
        }

        setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        addWindowListener(new WindowAdapter()
            {
                public void windowClosing(WindowEvent e)
                {
                    if(_coordinator != null)
                    {
                        _coordinator.exit(0);
                    }
                }
            });

        _coordinator = new Coordinator(this, new Ice.StringSeqHolder(args), Preferences.userNodeForPackage(getClass()));

        _coordinator.showMainFrame();
    }

    private Coordinator _coordinator;
}
