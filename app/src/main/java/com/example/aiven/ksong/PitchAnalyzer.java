package com.example.aiven.ksong;

import android.util.Log;

/**
 * Created by aiven on 16/5/6.
 */
public class PitchAnalyzer {

    /** Native methods, implemented in jni folder */
    public static native int NewPitchAnalyzer();
    public static native boolean PitchAnalyzerInit(int object, byte[] HeadData);
    public static native void PitchAnalyzerProcess(int object, byte[] Data, int length);
    public static native float PitchAnalyzerGetPitch(int object, long StartTime, long Duration);
    public static native int PitchAnalyzerGetFrequency(int object, long StartTime, long Duration);
    public static native void PitchAnalyzerReset(int object);
    public static native void DelPitchAnalyzer(int object);

    public static void callback_func(int starttime, int frequency)
    {
        Log.i("JNICallback", "starttime=" + starttime + ";frequency=" + frequency);
    }
    //    public native void nativeCallBack();
    /* this is used to load the 'hello-jni' library on application
     * startup. The library has already been unpacked into
     * /data/data/com.example.hellojni/lib/libhello-jni.so at
     * installation time by the package manager.
     */
    static {
        System.loadLibrary("hello-jni");
    }
}
