//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceGridGUI;

public interface Tab
{
    void selected();

    void showNode(TreeNodeBase node);
    void refresh();

    void back();
    void forward();

    boolean close();

    void save();
    void saveToRegistry(boolean restart);
    void saveToFile();
    void discardUpdates();
}
