package com.carsoncc.mobile;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;

public class CarsonService extends Service {
    private static final String CHANNEL_ID = "carsoncc_background";
    private static final String ACTION_LOG = "com.carsoncc.mobile.LOG";
    private static final int PORT = 3939;
    private volatile boolean running;
    private ServerSocket serverSocket;
    private Thread serverThread;

    private void log(String message) {
        Intent event = new Intent(ACTION_LOG);
        event.setPackage(getPackageName());
        event.putExtra("message", message);
        sendBroadcast(event);
    }

    @Override public void onCreate() {
        super.onCreate();
        createChannel();
        startForeground(1, buildNotification());
        running = true;
        log("Background service started");
        serverThread = new Thread(this::serve, "CarsonCC-local-server");
        serverThread.start();
        log("Local server thread launched");
    }

    private void serve() {
        try {
            serverSocket = new ServerSocket(PORT, 8, InetAddress.getByName("127.0.0.1"));
            log("Listening on 127.0.0.1:" + PORT);
            while (running) {
                try (Socket socket = serverSocket.accept()) {
                    log("Client connected: " + socket.getInetAddress().getHostAddress());
                    handle(socket);
                } catch (Exception error) {
                    if (running) log("Client connection error: " + error.getMessage());
                }
            }
        } catch (Exception error) {
            if (running) {
                log("ERROR: Server failed: " + error.getMessage());
                stopSelf();
            }
        }
    }

    private void handle(Socket socket) throws Exception {
        socket.setSoTimeout(5000);
        BufferedReader input = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        PrintWriter output = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()), true);
        String command = input.readLine();
        if (command == null) {
            log("Client disconnected without a command");
            return;
        }
        String normalized = command.trim().toLowerCase();
        log("Command received: " + normalized);
        switch (normalized) {
            case "ping":
                output.println("pong");
                log("Responded: pong");
                break;
            case "status":
                output.println("carsoncc-mobile running port=3939");
                log("Responded with service status");
                break;
            case "version":
                output.println("CarsonCC Mobile 0.1");
                log("Responded with version");
                break;
            case "help":
                output.println("ping status version help");
                log("Sent command list");
                break;
            default:
                output.println("error: unknown command");
                log("Unknown command rejected");
                break;
        }
    }

    private Notification buildNotification() {
        return new Notification.Builder(this, CHANNEL_ID)
                .setContentTitle("CarsonCC Mobile")
                .setContentText("Low-power terminal bridge on localhost:3939")
                .setSmallIcon(android.R.drawable.stat_sys_upload)
                .setOngoing(true)
                .build();
    }

    private void createChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                    CHANNEL_ID, "CarsonCC background service", NotificationManager.IMPORTANCE_LOW);
            channel.setDescription("Local CarsonCC terminal bridge");
            NotificationManager manager = getSystemService(NotificationManager.class);
            manager.createNotificationChannel(channel);
        }
    }

    @Override public int onStartCommand(Intent intent, int flags, int startId) {
        log("Service start command received");
        return START_NOT_STICKY;
    }

    @Override public void onDestroy() {
        log("Background service stopping");
        running = false;
        try {
            if (serverSocket != null) serverSocket.close();
        } catch (Exception error) {
            log("Socket close warning: " + error.getMessage());
        }
        log("Background service stopped");
        super.onDestroy();
    }

    @Override public IBinder onBind(Intent intent) { return null; }
}