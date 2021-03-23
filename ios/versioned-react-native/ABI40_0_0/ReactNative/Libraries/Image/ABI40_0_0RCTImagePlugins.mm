/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @generated by an internal plugin build system
 */

#ifndef ABI40_0_0RN_DISABLE_OSS_PLUGIN_HEADER

// OSS-compatibility layer

#import "ABI40_0_0RCTImagePlugins.h"

#import <string>
#import <unordered_map>

Class ABI40_0_0RCTImageClassProvider(const char *name) {
  static std::unordered_map<std::string, Class (*)(void)> sCoreModuleClassMap = {
    {"GIFImageDecoder", ABI40_0_0RCTGIFImageDecoderCls},
    {"ImageEditingManager", ABI40_0_0RCTImageEditingManagerCls},
    {"ImageLoader", ABI40_0_0RCTImageLoaderCls},
    {"ImageStoreManager", ABI40_0_0RCTImageStoreManagerCls},
    {"LocalAssetImageLoader", ABI40_0_0RCTLocalAssetImageLoaderCls},
  };

  auto p = sCoreModuleClassMap.find(name);
  if (p != sCoreModuleClassMap.end()) {
    auto classFunc = p->second;
    return classFunc();
  }
  return nil;
}

#endif // ABI40_0_0RN_DISABLE_OSS_PLUGIN_HEADER
