package com.example.aiven.ksong;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        /* Create a TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        TextView tv = new TextView(this);
        tv.setText("yiqiding1");
        setContentView(tv);
//        setContentView(R.layout.HelloJni);

        int i=0;
        i=PitchAnalyzer.NewPitchAnalyzer();

        File file = new File("/sdcard/wozhixihuanni.wav");
        InputStream in = null;
        try {
            System.out.println("以字节为单位读取文件内容，一次读多个字节：");
            // 一次读多个字节
            byte[] tempbytes = new byte[4096];
            byte[] headdata = new byte[44];
            int byteread = 0;
            in = new FileInputStream(file);

            byteread=in.read(headdata);
            PitchAnalyzer.PitchAnalyzerInit(i, headdata);

            float pitch=0;
            int frequency=0;
            pitch=PitchAnalyzer.PitchAnalyzerGetPitch(i, 0, 600);
            Log.v("HelloJni", "pitch0=" + pitch);
            // 读入多个字节到字节数组中，byteread为一次读入的字节数
            while ((byteread = in.read(tempbytes)) != -1) {
                PitchAnalyzer.PitchAnalyzerProcess(i, tempbytes, byteread);
            }

            pitch=PitchAnalyzer.PitchAnalyzerGetPitch(i, 0, 600);
            frequency=PitchAnalyzer.PitchAnalyzerGetFrequency(i, 0, 600);
            Log.v("HelloJni", "pitch1="+pitch);
            pitch=PitchAnalyzer.PitchAnalyzerGetPitch(i, 108158, 600);
            frequency=PitchAnalyzer.PitchAnalyzerGetFrequency(i, 108158, 600);
            Log.v("HelloJni", "pitch2="+pitch);
            PitchAnalyzer.DelPitchAnalyzer(i);
        } catch (Exception e1) {
            e1.printStackTrace();
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException e1) {
                }
            }
        }

    }
}
