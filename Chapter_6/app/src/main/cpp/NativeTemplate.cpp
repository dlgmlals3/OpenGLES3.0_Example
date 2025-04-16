// OpenGL ES 2.0 Cookcook code

#include <android/asset_manager_jni.h>
#include "NativeTemplate.h"
#ifdef __APPLE__
#include "renderer.h"
#else
#include "../Scene/renderer.h"
#endif

bool GraphicsInit(AAssetManager *mgr)
{
    Renderer::Instance().setAssetManager(mgr);
    Renderer::Instance().initializeRenderer();
    return true;
}

bool GraphicsResize( int width, int height )
{
    Renderer::Instance().resize(width, height);
    return true;
}

bool GraphicsRender()
{
    Renderer::Instance().setUpProjection();
    Renderer::Instance().render();
    return true;
}

void TouchEventDown( float x, float y )
{
    Renderer::Instance().TouchEventDown( x, y );
}

void TouchEventMove( float x, float y )
{
    Renderer::Instance().TouchEventMove( x, y );
}

void TouchEventRelease( float x, float y )
{
    Renderer::Instance().TouchEventRelease( x, y );
}


#ifdef __ANDROID__

JNIEXPORT void JNICALL Java_cookbook_gles_GLESNativeLib_init( JNIEnv *env, jobject obj, jstring FilePath, jobject assetManager )
{
	setenv( "FILESYSTEM", env->GetStringUTFChars( FilePath, NULL ), 1 );
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    GraphicsInit(mgr);
}

JNIEXPORT void JNICALL Java_cookbook_gles_GLESNativeLib_resize( JNIEnv *env, jobject obj, jint width, jint height)
{
	GraphicsResize( width, height );
}

JNIEXPORT void JNICALL Java_cookbook_gles_GLESNativeLib_step(JNIEnv * env, jobject obj)
{
	GraphicsRender();
    //renderFrame();
}

JNIEXPORT void JNICALL Java_cookbook_gles_GLESNativeLib_TouchEventStart(JNIEnv * env, jobject obj, float x, float y )
{
	TouchEventDown(x ,y);
}

JNIEXPORT void JNICALL Java_cookbook_gles_GLESNativeLib_TouchEventMove(JNIEnv * env, jobject obj, float x, float y )
{
	TouchEventMove(x ,y);
}

JNIEXPORT void JNICALL Java_cookbook_gles_GLESNativeLib_TouchEventRelease(JNIEnv * env, jobject obj, float x, float y )
{
	TouchEventRelease(x ,y);
}

#endif