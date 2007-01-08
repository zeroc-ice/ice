// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

public interface Tab
{
    void selected();

    void showNode(TreeNodeBase node);
    void refresh();

    void back();
    void forward();

    boolean close();

    void save();
    void saveToRegistry();
    void saveToFile();
    void discardUpdates();
}
