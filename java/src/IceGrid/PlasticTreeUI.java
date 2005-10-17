// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;
//
// This is JGoodies PlasticTreeUI modified to work-around bug 4833524
// (the MacOS CTRL+click bug):
// http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=4833524
//

/*
 * Copyright (c) 2001-2005 JGoodies Karsten Lentzsch. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 *  o Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer. 
 *     
 *  o Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution. 
 *     
 *  o Neither the name of JGoodies Karsten Lentzsch nor the names of 
 *    its contributors may be used to endorse or promote products derived 
 *    from this software without specific prior written permission. 
 *     
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

// package com.jgoodies.looks.plastic;

import java.awt.Component;
import java.awt.Graphics;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;

import javax.swing.Icon;
import javax.swing.JComponent;
import javax.swing.plaf.ComponentUI;
import javax.swing.plaf.basic.BasicTreeUI;

import com.jgoodies.looks.Options;

/**
 * The JGoodies Plastic Look&amp;Feel implementation of <code>TreeUI</code>.
 * It provides two line styles: angled dashed lines, or no lines at all.
 * By default, lines are drawn. 
 * <p>
 * You can change the line style by setting a client property.
 * The property key and values are a subset of the values used
 * by the Metal L&amp;F tree. To hide lines use one of the following:
 * <pre>
 * JTree tree1 = new JTree();
 * tree1.putClientProperty("JTree.lineStyle", "None");
 * 
 * JTree tree2 = new JTree();
 * tree1.putClientProperty(Options.TREE_LINE_STYLE_KEY, 
 *                         Options.TREE_LINE_STYLE_NONE_VALUE);
 * </pre>
 * 
 * Although lines are shown by default, you could code:
 * <pre>
 * JTree tree1 = new JTree();
 * tree1.putClientProperty("JTree.lineStyle", "Angled");
 * 
 * JTree tree2 = new JTree();
 * tree1.putClientProperty(Options.TREE_LINE_STYLE_KEY, 
 *                         Options.TREE_LINE_STYLE_ANGLED_VALUE);
 * </pre>
 * 
 * @author  Karsten Lentzsch
 * @version $Revision$
 */

public final class PlasticTreeUI extends BasicTreeUI {

    private boolean linesEnabled = true;
    private PropertyChangeListener lineStyleHandler;


    public static ComponentUI createUI(JComponent b) {
        return new PlasticTreeUI();
    }

    // Installation ***********************************************************

    public void installUI(JComponent c) {
        super.installUI(c);
        updateLineStyle(c.getClientProperty(Options.TREE_LINE_STYLE_KEY));
        lineStyleHandler = new LineStyleHandler();
        c.addPropertyChangeListener(lineStyleHandler);
    }

    public void uninstallUI(JComponent c) {
        c.removePropertyChangeListener(lineStyleHandler);
        super.uninstallUI(c);
    }
    
    
    // Painting ***************************************************************

    protected void paintVerticalLine(Graphics g, JComponent c, int x, int top, int bottom) {
        if (linesEnabled) {
            drawDashedVerticalLine(g, x, top, bottom);
        }
    }

    protected void paintHorizontalLine(Graphics g, JComponent c, int y, int left, int right) {
        if (linesEnabled) {
            drawDashedHorizontalLine(g, y, left, right);
        }
    }

    // Draws the icon centered at (x,y)
    protected void drawCentered(Component c, Graphics graphics, Icon icon, int x, int y) {
        icon.paintIcon(
            c,
            graphics,
            x - icon.getIconWidth()  / 2 - 1,
            y - icon.getIconHeight() / 2);
    }

    // Helper Code ************************************************************

    private void updateLineStyle(Object lineStyle) {
        linesEnabled = !Options.TREE_LINE_STYLE_NONE_VALUE.equals(lineStyle);
    }
    
    // Listens for changes of the line style property 
    private class LineStyleHandler implements PropertyChangeListener {
        public void propertyChange(PropertyChangeEvent e) {
            String name  = e.getPropertyName();
            Object value = e.getNewValue();
            if (name.equals(Options.TREE_LINE_STYLE_KEY)) {
                updateLineStyle(value);
            }
        }
    }

    //
    // Use suggested work-around for Java bug #4833524
    //
    protected boolean isToggleSelectionEvent(java.awt.event.MouseEvent event)
    {
	return (javax.swing.SwingUtilities.isLeftMouseButton(event) && 
		event.isControlDown() && !event.isPopupTrigger());
    }
    
}
