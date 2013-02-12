// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.hello;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.SeekBar.OnSeekBarChangeListener;

public class HelloWorld extends Activity
{
    enum DeliveryMode
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
        assert (host.length() > 0);
        // Change the preferences if necessary.
        if(!_prefs.getString(HOSTNAME_KEY, DEFAULT_HOST).equals(host))
        {
            SharedPreferences.Editor edit = _prefs.edit();
            edit.putString(HOSTNAME_KEY, host);
            edit.commit();
        }

        String s = "hello:tcp -h " + host + " -p 10000:ssl -h " + host + " -p 10001:udp -h " + host + " -p 10000";
        Ice.ObjectPrx prx = _communicator.stringToProxy(s);
        prx = _deliveryMode.apply(prx);
        int timeout = _timeout.getProgress();
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
        synchronized public void exception(final Ice.LocalException ex)
        {
            assert (!_response);
            _response = true;

            runOnUiThread(new Runnable()
            {
                public void run()
                {
                    handleException(ex);
                }
            });
        }

        @Override
        synchronized public void sent(boolean sentSynchronously)
        {
            if(_response)
            {
                return;
            }
            runOnUiThread(new Runnable()
            {
                public void run()
                {
                    if(isFinishing())
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
                        _activity.setVisibility(View.INVISIBLE);
                    }
                }
            });
        }

        @Override
        synchronized public void response()
        {
            assert (!_response);
            _response = true;
            runOnUiThread(new Runnable()
            {
                public void run()
                {
                    if(isFinishing())
                    {
                        return;
                    }

                    _activity.setVisibility(View.INVISIBLE);
                    _status.setText("Ready");
                }
            });
        }
    }

    private void sayHello()
    {
        Demo.HelloPrx hello = createProxy();
        try
        {
            if(!_deliveryMode.isBatch())
            {
                Ice.AsyncResult r = hello.begin_sayHello(_delay.getProgress(), new SayHelloI());
                if(r.sentSynchronously())
                {
                    if(_deliveryMode == DeliveryMode.TWOWAY || _deliveryMode == DeliveryMode.TWOWAY_SECURE)
                    {
                        _activity.setVisibility(View.VISIBLE);
                        _status.setText("Waiting for response");
                    }
                }
                else
                {
                    _activity.setVisibility(View.VISIBLE);
                    _status.setText("Sending request");
                }
            }
            else
            {
                _flushButton.setEnabled(true);
                hello.sayHello(_delay.getProgress());
                _status.setText("Queued hello request");
            }
        }
        catch(Ice.LocalException ex)
        {
            handleException(ex);
        }
    }

    private void handleException(Ice.LocalException ex)
    {
        if(isFinishing())
        {
            return;
        }

        _status.setText("Ready");
        _activity.setVisibility(View.INVISIBLE);

        _lastError = ex.toString();
        showDialog(DIALOG_ERROR);
        ex.printStackTrace();
    }

    private void shutdown()
    {
        Demo.HelloPrx hello = createProxy();
        try
        {
            if(!_deliveryMode.isBatch())
            {
                hello.begin_shutdown(new Demo.Callback_Hello_shutdown()
                {
                    @Override
                    public void exception(final Ice.LocalException ex)
                    {
                        runOnUiThread(new Runnable()
                        {
                            public void run()
                            {
                                handleException(ex);
                            }
                        });
                    }

                    @Override
                    public void response()
                    {
                        runOnUiThread(new Runnable()
                        {
                            public void run()
                            {
                                if(isFinishing())
                                {
                                    return;
                                }

                                _activity.setVisibility(View.INVISIBLE);
                                _status.setText("Ready");
                            }
                        });
                    }
                });
                if(_deliveryMode == DeliveryMode.TWOWAY || _deliveryMode == DeliveryMode.TWOWAY_SECURE)
                {
                    _activity.setVisibility(View.VISIBLE);
                    _status.setText("Waiting for response");
                }
            }
            else
            {
                _flushButton.setEnabled(true);
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
        new Thread(new Runnable()
        {
            public void run()
            {
                try
                {
                    _communicator.flushBatchRequests();
                }
                catch(final Ice.LocalException ex)
                {
                    runOnUiThread(new Runnable()
                    {
                        public void run()
                        {
                            handleException(ex);
                        }
                    });
                }
            }
        }).start();

        _flushButton.setEnabled(false);
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

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        _sayHelloButton = (Button)findViewById(R.id.sayHello);
        _sayHelloButton.setOnClickListener(new OnClickListener()
        {
            public void onClick(android.view.View v)
            {
                sayHello();
            }
        });
        _shutdownButton = (Button)findViewById(R.id.shutdown);
        _shutdownButton.setOnClickListener(new OnClickListener()
        {
            public void onClick(android.view.View v)
            {
                shutdown();
            }
        });

        _hostname = (EditText)findViewById(R.id.hostname);
        _hostname.addTextChangedListener(new TextWatcher()
        {
            public void afterTextChanged(Editable s)
            {
                String host = _hostname.getText().toString().trim();
                if(host.length() == 0)
                {
                    _sayHelloButton.setEnabled(false);
                    _shutdownButton.setEnabled(false);
                }
                else
                {
                    _sayHelloButton.setEnabled(true);
                    _shutdownButton.setEnabled(true);
                }
            }

            public void beforeTextChanged(CharSequence s, int start, int count, int after)
            {
            }

            public void onTextChanged(CharSequence s, int start, int count, int after)
            {
            }
        });

        _flushButton = (Button)findViewById(R.id.flush);
        _flushButton.setOnClickListener(new OnClickListener()
        {
            public void onClick(View v)
            {
                flush();
            }
        });

        Spinner mode = (Spinner)findViewById(R.id.mode);
        ArrayAdapter<String> modeAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item,
                new String[] { "Twoway", "Twoway Secure", "Oneway", "Oneway Batch", "Oneway Secure",
                        "Oneway Secure Batch", "Datagram", "Datagram Batch" });
        mode.setAdapter(modeAdapter);
        mode.setOnItemSelectedListener(new android.widget.AdapterView.OnItemSelectedListener()
        {
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id)
            {
                changeDeliveryMode(id);
            }


            public void onNothingSelected(AdapterView<?> arg0)
            {
            }
        });
        if(savedInstanceState == null)
        {
            mode.setSelection(0);
        }
        changeDeliveryMode(mode.getSelectedItemId());

        final TextView delayView = (TextView)findViewById(R.id.delayView);
        _delay = (SeekBar)findViewById(R.id.delay);
        _delay.setOnSeekBarChangeListener(new OnSeekBarChangeListener()
        {
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromThumb)
            {
                delayView.setText(String.format("%.1f", progress / 1000.0));
            }

            public void onStartTrackingTouch(SeekBar seekBar)
            {
            }

            public void onStopTrackingTouch(SeekBar seekBar)
            {
            }
        });
        // BUGFIX: Android doesn't save/restore SeekBar state.
        if(savedInstanceState != null)
        {
            _delay.setProgress(savedInstanceState.getInt(BUNDLE_KEY_DELAY));
        }

        final TextView timeoutView = (TextView)findViewById(R.id.timeoutView);
        _timeout = (SeekBar)findViewById(R.id.timeout);
        _timeout.setOnSeekBarChangeListener(new OnSeekBarChangeListener()
        {
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromThumb)
            {
                timeoutView.setText(String.format("%.1f", progress / 1000.0));
            }

            public void onStartTrackingTouch(SeekBar seekBar)
            {
            }

            public void onStopTrackingTouch(SeekBar seekBar)
            {
            }
        });
        // BUGFIX: Android doesn't save/restore SeekBar state.
        if(savedInstanceState != null)
        {
            _timeout.setProgress(savedInstanceState.getInt(BUNDLE_KEY_TIMEOUT));
        }

        _activity = (ProgressBar)findViewById(R.id.activity);
        _activity.setVisibility(View.INVISIBLE);

        _status = (TextView)findViewById(R.id.status);

        // Setup the defaults.
        _prefs = getPreferences(MODE_PRIVATE);
        if(savedInstanceState == null)
        {
            _hostname.setText(_prefs.getString(HOSTNAME_KEY, DEFAULT_HOST));
            _flushButton.setEnabled(false);
        }
        else
        {
            _flushButton.setEnabled(savedInstanceState.getBoolean(BUNDLE_KEY_FLUSH_ENABLED));
            _lastError = savedInstanceState.getString(BUNDLE_KEY_LAST_ERROR);
        }

        HelloApp app = (HelloApp)getApplication();
        app.setCommunicatorCallback(new HelloApp.CommunicatorCallback()
        {
            private boolean dismiss = false;

            public void onCreate(final Ice.Communicator communicator)
            {
                runOnUiThread(new Runnable()
                {
                    public void run()
                    {
                        if(dismiss)
                        {
                            dismissDialog(DIALOG_INITIALIZING);
                        }
                        _status.setText("Ready");
                        _communicator = communicator;
                    }
                });
            }

            public void onError(final Ice.LocalException ex)
            {
                runOnUiThread(new Runnable()
                {
                    public void run()
                    {
                        if(dismiss)
                        {
                            dismissDialog(DIALOG_INITIALIZING);
                        }
                        _lastError = ex.toString();
                        showDialog(DIALOG_FATAL);

                    }
                });
            }

            public void onWait()
            {
                // Show the initializing dialog.
                dismiss = true;
                showDialog(DIALOG_INITIALIZING);
            }
        });
    }

    @Override
    protected void onSaveInstanceState(Bundle outState)
    {
        super.onSaveInstanceState(outState);
        // BUGFIX: The SeekBar doesn't save/restore state automatically.
        outState.putInt(BUNDLE_KEY_DELAY, _delay.getProgress());
        outState.putInt(BUNDLE_KEY_TIMEOUT, _timeout.getProgress());
        outState.putBoolean(BUNDLE_KEY_FLUSH_ENABLED, _flushButton.isEnabled());
        outState.putString(BUNDLE_KEY_LAST_ERROR, _lastError);
    }

    @Override
    protected Dialog onCreateDialog(int id)
    {
        switch (id)
        {
        case DIALOG_INITIALIZING:
        {
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setTitle("Initializing");
            dialog.setMessage("Please wait while loading...");
            dialog.setIndeterminate(true);
            dialog.setCancelable(false);
            return dialog;
        }

        case DIALOG_ERROR:
        case DIALOG_FATAL:
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Error");
            builder.setMessage(_lastError);
            if(id == DIALOG_FATAL)
            {
                builder.setPositiveButton("Ok", new DialogInterface.OnClickListener()
                {
                    public void onClick(DialogInterface dialog, int whichButton)
                    {
                        finish();
                    }
                });
            }
            return builder.create();
        }

        }

        return null;
    }

    private static final int DIALOG_INITIALIZING = 1;
    private static final int DIALOG_ERROR = 2;
    private static final int DIALOG_FATAL = 3;

    private static final String DEFAULT_HOST = "10.0.2.2";
    private static final String HOSTNAME_KEY = "host";

    private static final String BUNDLE_KEY_TIMEOUT = "zeroc:timeout";
    private static final String BUNDLE_KEY_DELAY = "zeroc:delay";
    private static final String BUNDLE_KEY_FLUSH_ENABLED = "zeroc:flush";
    private static final String BUNDLE_KEY_LAST_ERROR = "zeroc:lastError";

    private Ice.Communicator _communicator = null;
    private DeliveryMode _deliveryMode;

    private Button _sayHelloButton;
    private Button _shutdownButton;
    private EditText _hostname;
    private TextView _status;
    private SeekBar _delay;
    private SeekBar _timeout;
    private ProgressBar _activity;
    private SharedPreferences _prefs;
    private Button _flushButton;

    private String _lastError = "";
}
