#include "djinni/jni/djinni_support.hpp"

#include <wasm_export.h>

// Called when library is loaded by the first class which uses it.
CJNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void * /*reserved*/) {
  djinni::jniInit(jvm);

  // Initialize WAMR runtime when library is loaded to setup mandatory
  // parameters.
  wasm_runtime_init();

  return JNI_VERSION_1_6;
}

// (Potentially) called when library is about to be unloaded.
CJNIEXPORT void JNICALL JNI_OnUnload(JavaVM * /*jvm*/, void * /*reserved*/) {
  // Deinit WAMR runtime when library is unloaded.
  wasm_runtime_destroy();

  djinni::jniShutdown();
}
