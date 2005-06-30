// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

import java.util.prefs.Preferences;
import java.util.prefs.BackingStoreException;
import javax.swing.*;
import javax.swing.tree.TreeCellRenderer;
import java.awt.*;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.ActionEvent;

import com.jgoodies.looks.Options;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;
import com.jgoodies.looks.windows.WindowsLookAndFeel;
import com.jgoodies.forms.factories.Borders;
import javax.swing.border.EmptyBorder;

import javax.swing.plaf.SplitPaneUI;
import javax.swing.plaf.basic.BasicSplitPaneUI;

import javax.swing.border.AbstractBorder;

public class MainPane extends JSplitPane
{
    public void updateUI()
    {
	super.updateUI();
	setEmptyDividerBorder();
    }
    
    private void setEmptyDividerBorder()
    {
	SplitPaneUI splitPaneUI = getUI();
	if(splitPaneUI instanceof BasicSplitPaneUI) 
	{
	    BasicSplitPaneUI basicUI = (BasicSplitPaneUI) splitPaneUI;
	    basicUI.getDivider().setBorder(BorderFactory.createEmptyBorder());
	}
    }
  

    MainPane(Model model)
    {
	super(JSplitPane.HORIZONTAL_SPLIT, true);
	_model = model;
	setBorder(new EmptyBorder(10, 10, 10, 10));

	//
	// Left pane
	//
	JTabbedPane tabbedPane = new JTabbedPane();
	tabbedPane.setMinimumSize(new Dimension(200, 300));
	tabbedPane.putClientProperty(Options.NO_CONTENT_BORDER_KEY, Boolean.TRUE);
	tabbedPane.setBorder(new ShadowBorder());

	TreeCellRenderer renderer = new CellRenderer();

	JTree nodeTree = new JTree(_model.getTreeModel(TreeModelI.NODE_VIEW));
	nodeTree.setCellRenderer(renderer);
        ToolTipManager.sharedInstance().registerComponent(nodeTree);

	JScrollPane nodeScroll = 
	    new JScrollPane(nodeTree, 
			    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
			    JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
	nodeScroll.setBorder(Borders.DIALOG_BORDER);
	

	tabbedPane.addTab("Node View", nodeScroll);
	
	JTree appTree = new JTree(_model.getTreeModel(TreeModelI.APPLICATION_VIEW));
	appTree.setCellRenderer(renderer);
	ToolTipManager.sharedInstance().registerComponent(appTree);
	JScrollPane appScroll = 
	    new JScrollPane(appTree,
			    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
			    JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
	appScroll.setBorder(Borders.DIALOG_BORDER);
	
	tabbedPane.addTab("Application View", appScroll);
	
	
	JPanel leftPane = new JPanel(new BorderLayout());
	leftPane.add(tabbedPane);

	//
	// Right pane
	//
	JPanel rightPane = new com.jgoodies.uif_lite.panel.SimpleInternalFrame("Properties");
	
	/* JScrollPane rightPane = new JScrollPane(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
						JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
	*/
	setLeftComponent(leftPane);
	setRightComponent(rightPane);
    }

    //
    // Adapted from JGoodies SimpleInternalFrame
    //
    private static class ShadowBorder extends AbstractBorder 
    {
        private static final Insets INSETS = new Insets(1, 1, 3, 3);
        public Insets getBorderInsets(Component c) { return INSETS; }

        public void paintBorder(Component c, Graphics g,
            int x, int y, int w, int h) 
	{
            Color shadow        = UIManager.getColor("controlShadow");
            if (shadow == null) 
	    {
                shadow = Color.GRAY;
            }
            Color lightShadow   = new Color(shadow.getRed(),
                                            shadow.getGreen(),
                                            shadow.getBlue(),
                                            170);
            Color lighterShadow = new Color(shadow.getRed(),
                                            shadow.getGreen(),
                                            shadow.getBlue(),
                                            70);
            g.translate(x, y);
         
            g.setColor(shadow);

	    int tabAdjustment = 24;

            // g.fillRect(0, 0, w - 3, 1);
            g.fillRect(0, 0 + tabAdjustment, 1, h - 3 - tabAdjustment);
	    g.fillRect(w - 3, 1 + tabAdjustment , 1, h - 3 - tabAdjustment);
            g.fillRect(1, h - 3, w - 3, 1);
            // Shadow line 1
            g.setColor(lightShadow);
            g.fillRect(w - 3, 0 + tabAdjustment, 1, 1);
            g.fillRect(0, h - 3, 1, 1);
            g.fillRect(w - 2, 1 + tabAdjustment, 1, h - 3 - tabAdjustment);
            g.fillRect(1, h - 2, w - 3, 1);
            // Shadow line2
            g.setColor(lighterShadow);
            g.fillRect(w - 2, 0 + tabAdjustment, 1, 1);
            g.fillRect(0, h - 2, 1, 1);
            g.fillRect(w-2, h - 2, 1, 1);
            g.fillRect(w - 1, 1 + tabAdjustment, 1, h - 2 - tabAdjustment);
            g.fillRect(1, h - 1, w - 2, 1);
            g.translate(-x, -y);
        }
    }

    private Model _model;
}
