package cookbook.gles;

import android.content.res.AssetManager;

public class GLESNativeLib {

    static {
        System.loadLibrary("glNative");
    }

    /**
     * @param width the current opengl view window width
     * @param height the current opengl view window view height
     */
    public static native void init( String apkFilePath, AssetManager assetManager);
    public static native void resize(int width, int height );
    public static native void step();

    public static native void TouchEventStart( float x, float y );
    public static native void TouchEventMove( float x, float y );
    public static native void TouchEventRelease( float x, float y );

}
