/*!
 * \brief Implements all the functions exposed as part of the MC/Nova interface
 *
 * \author ddubois 
 * \date 14-Aug-18.
 */

#include "jni/com_continuum_nova_system_NovaNative.h"
#include "../nova_renderer.h"

/*!
 * \brief Initializes Nova
 *
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    initialize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_initialize
        (JNIEnv *, jclass) {
    nova::nova_renderer::initialize();
}

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    executeFrame
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_executeFrame
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    sendLightmapTexture
 * Signature: ([IIII)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_sendLightmapTexture
        (JNIEnv *, jclass, jintArray, jint, jint, jint);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    addTexture
 * Signature: (Lcom/continuum/nova/system/MinecraftAtlasTexture;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_addTexture
        (JNIEnv *, jclass, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    addTextureLocation
 * Signature: (Lcom/continuum/nova/system/MinecraftTextureAtlasLocation;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_addTextureLocation
        (JNIEnv *, jclass, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getMaxTextureSize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_continuum_nova_system_NovaNative_getMaxTextureSize
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    resetTextureManager
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_resetTextureManager
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    addChunkGeometryForFilter
 * Signature: (Ljava/lang/String;Lcom/continuum/nova/system/MinecraftChunkRenderObject;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_addChunkGeometryForFilter
        (JNIEnv *, jclass, jstring, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    removeChunkGeometryForFilter
 * Signature: (Ljava/lang/String;Lcom/continuum/nova/system/MinecraftChunkRenderObject;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_removeChunkGeometryForFilter
        (JNIEnv *, jclass, jstring, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    shouldClose
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_continuum_nova_system_NovaNative_shouldClose
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    addGuiGeometry
 * Signature: (Ljava/lang/String;Lcom/continuum/nova/system/MinecraftGuiBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_addGuiGeometry
        (JNIEnv *, jclass, jstring, jobject);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    clearGuiBuffers
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_clearGuiBuffers
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    setMouseGrabbed
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_setMouseGrabbed
        (JNIEnv *, jclass, jboolean);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getNextMouseButtonEvent
 * Signature: ()Lcom/continuum/nova/system/MouseButtonEvent;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_getNextMouseButtonEvent
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getNextMousePositionEvent
 * Signature: ()Lcom/continuum/nova/system/MousePositionEvent;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_getNextMousePositionEvent
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getNextMouseScrollEvent
 * Signature: ()Lcom/continuum/nova/system/MouseScrollEvent;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_getNextMouseScrollEvent
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getNextKeyPressEvent
 * Signature: ()Lcom/continuum/nova/system/KeyPressEvent;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_getNextKeyPressEvent
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getNextKeyCharEvent
 * Signature: ()Lcom/continuum/nova/system/KeyCharEvent;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_getNextKeyCharEvent
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getWindowSize
 * Signature: ()Lcom/continuum/nova/system/WindowSize;
 */
JNIEXPORT jobject JNICALL Java_com_continuum_nova_system_NovaNative_getWindowSize
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    setFullscreen
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_setFullscreen
        (JNIEnv *, jclass, jboolean);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    displayIsActive
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_continuum_nova_system_NovaNative_displayIsActive
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    setStringSetting
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_setStringSetting
        (JNIEnv *, jclass, jstring, jstring);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    setFloatSetting
 * Signature: (Ljava/lang/String;F)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_setFloatSetting
        (JNIEnv *, jclass, jstring, jfloat);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    setPlayerCameraTransform
 * Signature: (DDDFF)V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_setPlayerCameraTransform
        (JNIEnv *, jclass, jdouble, jdouble, jdouble, jfloat, jfloat);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    getMaterialsAndFilters
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_continuum_nova_system_NovaNative_getMaterialsAndFilters
        (JNIEnv *, jclass);

/*
 * Class:     com_continuum_nova_system_NovaNative
 * Method:    destruct
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_continuum_nova_system_NovaNative_destruct
        (JNIEnv *, jclass) {
    nova::nova_renderer::deinitialize();
}
