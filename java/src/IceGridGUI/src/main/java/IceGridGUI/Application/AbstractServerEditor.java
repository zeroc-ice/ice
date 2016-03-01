// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.Application;

import javax.swing.JOptionPane;
import IceGrid.*;

//
// Base class for ServerEditor and ServerInstanceEditor
//

abstract class AbstractServerEditor extends Editor
{
    abstract protected void writeDescriptor();
    abstract protected boolean isSimpleUpdate();

    @Override
    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Server Properties");
    }

    @Override
    protected boolean applyUpdate(boolean refresh)
    {
        Root root = _target.getRoot();
        Server server = (Server)_target;

        root.disableSelectionListener();
        try
        {
            if(_target.isEphemeral())
            {
                Node node = (Node)_target.getParent();
                writeDescriptor();

                _target.destroy(); // just removes the child

                try
                {
                    if(server instanceof PlainServer)
                    {
                        node.tryAdd((ServerDescriptor)server.getDescriptor(), true);
                    }
                    else
                    {
                        node.tryAdd((ServerInstanceDescriptor)server.getDescriptor(), true);
                    }
                }
                catch(UpdateFailedException e)
                {
                    //
                    // Add back ephemeral child
                    //
                    try
                    {
                        node.insertServer(_target, true);
                    }
                    catch(UpdateFailedException die)
                    {
                        assert false;
                    }
                    root.setSelectedNode(_target);

                    JOptionPane.showMessageDialog(
                        root.getCoordinator().getMainFrame(),
                        e.toString(),
                        "Apply failed",
                        JOptionPane.ERROR_MESSAGE);
                    return false;
                }

                //
                // Success
                //
                _target = node.findChildWithDescriptor(server.getDescriptor());
                root.updated();

                if(refresh)
                {
                    root.setSelectedNode(_target);
                }

            }
            else if(isSimpleUpdate())
            {
                writeDescriptor();
                root.updated();
                server.getEditable().markModified();
            }
            else
            {
                //
                // Save to be able to rollback
                //
                Object savedDescriptor = server.saveDescriptor();
                Node node = (Node)_target.getParent();
                writeDescriptor();

                node.removeServer(_target);

                try
                {
                    if(server instanceof PlainServer)
                    {
                        node.tryAdd((ServerDescriptor)server.getDescriptor(), false);
                    }
                    else
                    {
                        node.tryAdd((ServerInstanceDescriptor)server.getDescriptor(), false);
                    }
                }
                catch(UpdateFailedException e)
                {
                    //
                    // Restore
                    //
                    try
                    {
                        node.insertServer(_target, true);
                    }
                    catch(UpdateFailedException die)
                    {
                        assert false;
                    }
                    server.restoreDescriptor(savedDescriptor);
                    root.setSelectedNode(_target);

                    JOptionPane.showMessageDialog(
                        root.getCoordinator().getMainFrame(),
                        e.toString(),
                        "Apply failed",
                        JOptionPane.ERROR_MESSAGE);
                    return false;
                }

                //
                // Success
                //
                node.getEditable().removeElement(_target.getId(), server.getEditable(),
                                                 Server.class); // replaced by brand new Server

                _target = node.findChildWithDescriptor(server.getDescriptor());
                root.updated();
                if(refresh)
                {
                    root.setSelectedNode(_target);
                }
            }

            if(refresh)
            {
                root.getCoordinator().getCurrentTab().showNode(_target);
            }
            _applyButton.setEnabled(false);
            _discardButton.setEnabled(false);
            return true;
        }
        finally
        {
            root.enableSelectionListener();
        }
    }
}
