package com.carsoncc.mobile;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

public class MainActivity extends Activity {
    private TextView status;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(40, 48, 40, 40);

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
            Intent intent = new Intent(this, CarsonService.class);
            startForegroundService(intent);
            status.setText("\nStatus: running on 127.0.0.1:3939");
        });
        root.addView(start);

        Button stop = new Button(this);
        stop.setText("Stop service");
        stop.setOnClickListener(v -> {
            stopService(new Intent(this, CarsonService.class));
            status.setText("\nStatus: stopped");
        });
        root.addView(stop);

        setContentView(root);
    }
}
