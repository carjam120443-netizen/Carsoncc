package com.carsoncc.mobile;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import androidx.activity.ComponentActivity;
import androidx.core.content.ContextCompat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class MainActivity extends ComponentActivity {
    private static final String ACTION_LOG = "com.carsoncc.mobile.LOG";
    private TextView status;
    private TextView console;
    private ScrollView consoleScroll;
    private final SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm:ss", Locale.getDefault());

    private final BroadcastReceiver logReceiver = new BroadcastReceiver() {
        @Override public void onReceive(Context context, Intent intent) {
            String message = intent.getStringExtra("message");
            if (message != null) appendLog(message);
        }
    };

    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        ScrollView page = new ScrollView(this);
        page.setFillViewport(true);
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(40, 48, 40, 40);
        page.addView(root);

        TextView title = new TextView(this);
        title.setText("CarsonCC Mobile");
        title.setTextSize(28);
        root.addView(title);

        TextView info = new TextView(this);
        info.setText("Low-power background bridge for CarsonCC terminal integrations.\n\nThe service only listens on 127.0.0.1:3939 and does not hold a wake lock or continuously poll the device.");
        info.setTextSize(16);
        root.addView(info);

        status = new TextView(this);
        status.setText("\nStatus: stopped");
        status.setTextSize(18);
        root.addView(status);

        Button start = new Button(this);
        start.setText("Start CarsonCC background service");
        start.setOnClickListener(v -> {
            appendLog("Start requested");
            try {
                Intent intent = new Intent(this, CarsonService.class);
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) startForegroundService(intent);
                else startService(intent);
                status.setText("\nStatus: starting on 127.0.0.1:3939");
            } catch (Exception error) {
                status.setText("\nStatus: failed to start");
                appendLog("ERROR: Could not start service: " + error.getMessage());
            }
        });
        root.addView(start);

        Button stop = new Button(this);
        stop.setText("Stop service");
        stop.setOnClickListener(v -> {
            appendLog("Stop requested");
            stopService(new Intent(this, CarsonService.class));
            status.setText("\nStatus: stopped");
        });
        root.addView(stop);

        TextView consoleTitle = new TextView(this);
        consoleTitle.setText("\nActivity Console");
        consoleTitle.setTextSize(20);
        consoleTitle.setPadding(0, 20, 0, 8);
        root.addView(consoleTitle);

        TextView hint = new TextView(this);
        hint.setText("Live app and background-service events");
        hint.setTextSize(13);
        root.addView(hint);

        consoleScroll = new ScrollView(this);
        consoleScroll.setFillViewport(false);
        consoleScroll.setBackgroundColor(0xFF101418);
        consoleScroll.setPadding(14, 12, 14, 12);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, 300);
        params.topMargin = 10;

        console = new TextView(this);
        console.setTextColor(0xFFB8F5C6);
        console.setTextSize(12);
        console.setTypeface(android.graphics.Typeface.MONOSPACE);
        console.setText("CarsonCC Mobile console ready.\n");
        consoleScroll.addView(console);
        root.addView(consoleScroll, params);

        Button clear = new Button(this);
        clear.setText("Clear console");
        clear.setOnClickListener(v -> {
            console.setText("");
            appendLog("Console cleared");
        });
        root.addView(clear);

        setContentView(page);
        appendLog("App interface ready");
    }

    @Override protected void onStart() {
        super.onStart();
        IntentFilter filter = new IntentFilter(ACTION_LOG);
        ContextCompat.registerReceiver(this, logReceiver, filter, ContextCompat.RECEIVER_NOT_EXPORTED);
        appendLog("Console connected to service events");
    }

    @Override protected void onStop() {
        try { unregisterReceiver(logReceiver); }
        catch (IllegalArgumentException ignored) { }
        super.onStop();
    }

    private void appendLog(String message) {
        if (console == null) return;
        console.append("[" + timeFormat.format(new Date()) + "] " + message + "\n");
        if (consoleScroll != null) consoleScroll.post(() -> consoleScroll.fullScroll(View.FOCUS_DOWN));
    }
}