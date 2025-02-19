package com.example.opengles;

import android.util.Log;

public class GLESNativeLib {

    static {
        try {
            System.loadLibrary("glNative");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Native code library failed to load.\n" + e);
            System.exit(1);
        }
    }
    public static void onCallBack() {
        Log.d("GLESNativeLib", "onCallbacak");
    }

    /**
     * @param width the current opengl view window width
     * @param height the current opengl view window view height
     */

    public static native void init( String apkFilePath );
    public static native void resize(int width, int height );
    public static native void step();
}
