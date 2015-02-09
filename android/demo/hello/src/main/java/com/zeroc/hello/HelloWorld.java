// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.hello;

import Ice.LocalException;
import android.app.*;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
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
    public static class ErrorDialogFragment extends DialogFragment
    {
        public static ErrorDialogFragment newInstance(String message, boolean fatal)
        {
            ErrorDialogFragment frag = new ErrorDialogFragment();
            Bundle args = new Bundle();
            args.putString("message", message);
            args.putBoolean("fatal", fatal);
            frag.setArguments(args);
            return frag;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState)
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setTitle("Error")
                   .setMessage(getArguments().getString("message"))
                   .setPositiveButton("Ok", new DialogInterface.OnClickListener()
                    {
                        public void onClick(DialogInterface dialog, int whichButton)
                        {
                            if(getArguments().getBoolean("fatal"))
                            {
                                ((HelloWorld) getActivity()).finish();
                            }
                        }
                    });
            return builder.create();
        }
    }

    public static class InitializeDialogFragment extends DialogFragment
    {
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState)
        {
            ProgressDialog dialog = new ProgressDialog(getActivity());
            dialog.setTitle("Initializing");
            dialog.setMessage("Please wait while loading...");
            dialog.setIndeterminate(true);
            dialog.setCanceledOnTouchOutside(false);

            setCancelable(false);
            return dialog;
        }
    }

    // These two arrays match.
    private final static DeliveryMode DELIVERY_MODES[] = {
        DeliveryMode.TWOWAY,
        DeliveryMode.TWOWAY_SECURE,
        DeliveryMode.ONEWAY,
        DeliveryMode.ONEWAY_BATCH,
        DeliveryMode.ONEWAY_SECURE,
        DeliveryMode.ONEWAY_SECURE_BATCH,
        DeliveryMode.DATAGRAM,
        DeliveryMode.DATAGRAM_BATCH,
    };

    private final static String DELIVERY_MODE_DESC[] = new String[] {
            "Twoway",
            "Twoway Secure",
            "Oneway",
            "Oneway Batch",
            "Oneway Secure",
            "Oneway Secure Batch",
            "Datagram",
            "Datagram Batch"
    };

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        final Button sayHelloButton = (Button)findViewById(R.id.sayHello);
        final SeekBar delaySeekBar = (SeekBar)findViewById(R.id.delay);
        final ProgressBar activityProgressBar = (ProgressBar)findViewById(R.id.activity);
        final Button shutdownButton = (Button)findViewById(R.id.shutdown);
        final EditText hostEditText = (EditText)findViewById(R.id.host);
        final Button flushButton = (Button)findViewById(R.id.flush);
        final TextView statusTextView = (TextView)findViewById(R.id.status);
        final Spinner modeSpinner = (Spinner)findViewById(R.id.mode);
        final TextView delayTextView = (TextView)findViewById(R.id.delayView);
        final TextView timeoutTextView = (TextView)findViewById(R.id.timeoutView);
        final SeekBar timeoutSeekBar = (SeekBar)findViewById(R.id.timeout);
        final SharedPreferences prefs = getPreferences(MODE_PRIVATE);

        _app = (HelloApp)getApplication();

        sayHelloButton.setOnClickListener(new OnClickListener()
        {
            public void onClick(android.view.View v)
            {
                if(_app.getDeliveryMode().isBatch())
                {
                    flushButton.setEnabled(true);
                    _app.sayHello(delaySeekBar.getProgress());
                    statusTextView.setText("Queued hello request");

                }
                else
                {
                    _app.sayHelloAsync(delaySeekBar.getProgress());
                }
            }
        });

        shutdownButton.setOnClickListener(new OnClickListener()
        {
            public void onClick(android.view.View v)
            {
                if(_app.getDeliveryMode().isBatch())
                {
                    flushButton.setEnabled(true);
                    _app.shutdown();
                    statusTextView.setText("Queued shutdown request");
                }
                else
                {
                    _app.shutdownAsync();
                }
            }
        });


        hostEditText.addTextChangedListener(new TextWatcher()
        {
            @Override
            public void afterTextChanged(Editable s)
            {
                String host = hostEditText.getText().toString().trim();
                if(host.isEmpty())
                {
                    sayHelloButton.setEnabled(false);
                    shutdownButton.setEnabled(false);
                }
                else
                {
                    sayHelloButton.setEnabled(true);
                    shutdownButton.setEnabled(true);
                }

                _app.setHost(host);

                // Change the preferences if necessary.
                if(!prefs.getString(HOSTNAME_KEY, DEFAULT_HOST).equals(host))
                {
                    SharedPreferences.Editor edit = prefs.edit();
                    edit.putString(HOSTNAME_KEY, host);
                    edit.apply();
                }
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after)
            {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int count, int after)
            {
            }
        });

        flushButton.setOnClickListener(new OnClickListener()
        {
            public void onClick(View v)
            {
                _app.flush();
                flushButton.setEnabled(false);
                statusTextView.setText("Flushed batch requests");
            }
        });

        ArrayAdapter<String> modeAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item,
                DELIVERY_MODE_DESC);
        modeSpinner.setAdapter(modeAdapter);
        modeSpinner.setOnItemSelectedListener(new android.widget.AdapterView.OnItemSelectedListener()
        {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id)
            {
                _app.setDeliveryMode(DELIVERY_MODES[(int) id]);
            }

            @Override
            public void onNothingSelected(AdapterView<?> arg0)
            {
            }
        });
        modeSpinner.setSelection(0);
        _app.setDeliveryMode(DELIVERY_MODES[(int)modeSpinner.getSelectedItemId()]);

        delaySeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener()
        {
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromThumb)
            {
                delayTextView.setText(String.format("%.1f", progress / 1000.0));
            }

            public void onStartTrackingTouch(SeekBar seekBar)
            {
            }

            public void onStopTrackingTouch(SeekBar seekBar)
            {
            }
        });

        timeoutSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener()
        {
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromThumb)
            {
                timeoutTextView.setText(String.format("%.1f", progress / 1000.0));
                _app.setTimeout(progress);
            }

            public void onStartTrackingTouch(SeekBar seekBar)
            {
            }

            public void onStopTrackingTouch(SeekBar seekBar)
            {
            }
        });

        activityProgressBar.setVisibility(View.GONE);

        // Setup the defaults.
        hostEditText.setText(prefs.getString(HOSTNAME_KEY, DEFAULT_HOST));
        flushButton.setEnabled(false);

        statusTextView.setText("Ready");

        _handler = new Handler(Looper.getMainLooper()) {
            @Override
            public void handleMessage(Message m)
            {
                switch(m.what)
                {
                    case HelloApp.MSG_WAIT:
                    {
                        // Show the initializing dialog if it isn't already on the stack.
                        if(getFragmentManager().findFragmentByTag(INITIALIZE_TAG) == null)
                        {
                            DialogFragment dialog = new InitializeDialogFragment();
                            dialog.show(getFragmentManager(), INITIALIZE_TAG);
                        }
                        break;
                    }

                    case HelloApp.MSG_READY:
                    {
                        // Hide the initializing dialog if it is on the stack.
                        DialogFragment initDialog = (DialogFragment) getFragmentManager().findFragmentByTag(INITIALIZE_TAG);
                        if(initDialog != null)
                        {
                            initDialog.dismiss();
                        }

                        HelloApp.MessageReady ready = (HelloApp.MessageReady) m.obj;
                        if(ready.ex != null)
                        {
                            LocalException ex = (LocalException) ready.ex;
                            DialogFragment dialog = ErrorDialogFragment.newInstance(ex.toString(), true);
                            dialog.show(getFragmentManager(), ERROR_TAG);
                        }
                        statusTextView.setText("Ready");
                        break;
                    }

                    case HelloApp.MSG_EXCEPTION:
                    {
                        statusTextView.setText("Ready");
                        activityProgressBar.setVisibility(View.GONE);

                        LocalException ex = (LocalException) m.obj;
                        DialogFragment dialog = ErrorDialogFragment.newInstance(ex.toString(), false);
                        dialog.show(getFragmentManager(), ERROR_TAG);
                        break;
                    }

                    case HelloApp.MSG_RESPONSE:
                    {
                        activityProgressBar.setVisibility(View.GONE);
                        statusTextView.setText("Ready");
                        break;
                    }
                    case HelloApp.MSG_SENT:
                    {
                        DeliveryMode mode = (DeliveryMode) m.obj;
                        activityProgressBar.setVisibility(View.VISIBLE);
                        statusTextView.setText("Waiting for response");
                        break;
                    }

                    case HelloApp.MSG_SENDING:
                    {
                        activityProgressBar.setVisibility(View.VISIBLE);
                        statusTextView.setText("Sending request");
                        break;
                    }
                }
            }
        };
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        _app.setHandler(_handler);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState)
    {
        super.onRestoreInstanceState(savedInstanceState);

        final SeekBar delaySeekBar = (SeekBar)findViewById(R.id.delay);
        final Button flushButton = (Button)findViewById(R.id.flush);
        final Spinner modeSpinner = (Spinner)findViewById(R.id.mode);
        final SeekBar timeoutSeekBar = (SeekBar)findViewById(R.id.timeout);
        final TextView statusTextView = (TextView)findViewById(R.id.status);
        final ProgressBar activityProgressBar = (ProgressBar)findViewById(R.id.activity);

        modeSpinner.setSelection(savedInstanceState.getInt(BUNDLE_KEY_MODE));
        flushButton.setEnabled(savedInstanceState.getBoolean(BUNDLE_KEY_FLUSH_ENABLED));
        delaySeekBar.setProgress(savedInstanceState.getInt(BUNDLE_KEY_DELAY));
        timeoutSeekBar.setProgress(savedInstanceState.getInt(BUNDLE_KEY_TIMEOUT));
        statusTextView.setText(savedInstanceState.getString(BUNDLE_KEY_STATUS));
        activityProgressBar.setVisibility(
                savedInstanceState.getBoolean(BUNDLE_KEY_PROGRESS) ? View.VISIBLE : View.GONE);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState)
    {
        super.onSaveInstanceState(outState);

        final SeekBar delaySeekBar = (SeekBar)findViewById(R.id.delay);
        final Button flushButton = (Button)findViewById(R.id.flush);
        final Spinner modeSpinner = (Spinner)findViewById(R.id.mode);
        final SeekBar timeoutSeekBar = (SeekBar)findViewById(R.id.timeout);
        final TextView statusTextView = (TextView)findViewById(R.id.status);
        final ProgressBar activityProgressBar = (ProgressBar)findViewById(R.id.activity);

        outState.putInt(BUNDLE_KEY_MODE, (int)modeSpinner.getSelectedItemId());
        outState.putInt(BUNDLE_KEY_DELAY, delaySeekBar.getProgress());
        outState.putInt(BUNDLE_KEY_TIMEOUT, timeoutSeekBar.getProgress());
        outState.putBoolean(BUNDLE_KEY_FLUSH_ENABLED, flushButton.isEnabled());
        outState.putString(BUNDLE_KEY_STATUS, statusTextView.getText().toString());
        outState.putBoolean(BUNDLE_KEY_PROGRESS, activityProgressBar.getVisibility() == View.VISIBLE);

        // Clear the application handler. We don't want any further messages while
        // in the background.
        _app.setHandler(null);
    }

    @Override
    protected void onDestroy()
    {
        super.onDestroy();
        _app.setHandler(null);
    }

    public static final String INITIALIZE_TAG = "initialize";
    public static final String ERROR_TAG = "error";

    private static final String DEFAULT_HOST = "";
    private static final String HOSTNAME_KEY = "host";

    private static final String BUNDLE_KEY_PROGRESS = "zeroc:progress";
    private static final String BUNDLE_KEY_STATUS = "zeroc:status";
    private static final String BUNDLE_KEY_MODE = "zeroc:mode";
    private static final String BUNDLE_KEY_TIMEOUT = "zeroc:timeout";
    private static final String BUNDLE_KEY_DELAY = "zeroc:delay";
    private static final String BUNDLE_KEY_FLUSH_ENABLED = "zeroc:flush";

    private HelloApp _app;
    private Handler _handler;
}
