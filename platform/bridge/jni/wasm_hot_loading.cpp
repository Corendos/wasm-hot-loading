// AUTOGENERATED FILE - DO NOT MODIFY!
// This file was generated by Djinni from whl.djinni

#include "wasm_hot_loading.hpp"  // my header
#include "configuration.hpp"
#include "djinni/jni/Marshal.hpp"
#include "state_listener.hpp"

namespace whl { namespace bridge {

WasmHotLoading::WasmHotLoading() : ::djinni::JniInterface<::whl::WasmHotLoading, WasmHotLoading>("io/corendos/whl/bridge/WasmHotLoading$CppProxy") {}

WasmHotLoading::~WasmHotLoading() = default;


CJNIEXPORT void JNICALL Java_io_corendos_whl_bridge_WasmHotLoading_00024CppProxy_nativeDestroy(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        delete reinterpret_cast<::djinni::CppProxyHandle<::whl::WasmHotLoading>*>(nativeRef);
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT jobject JNICALL Java_io_corendos_whl_bridge_WasmHotLoading_00024CppProxy_create(JNIEnv* jniEnv, jobject /*this*/, jobject j_configuration)
{
    try {
        DJINNI_FUNCTION_PROLOGUE0(jniEnv);
        auto r = ::whl::WasmHotLoading::create(::whl::bridge::Configuration::toCpp(jniEnv, j_configuration));
        return ::djinni::release(::whl::bridge::WasmHotLoading::fromCpp(jniEnv, r));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, 0 /* value doesn't matter */)
}

CJNIEXPORT void JNICALL Java_io_corendos_whl_bridge_WasmHotLoading_00024CppProxy_native_1registerStateListener(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jobject j_listener)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::whl::WasmHotLoading>(nativeRef);
        ref->register_state_listener(::whl::bridge::StateListener::toCpp(jniEnv, j_listener));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_io_corendos_whl_bridge_WasmHotLoading_00024CppProxy_native_1getState(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::whl::WasmHotLoading>(nativeRef);
        ref->get_state();
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

CJNIEXPORT void JNICALL Java_io_corendos_whl_bridge_WasmHotLoading_00024CppProxy_native_1update(JNIEnv* jniEnv, jobject /*this*/, jlong nativeRef, jstring j_name, jint j_value)
{
    try {
        DJINNI_FUNCTION_PROLOGUE1(jniEnv, nativeRef);
        const auto& ref = ::djinni::objectFromHandleAddress<::whl::WasmHotLoading>(nativeRef);
        ref->update(::djinni::String::toCpp(jniEnv, j_name),
                    ::djinni::I32::toCpp(jniEnv, j_value));
    } JNI_TRANSLATE_EXCEPTIONS_RETURN(jniEnv, )
}

} }  // namespace whl::bridge
