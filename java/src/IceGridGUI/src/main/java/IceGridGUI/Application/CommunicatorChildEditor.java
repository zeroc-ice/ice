// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.Application;

import javax.swing.JOptionPane;

abstract class CommunicatorChildEditor extends Editor
{
    abstract void writeDescriptor();
    abstract boolean isSimpleUpdate();
    abstract Communicator.ChildList getChildList();

    void postUpdate()
    {
    }

    @Override
    @SuppressWarnings("unchecked")
    protected boolean applyUpdate(boolean refresh)
    {
        Root root = _target.getRoot();
        root.disableSelectionListener();
        try
        {
            if(_target.isEphemeral())
            {
                Communicator.ChildList childList = getChildList();

                writeDescriptor();
                Object descriptor = _target.getDescriptor();
                _target.destroy(); // just removes the child

                try
                {
                    //@SuppressWarnings("unchecked")
                    childList.tryAdd(descriptor);
                }
                catch(UpdateFailedException e)
                {
                    //
                    // Restore ephemeral
                    //
                    try
                    {
                        childList.addChild(_target, true);
                    }
                    catch(UpdateFailedException die)
                    {
                        assert false;
                    }

                    JOptionPane.showMessageDialog(
                        root.getCoordinator().getMainFrame(),
                        e.toString(),
                        "Apply failed",
                        JOptionPane.ERROR_MESSAGE);

                    root.setSelectedNode(_target);
                    return false;
                }

                //
                // Success
                //
                //@SuppressWarnings("unchecked")
                _target = childList.findChildWithDescriptor(descriptor);
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
                ((Communicator)_target.getParent()).getEnclosingEditable().markModified();
            }
            else
            {
                //
                // Save to be able to rollback
                //
                Object savedDescriptor = ((DescriptorHolder)_target).saveDescriptor();
                Communicator.ChildList childList = getChildList();
                writeDescriptor();
                try
                {
                    childList.tryUpdate(_target);
                }
                catch(UpdateFailedException e)
                {
                    ((DescriptorHolder)_target).restoreDescriptor(savedDescriptor);
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
                _target = childList.findChildWithDescriptor(_target.getDescriptor());
                root.updated();
                if(refresh)
                {
                    root.setSelectedNode(_target);
                }
            }

            postUpdate();

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
