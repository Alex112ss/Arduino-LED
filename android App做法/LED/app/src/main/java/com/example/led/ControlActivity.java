package com.example.led;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import java.io.IOException;
import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.MediaType;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;
import android.content.Intent;

public class ControlActivity extends AppCompatActivity {

    private Button buttonOn;
    private Button buttonOff;
    private Button buttonBrighter;
    private Button buttonDimmer;
    private TextView textBrightness;
    private OkHttpClient client;
    private Button buttonLogout;

    private int ledBrightness = 128; // 初始亮度

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_control);

        client = new OkHttpClient();

        buttonOn = findViewById(R.id.buttonOn);
        buttonOff = findViewById(R.id.buttonOff);
        buttonBrighter = findViewById(R.id.buttonBrighter);
        buttonDimmer = findViewById(R.id.buttonDimmer);
        buttonLogout = findViewById(R.id.buttonLogout);
        textBrightness = findViewById(R.id.textBrightness);

        updateBrightnessText();

        buttonOn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                sendRequestToServer("on");
            }
        });

        buttonOff.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                sendRequestToServer("off");
            }
        });

        buttonBrighter.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ledBrightness = Math.min(ledBrightness + 50, 255);
                sendRequestToServer("brighter");
                updateBrightnessText();
            }
        });

        buttonDimmer.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ledBrightness = Math.max(ledBrightness - 50, 0);
                sendRequestToServer("dimmer");
                updateBrightnessText();
            }
        });

        buttonLogout.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                // 執行登出操作，回到登入頁面
                Intent intent = new Intent(ControlActivity.this, LoginActivity.class);
                startActivity(intent);
                finish(); // 結束當前活動
            }
        });
    }

    private void sendRequestToServer(String action) {
        MediaType mediaType = MediaType.parse("application/x-www-form-urlencoded");
        RequestBody requestBody = RequestBody.create(mediaType, "button=" + action);

        Request request = new Request.Builder()
                .url("http://192.168.137.224/" + action)
                .post(requestBody)
                .addHeader("Content-Type", "application/x-www-form-urlencoded")
                .build();

        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(ControlActivity.this, "请求失败：" + e.getMessage(), Toast.LENGTH_SHORT).show();
                    }
                });
            }

            @Override
            public void onResponse(Call call, Response response) throws IOException {
                // 不需要顯示成功訊息
            }
        });
    }

    private void updateBrightnessText() {
        textBrightness.setText("當前亮度：" + ledBrightness);
    }
}
