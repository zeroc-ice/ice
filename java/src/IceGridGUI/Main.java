// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

import javax.swing.SwingUtilities;
import javax.swing.UIManager;

import java.util.prefs.Preferences;

public class Main extends JFrame
{
    public static void main(final String[] args) 
    {
        SwingUtilities.invokeLater(new Runnable() 
            {
                public void run() 
                {
                    createAndShowGUI(args);
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

        _coordinator = 
            new Coordinator(this, new Ice.StringSeqHolder(args), 
                            Preferences.userNodeForPackage(getClass()));
        
        _coordinator.showMainFrame();
    }
   
    private static void createAndShowGUI(String[] args) 
    {
        try 
        {
            if(UIManager.getSystemLookAndFeelClassName().equals("apple.laf.AquaLookAndFeel"))
            {
                System.setProperty("apple.laf.useScreenMenuBar", "true");
                UIManager.setLookAndFeel("apple.laf.AquaLookAndFeel");
            }
            else  // JGoodies L&F
            {
                UIManager.setLookAndFeel("com.jgoodies.looks.plastic.PlasticXPLookAndFeel");
            }
        } 
        catch(Exception e) 
        {
            System.err.println(e.toString());
        }
        
        //
        // Create and set up the window.
        //
        new Main(args);
     
    }

    private Coordinator _coordinator;
}


