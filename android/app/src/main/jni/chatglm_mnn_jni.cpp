#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/log.h>

#include <jni.h>
#include <string>
#include <vector>
#include <sstream>
#include <thread>

#include "chat.hpp"

static ChatGLM chatglm;
static std::stringstream response_buffer;

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "JNI_OnLoad");
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "JNI_OnUnload");
}

JNIEXPORT jboolean JNICALL Java_com_mnn_chatglm_Chat_Init(JNIEnv* env, jobject thiz, jstring modelDir, jstring tokenizerDir) {
    if (chatglm.loadProgress() < 100) {
        const char* model_dir = env->GetStringUTFChars(modelDir, 0);
        const char* token_dir = env->GetStringUTFChars(tokenizerDir, 0);
        chatglm.load(4, 0, model_dir, token_dir);
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_mnn_chatglm_Chat_Ready(JNIEnv* env, jobject thiz) {
    if (chatglm.loadProgress() >= 100) {
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

JNIEXPORT jfloat JNICALL Java_com_mnn_chatglm_Chat_Progress(JNIEnv* env, jobject thiz) {
    return jfloat(chatglm.loadProgress());
}

JNIEXPORT jstring JNICALL Java_com_mnn_chatglm_Chat_Submit(JNIEnv* env, jobject thiz, jstring inputStr) {
    if (chatglm.loadProgress() < 100) {
        return env->NewStringUTF("Failed, Chat is not ready!");
    }
    const char* input_str = env->GetStringUTFChars(inputStr, 0);
    auto chat = [&](std::string str) {
        chatglm.response(str, &response_buffer);
    };
    std::thread chat_thread(chat, input_str);
    chat_thread.detach();
    jstring result = env->NewStringUTF("Submit success!");
    return result;
}

JNIEXPORT jstring JNICALL Java_com_mnn_chatglm_Chat_Response(JNIEnv* env, jobject thiz) {
    jstring result = env->NewStringUTF(response_buffer.str().c_str());
    return result;
}

JNIEXPORT void JNICALL Java_com_mnn_chatglm_Chat_Done(JNIEnv* env, jobject thiz) {
    response_buffer.str("");
}

JNIEXPORT void JNICALL Java_com_mnn_chatglm_Chat_Reset(JNIEnv* env, jobject thiz) {
    chatglm.reset();
}

} // extern "C"