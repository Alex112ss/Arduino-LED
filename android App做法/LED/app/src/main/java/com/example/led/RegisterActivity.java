package com.example.led;

import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import java.io.BufferedWriter;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;

public class RegisterActivity extends AppCompatActivity {

    private EditText editTextUsername;
    private EditText editTextPassword;
    private Button buttonRegister;
    private Button buttonBack;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_register);

        editTextUsername = findViewById(R.id.editTextUsername);
        editTextPassword = findViewById(R.id.editTextPassword);
        buttonRegister = findViewById(R.id.buttonRegister);
        buttonBack = findViewById(R.id.buttonBack);

        buttonRegister.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String username = editTextUsername.getText().toString();
                String password = editTextPassword.getText().toString();

                RegisterTask registerTask = new RegisterTask();
                registerTask.execute(username, password);
            }
        });

        buttonBack.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onBackPressed(); // 使用內建的返回行為
            }
        });
    }
    private class RegisterTask extends AsyncTask<String, Void, Boolean> {
        @Override
        protected Boolean doInBackground(String... params) {
            try {
                URL url = new URL("http://192.168.137.224/register"); // 替換為你的網頁伺服器的 URL
                HttpURLConnection connection = (HttpURLConnection) url.openConnection();
                connection.setRequestMethod("POST");
                connection.setDoOutput(true);

                OutputStream os = connection.getOutputStream();
                BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(os, "UTF-8"));
                String data = URLEncoder.encode("username", "UTF-8") + "=" + URLEncoder.encode(params[0], "UTF-8")
                        + "&" + URLEncoder.encode("password", "UTF-8") + "=" + URLEncoder.encode(params[1], "UTF-8");
                writer.write(data);
                writer.flush();
                writer.close();
                os.close();

                InputStream is = connection.getInputStream();
                InputStreamReader reader = new InputStreamReader(is, "UTF-8");
                char[] buffer = new char[256];
                int bytesRead;
                StringBuilder response = new StringBuilder();
                while ((bytesRead = reader.read(buffer)) != -1) {
                    response.append(buffer, 0, bytesRead);
                }
                reader.close();
                is.close();

                return response.toString().equals("註冊成功"); // 確保伺服器端回傳的訊息正確
            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }

        @Override
        protected void onPostExecute(Boolean result) {
            if (result) {
                // 註冊成功時顯示訊息
                Toast.makeText(RegisterActivity.this, "註冊成功！", Toast.LENGTH_SHORT).show();
            }
            else{

                Toast.makeText(RegisterActivity.this, "註冊成功！", Toast.LENGTH_SHORT).show();
            }
        }
    }
}
