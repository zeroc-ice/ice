// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

public class HelloApplet extends JApplet
{
    public void init()
    {
        //
        // Make sure we create the GUI from the Swing event dispatch thread.
        //
        try
        {
            SwingUtilities.invokeAndWait(new Runnable()
            {
                public void run()
                {
                    initUI();
                }
            });
        }
        catch(Throwable ex)
        {
            ex.printStackTrace();
            return;
        }

        //
        // Initialize an Ice communicator.
        //
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.load("config.applet");
            initData.dispatcher = new Ice.Dispatcher()
            {
                public void
                dispatch(Runnable runnable, Ice.Connection connection)
                {
                    SwingUtilities.invokeLater(runnable);
                }
            };
            _communicator = Ice.Util.initialize(initData);
        }
        catch(Throwable ex)
        {
            handleException(ex);
        }
    }

    public void start()
    {
        // Nothing to do.
    }

    public void stop()
    {
        // Nothing to do.
    }

    public void destroy()
    {
        //
        // Destroy the Ice run time.
        //
        if(_communicator != null)
        {
            try
            {
                _communicator.destroy();
            }
            catch(Throwable ex)
            {
                ex.printStackTrace();
            }
            _communicator = null;
        }
    }

    private void initUI()
    {
        Container cp = getContentPane();

        JLabel l1 = new JLabel("Hostname");
        _hostname = new JTextField();
        JLabel l2 = new JLabel("Mode");
        _mode = new JComboBox();
        JLabel l3 = new JLabel("Timeout");
        _timeoutSlider = new JSlider(0, MAX_TIME);
        _timeoutLabel = new JLabel("0.0");
        JLabel l4 = new JLabel("Delay");
        _delaySlider = new JSlider(0, MAX_TIME);
        _delayLabel = new JLabel("0.0");
        JPanel buttonPanel = new JPanel();
        _hello = new JButton("Hello World!");
        _shutdown = new JButton("Shutdown");
        _flush = new JButton("Flush");
        _flush.setEnabled(false);
        JPanel statusPanel = new JPanel();
        JSeparator statusPanelSeparator = new JSeparator();
        _status = new JLabel();
        _status.setText("Ready");

        //
        // Default to the host from which the applet was downloaded.
        //
        _hostname.setText(getCodeBase().getHost());

        final String[] modes = new String[]
        {
            "Twoway", "Twoway Secure", "Oneway", "Oneway Batch", "Oneway Secure", "Oneway Secure Batch", "Datagram",
            "Datagram Batch"
        };
        _mode.setModel(new DefaultComboBoxModel(modes));

        _hello.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                sayHello();
            }
        });
        _shutdown.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                shutdown();
            }
        });
        _flush.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                flush();
            }
        });
        _mode.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent e)
            {
                changeDeliveryMode(_mode.getSelectedIndex());
            }
        });
        changeDeliveryMode(_mode.getSelectedIndex());

        _timeoutSlider.addChangeListener(new SliderListener(_timeoutSlider, _timeoutLabel));
        _timeoutSlider.setValue(0);
        _delaySlider.addChangeListener(new SliderListener(_delaySlider, _delayLabel));
        _delaySlider.setValue(0);

        GridBagConstraints gridBagConstraints;

        cp.setMaximumSize(null);
        cp.setPreferredSize(null);
        cp.setLayout(new GridBagLayout());

        l1.setText("Hostname");
        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = GridBagConstraints.WEST;
        gridBagConstraints.insets = new Insets(5, 5, 5, 5);
        cp.add(l1, gridBagConstraints);

        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new Insets(5, 0, 5, 5);
        cp.add(_hostname, gridBagConstraints);

        l2.setText("Mode");
        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.anchor = GridBagConstraints.WEST;
        gridBagConstraints.insets = new Insets(0, 5, 5, 0);
        cp.add(l2, gridBagConstraints);

        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new Insets(0, 0, 5, 5);
        cp.add(_mode, gridBagConstraints);

        l3.setText("Timeout");
        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.anchor = GridBagConstraints.WEST;
        gridBagConstraints.insets = new Insets(0, 5, 5, 0);
        cp.add(l3, gridBagConstraints);

        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.fill = GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = GridBagConstraints.WEST;
        cp.add(_timeoutSlider, gridBagConstraints);

        _timeoutLabel.setMinimumSize(new Dimension(20, 17));
        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.anchor = GridBagConstraints.WEST;
        gridBagConstraints.insets = new Insets(0, 5, 5, 5);
        cp.add(_timeoutLabel, gridBagConstraints);

        l4.setText("Delay");
        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.anchor = GridBagConstraints.WEST;
        gridBagConstraints.insets = new Insets(0, 5, 5, 0);
        cp.add(l4, gridBagConstraints);

        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.fill = GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = GridBagConstraints.WEST;
        cp.add(_delaySlider, gridBagConstraints);

        _delayLabel.setMinimumSize(new Dimension(20, 17));
        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.anchor = GridBagConstraints.WEST;
        gridBagConstraints.insets = new Insets(0, 5, 5, 5);
        cp.add(_delayLabel, gridBagConstraints);

        _hello.setText("Hello World!");
        buttonPanel.add(_hello);

        _shutdown.setText("Shutdown");
        buttonPanel.add(_shutdown);

        _flush.setText("Flush");
        buttonPanel.add(_flush);

        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 4;
        gridBagConstraints.gridwidth = 3;
        gridBagConstraints.ipady = 5;
        cp.add(buttonPanel, gridBagConstraints);

        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 5;
        gridBagConstraints.gridwidth = 3;
        gridBagConstraints.fill = GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new Insets(0, 5, 5, 5);
        cp.add(statusPanelSeparator, gridBagConstraints);

        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 6;
        gridBagConstraints.gridwidth = 3;
        gridBagConstraints.fill = GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new Insets(0, 5, 5, 5);
        cp.add(_status, gridBagConstraints);
    }

    private enum DeliveryMode
    {
        TWOWAY,
        TWOWAY_SECURE,
        ONEWAY,
        ONEWAY_BATCH,
        ONEWAY_SECURE,
        ONEWAY_SECURE_BATCH,
        DATAGRAM,
        DATAGRAM_BATCH;

        Ice.ObjectPrx apply(Ice.ObjectPrx prx)
        {
            switch (this)
            {
            case TWOWAY:
                prx = prx.ice_twoway();
                break;
            case TWOWAY_SECURE:
                prx = prx.ice_twoway().ice_secure(true);
                break;
            case ONEWAY:
                prx = prx.ice_oneway();
                break;
            case ONEWAY_BATCH:
                prx = prx.ice_batchOneway();
                break;
            case ONEWAY_SECURE:
                prx = prx.ice_oneway().ice_secure(true);
                break;
            case ONEWAY_SECURE_BATCH:
                prx = prx.ice_batchOneway().ice_secure(true);
                break;
            case DATAGRAM:
                prx = prx.ice_datagram();
                break;
            case DATAGRAM_BATCH:
                prx = prx.ice_batchDatagram();
                break;
            }
            return prx;
        }

        public boolean isBatch()
        {
            return this == ONEWAY_BATCH || this == DATAGRAM_BATCH || this == ONEWAY_SECURE_BATCH;
        }
    }

    private Demo.HelloPrx createProxy()
    {
        String host = _hostname.getText().toString().trim();
        if(host.length() == 0)
        {
            _status.setText("No hostname");
            return null;
        }

        String s = "hello:tcp -h " + host + " -p 10000:ssl -h " + host + " -p 10001:udp -h " + host + " -p 10000";
        Ice.ObjectPrx prx = _communicator.stringToProxy(s);
        prx = _deliveryMode.apply(prx);
        int timeout = _timeoutSlider.getValue();
        if(timeout != 0)
        {
            prx = prx.ice_timeout(timeout);
        }
        return Demo.HelloPrxHelper.uncheckedCast(prx);
    }

    class SayHelloI extends Demo.Callback_Hello_sayHello
    {
        private boolean _response = false;

        @Override
        synchronized public void response()
        {
            assert (!_response);
            _response = true;
            _status.setText("Ready");
        }

        @Override
        synchronized public void exception(final Ice.LocalException ex)
        {
            assert (!_response);
            _response = true;
            handleException(ex);
        }

        @Override
        synchronized public void sent(boolean ss)
        {
            if(_response)
            {
                return;
            }

            if(_deliveryMode == DeliveryMode.TWOWAY || _deliveryMode == DeliveryMode.TWOWAY_SECURE)
            {
                _status.setText("Waiting for response");
            }
            else
            {
                _status.setText("Ready");
            }
        }
    }

    private void sayHello()
    {
        Demo.HelloPrx hello = createProxy();
        if(hello == null)
        {
            return;
        }

        int delay = _delaySlider.getValue();
        try
        {
            if(!_deliveryMode.isBatch())
            {
                Ice.AsyncResult r = hello.begin_sayHello(delay, new SayHelloI());
                if(!r.sentSynchronously())
                {
                    _status.setText("Sending request");
                }
            }
            else
            {
                _flush.setEnabled(true);
                hello.sayHello(delay);
                _status.setText("Queued sayHello request");
            }
        }
        catch(Ice.LocalException ex)
        {
            handleException(ex);
        }
    }

    private void shutdown()
    {
        Demo.HelloPrx hello = createProxy();
        if(hello == null)
        {
            return;
        }

        try
        {
            if(!_deliveryMode.isBatch())
            {
                hello.begin_shutdown(new Demo.Callback_Hello_shutdown()
                {
                    @Override
                    public void response()
                    {
                        _status.setText("Ready");
                    }

                    @Override
                    public void exception(final Ice.LocalException ex)
                    {
                        handleException(ex);
                    }
                });
                if(_deliveryMode == DeliveryMode.TWOWAY || _deliveryMode == DeliveryMode.TWOWAY_SECURE)
                {
                    _status.setText("Waiting for response");
                }
            }
            else
            {
                _flush.setEnabled(true);
                hello.shutdown();
                _status.setText("Queued shutdown request");
            }
        }
        catch(Ice.LocalException ex)
        {
            handleException(ex);
        }
    }

    private void flush()
    {
        _communicator.begin_flushBatchRequests(new Ice.Callback_Communicator_flushBatchRequests()
            {
                public void exception(final Ice.LocalException ex)
                {
                    handleException(ex);
                }
            });

        _flush.setEnabled(false);
        _status.setText("Flushed batch requests");
    }

    private void changeDeliveryMode(long id)
    {
        switch ((int)id)
        {
        case 0:
            _deliveryMode = DeliveryMode.TWOWAY;
            break;
        case 1:
            _deliveryMode = DeliveryMode.TWOWAY_SECURE;
            break;
        case 2:
            _deliveryMode = DeliveryMode.ONEWAY;
            break;
        case 3:
            _deliveryMode = DeliveryMode.ONEWAY_BATCH;
            break;
        case 4:
            _deliveryMode = DeliveryMode.ONEWAY_SECURE;
            break;
        case 5:
            _deliveryMode = DeliveryMode.ONEWAY_SECURE_BATCH;
            break;
        case 6:
            _deliveryMode = DeliveryMode.DATAGRAM;
            break;
        case 7:
            _deliveryMode = DeliveryMode.DATAGRAM_BATCH;
            break;
        }
    }

    private void handleException(final Throwable ex)
    {
        ex.printStackTrace();
        _status.setText(ex.getClass().getName());
    }

    private static class SliderListener implements ChangeListener
    {
        SliderListener(JSlider slider, JLabel label)
        {
            _slider = slider;
            _label = label;
        }

        public void stateChanged(ChangeEvent ce)
        {
            float value = (float)(_slider.getValue() / 1000.0);
            _label.setText(String.format("%.1f", value));
        }

        private JSlider _slider;
        private JLabel _label;
    }

    private static final int MAX_TIME = 5000; // 5 seconds

    private JTextField _hostname;
    private JComboBox _mode;
    private JSlider _timeoutSlider;
    private JLabel _timeoutLabel;
    private JSlider _delaySlider;
    private JLabel _delayLabel;
    private JButton _hello;
    private JButton _shutdown;
    private JButton _flush;
    private JLabel _status;

    private Ice.Communicator _communicator;
    private DeliveryMode _deliveryMode;
}
