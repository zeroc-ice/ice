// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import javax.swing.JFrame;

import com.jgoodies.forms.builder.DefaultFormBuilder;

class ServerTemplateEditor extends TemplateEditor
{
    ServerTemplateEditor(JFrame parentFrame)
    {
	super(parentFrame);
	_subEditor = new ServerSubEditor(this, parentFrame);
    }
    
    void writeDescriptor()
    {
	super.writeDescriptor();
	_subEditor.writeDescriptor();
    }	    

    boolean isSimpleUpdate()
    {
	return super.isSimpleUpdate() && _subEditor.isSimpleUpdate();
    }

    protected void appendProperties(DefaultFormBuilder builder)
    { 
	super.appendProperties(builder);
	builder.appendSeparator();
	builder.nextLine();
	_subEditor.appendProperties(builder);
    }

    protected void buildPropertiesPanel()
    {
	super.buildPropertiesPanel();
	_propertiesPanel.setName("Server Template Properties");
    }

    void show(ServerTemplate t)
    {
	detectUpdates(false);
	_target = t;
	
	super.show();
	_subEditor.show(true);

	_applyButton.setEnabled(t.isEphemeral());
	_discardButton.setEnabled(t.isEphemeral());	  
	detectUpdates(true);
    }

    private ServerSubEditor _subEditor;
}
