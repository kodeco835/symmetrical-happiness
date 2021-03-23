/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ABI38_0_0ImageManager.h"

#import <ABI38_0_0React/ABI38_0_0RCTImageLoaderWithAttributionProtocol.h>
#import <ABI38_0_0React/utils/ManagedObjectWrapper.h>

#import "ABI38_0_0RCTImageManager.h"

namespace ABI38_0_0facebook {
namespace ABI38_0_0React {

ImageManager::ImageManager(ContextContainer::Shared const &contextContainer)
{
  id<ABI38_0_0RCTImageLoaderWithAttributionProtocol> imageLoader =
      (id<ABI38_0_0RCTImageLoaderWithAttributionProtocol>)unwrapManagedObject(
          contextContainer->at<std::shared_ptr<void>>("ABI38_0_0RCTImageLoader"));
  self_ = (__bridge_retained void *)[[ABI38_0_0RCTImageManager alloc] initWithImageLoader:imageLoader];
}

ImageManager::~ImageManager()
{
  CFRelease(self_);
  self_ = nullptr;
}

ImageRequest ImageManager::requestImage(const ImageSource &imageSource, SurfaceId surfaceId) const
{
  ABI38_0_0RCTImageManager *imageManager = (__bridge ABI38_0_0RCTImageManager *)self_;
  return [imageManager requestImage:imageSource surfaceId:surfaceId];
}

} // namespace ABI38_0_0React
} // namespace ABI38_0_0facebook
