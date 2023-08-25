// AUTOGENERATED FILE - DO NOT MODIFY!
// This file was generated by Djinni from whl.djinni

#import <Foundation/Foundation.h>

/** Configuration to instantiate a wasm_hot_loading instance. */
@interface Configuration : NSObject
- (nonnull instancetype)initWithSampleModuleUrl:(nonnull NSString *)sampleModuleUrl
                                      publicKey:(nonnull NSString *)publicKey
                                  wasmStackSize:(int32_t)wasmStackSize
                                   wasmHeapSize:(int32_t)wasmHeapSize;
+ (nonnull instancetype)configurationWithSampleModuleUrl:(nonnull NSString *)sampleModuleUrl
                                               publicKey:(nonnull NSString *)publicKey
                                           wasmStackSize:(int32_t)wasmStackSize
                                            wasmHeapSize:(int32_t)wasmHeapSize;

/** The URL to retrieve the Wasm module to load. */
@property (nonatomic, readonly, nonnull) NSString * sampleModuleUrl;

/** The public key used to verify module's signature. */
@property (nonatomic, readonly, nonnull) NSString * publicKey;

/** The stack size to use for the Wasm execution environment. */
@property (nonatomic, readonly) int32_t wasmStackSize;

/** The heap size to use for the Wasm execution environment. */
@property (nonatomic, readonly) int32_t wasmHeapSize;

@end