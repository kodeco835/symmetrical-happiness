/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#import <UIKit/UIKit.h>

@class ABI38_0_0RCTSurface;

@interface ABI38_0_0RCTSurfaceBackedComponentState: NSObject

@property (atomic, readonly, strong) ABI38_0_0RCTSurface *surface;

+ (instancetype)newWithSurface:(ABI38_0_0RCTSurface *)surface;

@end
