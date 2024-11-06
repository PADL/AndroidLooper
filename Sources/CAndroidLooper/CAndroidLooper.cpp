//
// Copyright (c) 2024 PADL Software Pty Ltd
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an 'AS IS' BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <cstring>

#include <jni.h>
#include <pthread.h>
#include <android/log.h>

#include "CAndroidLooper.h"

extern "C" {
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved);
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *pJavaVM, void *pReserved);
}

void CAndroidLooper_log(ALooper *_Nullable looper, const char *_Nonnull msg) {
  if (looper)
    __android_log_print(ANDROID_LOG_DEBUG, "CAndroidLooper",
                        "thread %lx looper %p -- %s", pthread_self(), looper,
                        msg);
  else
    __android_log_print(ANDROID_LOG_DEBUG, "CAndroidLooper", "thread %lx -- %s",
                        pthread_self(), msg);
}

static ALooper *_Nullable CAndroidLooper_mainLooper;
static JNIEnv *_Nullable CAndroidLooper_mainEnvironment;

static jobject CAndroidLooper_classLoader;
static jmethodID CAndroidLooper_findClassMethodID;

ALooper *_Nullable CAndroidLooper_getMainLooper(void) {
  return CAndroidLooper_mainLooper;
}

JNIEnv *_Nullable CAndroidLooper_getMainEnvironment(void) {
  return CAndroidLooper_mainEnvironment;
}

jclass CAndroidLooper_findClass(JNIEnv *jniEnv, const char *className) {
  if (CAndroidLooper_classLoader == nullptr ||
      CAndroidLooper_findClassMethodID == nullptr)
    return nullptr;

  auto aClass = static_cast<jclass>(jniEnv->CallObjectMethod(
      CAndroidLooper_classLoader, CAndroidLooper_findClassMethodID,
      jniEnv->NewStringUTF(className)));
  if (jniEnv->ExceptionOccurred()) {
    jniEnv->ExceptionClear();
    return nullptr;
  }

  return aClass;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
  JNIEnv *jniEnv;

  if (CAndroidLooper_mainLooper)
    ALooper_release(CAndroidLooper_mainLooper);
  CAndroidLooper_mainLooper = ALooper_forThread();
  ALooper_acquire(CAndroidLooper_mainLooper);

  jvm->GetEnv(reinterpret_cast<void **>(&jniEnv), JNI_VERSION_1_6);

  CAndroidLooper_log(nullptr, "loaded Swift AndroidLooper library");

  // now we are going to swizzle FindClass
  auto aClass = jniEnv->FindClass("com/padl/FlutterAndroid/SwiftObjectHolder");
  auto metaClass = jniEnv->GetObjectClass(aClass);
  auto classLoaderClass = jniEnv->FindClass("java/lang/ClassLoader");
  auto getClassLoaderMethodID = jniEnv->GetMethodID(
      metaClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
  auto classLoader = jniEnv->CallObjectMethod(aClass, getClassLoaderMethodID);

  // TODO: check exceptions

  CAndroidLooper_classLoader = jniEnv->NewGlobalRef(classLoader);
  CAndroidLooper_findClassMethodID = jniEnv->GetMethodID(
      classLoaderClass, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");

  CAndroidLooper_mainEnvironment = jniEnv;

  return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *pJavaVM, void *pReserved) {
  if (CAndroidLooper_mainLooper) {
    ALooper_release(CAndroidLooper_mainLooper);
    CAndroidLooper_mainLooper = nullptr;
  }

  if (CAndroidLooper_classLoader) {
    CAndroidLooper_mainEnvironment->DeleteGlobalRef(CAndroidLooper_classLoader);
    CAndroidLooper_classLoader = nullptr;
  }

  CAndroidLooper_mainEnvironment = nullptr;
  CAndroidLooper_log(nullptr, "unloaded Swift AndroidLooper library");
}
