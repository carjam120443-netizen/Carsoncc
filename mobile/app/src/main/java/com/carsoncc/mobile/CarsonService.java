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
    private static final int PORT = 3939;
    private volatile boolean running;
    private ServerSocket serverSocket;
    private Thread serverThread;

    @Override
    public void onCreate() {
        super.onCreate();
        createChannel();
        startForeground(1, buildNotification());
        running = true;
        serverThread = new Thread(this::serve, "CarsonCC-local-server");
        serverThread.start();
    }

    private void serve() {
        try {
            serverSocket = new ServerSocket(PORT, 8, InetAddress.getByName("127.0.0.1"));
            while (running) {
                try (Socket socket = serverSocket.accept()) {
                    handle(socket);
                } catch (Exception ignored) {
                    if (running) {
                        // Keep the lightweight local bridge alive after a bad client.
                    }
                }
            }
        } catch (Exception ignored) {
            stopSelf();
        }
    }

    private void handle(Socket socket) throws Exception {
        socket.setSoTimeout(5000);
        BufferedReader input = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        PrintWriter output = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()), true);
        String command = input.readLine();
        if (command == null) {
            return;
        }
        switch (command.trim().toLowerCase()) {
            case "ping":
                output.println("pong");
                break;
            case "status":
                output.println("carsoncc-mobile running port=3939");
                break;
            case "version":
                output.println("CarsonCC Mobile 0.1");
                break;
            case "help":
                output.println("ping status version help");
                break;
            default:
                output.println("error: unknown command");
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
                    CHANNEL_ID,
                    "CarsonCC background service",
                    NotificationManager.IMPORTANCE_LOW);
            channel.setDescription("Local CarsonCC terminal bridge");
            NotificationManager manager = getSystemService(NotificationManager.class);
            manager.createNotificationChannel(channel);
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        running = false;
        try {
            if (serverSocket != null) {
                serverSocket.close();
            }
        } catch (Exception ignored) {
        }
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
