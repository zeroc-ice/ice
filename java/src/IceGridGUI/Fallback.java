// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI;

public class Fallback extends javax.swing.JApplet
{
    public void start()
    {
        IceGridGUI.Main.main(MainProxy.args());
    }
}