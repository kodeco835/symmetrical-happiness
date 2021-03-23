/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#import <ABI38_0_0React/ABI38_0_0RCTVirtualTextShadowView.h>

#import <ABI38_0_0React/ABI38_0_0RCTShadowView+Layout.h>
#import <ABI38_0_0yoga/ABI38_0_0Yoga.h>

#import <ABI38_0_0React/ABI38_0_0RCTRawTextShadowView.h>

@implementation ABI38_0_0RCTVirtualTextShadowView {
  BOOL _isLayoutDirty;
}

#pragma mark - Layout

- (void)dirtyLayout
{
  [super dirtyLayout];

  if (_isLayoutDirty) {
    return;
  }
  _isLayoutDirty = YES;

  [self.superview dirtyLayout];
}

- (void)clearLayout
{
  _isLayoutDirty = NO;
}

@end
