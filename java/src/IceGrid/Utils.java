// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

import javax.swing.Icon;
import javax.swing.ImageIcon;

public class Utils
{
    static public Icon getIcon(String path)
    {
	java.net.URL imgURL = Utils.class.getResource(path);
	assert(imgURL != null);
	return new ImageIcon(imgURL);
    }
}
