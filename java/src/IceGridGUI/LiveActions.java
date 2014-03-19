// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI;

import java.awt.event.ActionEvent;
import javax.swing.AbstractAction;
import javax.swing.Action;

import IceGrid.*;
import IceGridGUI.LiveDeployment.*;

//
// Holds all actions for the "Live Deployment" view
//
public class LiveActions
{
    class SendSignal extends AbstractAction
    {
        SendSignal(String name)
        {
            super(name);
            _name = name;
            putValue(Action.SHORT_DESCRIPTION, "Send " + _name);
        }

        public void actionPerformed(ActionEvent e)
        {
            _target.signal(_name);
        }
        private String _name;
    }

    public Action get(int index)
    {
        return _array[index];
    }

    public boolean[] setTarget(TreeNode target)
    {
        _target = target;

        boolean[] availableActions;
        if(_target == null)
        {
            availableActions = new boolean[TreeNode.ACTION_COUNT];
        }
        else
        {
            availableActions = _target.getAvailableActions();
        }

        for(int i = 0; i < _array.length; ++i)
        {
            _array[i].setEnabled(availableActions[i]);
        }
        return availableActions;
    }

    LiveActions()
    {
        _array[TreeNode.START] = new AbstractAction("Start", Utils.getIcon("/icons/16x16/start.png"))
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.start();
                }
            };
        _array[TreeNode.START].putValue(Action.SHORT_DESCRIPTION, "Start this server or service");

        _array[TreeNode.STOP] = new AbstractAction("Stop", Utils.getIcon("/icons/16x16/stop.png"))
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.stop();
                }
            };
        _array[TreeNode.STOP].putValue(Action.SHORT_DESCRIPTION, "Stop this server or service");


        _array[TreeNode.ENABLE] = new AbstractAction("Enable", Utils.getIcon("/icons/16x16/enable.png"))
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.enable();
                }
            };
        _array[TreeNode.ENABLE].putValue(Action.SHORT_DESCRIPTION, "Enable this server");

        _array[TreeNode.DISABLE] = new AbstractAction("Disable", Utils.getIcon("/icons/16x16/disable.png"))
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.disable();
                }
            };
        _array[TreeNode.DISABLE].putValue(Action.SHORT_DESCRIPTION, "Disable this server");

        _array[TreeNode.SIGHUP] = new SendSignal("SIGHUP");
        _array[TreeNode.SIGINT] = new SendSignal("SIGINT");
        _array[TreeNode.SIGQUIT] = new SendSignal("SIGQUIT");
        _array[TreeNode.SIGKILL] = new SendSignal("SIGKILL");
        _array[TreeNode.SIGUSR1] = new SendSignal("SIGUSR1");
        _array[TreeNode.SIGUSR2] = new SendSignal("SIGUSR2");
        _array[TreeNode.SIGTERM] = new SendSignal("SIGTERM");

        _array[TreeNode.WRITE_MESSAGE] = new AbstractAction("Write Message")
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.writeMessage();
                }
            };
        _array[TreeNode.WRITE_MESSAGE].putValue(Action.SHORT_DESCRIPTION, "Write message to stdout or stderr");


        _array[TreeNode.RETRIEVE_STDOUT] = new AbstractAction("Retrieve stdout")
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.retrieveOutput(true);
                }
            };
        _array[TreeNode.RETRIEVE_STDOUT].putValue(Action.SHORT_DESCRIPTION, "Retrieve stdout");

        _array[TreeNode.RETRIEVE_STDERR] = new AbstractAction("Retrieve stderr")
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.retrieveOutput(false);
                }
            };
        _array[TreeNode.RETRIEVE_STDERR].putValue(Action.SHORT_DESCRIPTION, "Retrieve stderr");

        _array[TreeNode.RETRIEVE_LOG] = new AbstractAction("Retrieve Log")
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.retrieveLog();
                }
            };
        _array[TreeNode.RETRIEVE_LOG].putValue(Action.SHORT_DESCRIPTION, "Retrieve log file from the server");

        _array[TreeNode.SHUTDOWN_NODE] = new AbstractAction("Shutdown")
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.shutdownNode();
                }
            };

        _array[TreeNode.SHUTDOWN_REGISTRY] = new AbstractAction("Shutdown")
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.shutdownRegistry();
                }
            };

        _array[TreeNode.PATCH_SERVER] = new AbstractAction("Patch Distribution")
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.patchServer();
                }
            };

        _array[TreeNode.ADD_OBJECT] = new AbstractAction("Add Well-known Object")
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.addObject();
                }
            };

        _array[TreeNode.OPEN_DEFINITION] = new AbstractAction("Open Definition")
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.openDefinition();
                }
            };

        _array[TreeNode.ENABLE_METRICS_VIEW] = new AbstractAction("Enable Metrics View")
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.enableMetricsView(true);
                }
            };

        _array[TreeNode.DISABLE_METRICS_VIEW] = new AbstractAction("Disable Metrics View")
            {
                public void actionPerformed(ActionEvent e)
                {
                    _target.enableMetricsView(false);
                }
            };
    }

    private TreeNode _target;
    private Action[] _array = new Action[TreeNode.ACTION_COUNT];
}
