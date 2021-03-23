/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#import <ABI38_0_0React/ABI38_0_0RCTActionSheetManager.h>

#import <ABI38_0_0React/ABI38_0_0RCTBridge.h>
#import <ABI38_0_0React/ABI38_0_0RCTConvert.h>
#import <ABI38_0_0React/ABI38_0_0RCTLog.h>
#import <ABI38_0_0React/ABI38_0_0RCTUIManager.h>
#import <ABI38_0_0React/ABI38_0_0RCTUtils.h>

#import <ABI38_0_0FBReactNativeSpec/ABI38_0_0FBReactNativeSpec.h>
#import <ABI38_0_0RCTTypeSafety/ABI38_0_0RCTConvertHelpers.h>

#import "ABI38_0_0CoreModulesPlugins.h"

using namespace ABI38_0_0facebook::ABI38_0_0React;

@interface ABI38_0_0RCTActionSheetManager () <UIActionSheetDelegate, ABI38_0_0NativeActionSheetManagerSpec>
@end

@implementation ABI38_0_0RCTActionSheetManager
{
  // Use NSMapTable, as UIAlertViews do not implement <NSCopying>
  // which is required for NSDictionary keys
  NSMapTable *_callbacks;
}

ABI38_0_0RCT_EXPORT_MODULE()

@synthesize bridge = _bridge;

- (dispatch_queue_t)methodQueue
{
  return dispatch_get_main_queue();
}

- (void)presentViewController:(UIViewController *)alertController
       onParentViewController:(UIViewController *)parentViewController
                anchorViewTag:(NSNumber *)anchorViewTag
{
  alertController.modalPresentationStyle = UIModalPresentationPopover;
  UIView *sourceView = parentViewController.view;

  if (anchorViewTag) {
    sourceView = [self.bridge.uiManager viewForABI38_0_0ReactTag:anchorViewTag];
  } else {
    alertController.popoverPresentationController.permittedArrowDirections = 0;
  }
  alertController.popoverPresentationController.sourceView = sourceView;
  alertController.popoverPresentationController.sourceRect = sourceView.bounds;
  [parentViewController presentViewController:alertController animated:YES completion:nil];
}

ABI38_0_0RCT_EXPORT_METHOD(showActionSheetWithOptions:(JS::NativeActionSheetManager::SpecShowActionSheetWithOptionsOptions &)options
                  callback:(ABI38_0_0RCTResponseSenderBlock)callback)
{
  if (ABI38_0_0RCTRunningInAppExtension()) {
    ABI38_0_0RCTLogError(@"Unable to show action sheet from app extension");
    return;
  }

  if (!_callbacks) {
    _callbacks = [NSMapTable strongToStrongObjectsMapTable];
  }

  NSString *title = options.title();
  NSString *message = options.message();
  NSArray<NSString *> *buttons = ABI38_0_0RCTConvertOptionalVecToArray(options.options(), ^id(NSString *element) { return element; });
  NSInteger cancelButtonIndex = options.cancelButtonIndex() ? [ABI38_0_0RCTConvert NSInteger:@(*options.cancelButtonIndex())] : -1;
  NSArray<NSNumber *> *destructiveButtonIndices;
  if (options.destructiveButtonIndices()) {
    destructiveButtonIndices = ABI38_0_0RCTConvertVecToArray(*options.destructiveButtonIndices(), ^id(double element) { return @(element); });
  } else {
    NSNumber *destructiveButtonIndex = @-1;
    destructiveButtonIndices = @[destructiveButtonIndex];
  }

  UIViewController *controller = ABI38_0_0RCTPresentedViewController();
  NSNumber *anchor = [ABI38_0_0RCTConvert NSNumber:options.anchor() ? @(*options.anchor()) : nil];
  UIColor *tintColor = [ABI38_0_0RCTConvert UIColor:options.tintColor() ? @(*options.tintColor()) : nil];

  if (controller == nil) {
    ABI38_0_0RCTLogError(@"Tried to display action sheet but there is no application window. options: %@", @{
                                                                                      @"title": title,
                                                                                    @"message": message,
                                                                                    @"options": buttons,
                                                                          @"cancelButtonIndex": @(cancelButtonIndex),
                                                                   @"destructiveButtonIndices": destructiveButtonIndices,
                                                                                     @"anchor": anchor,
                                                                                  @"tintColor": tintColor,
    });
    return;
  }

  /*
   * The `anchor` option takes a view to set as the anchor for the share
   * popup to point to, on iPads running iOS 8. If it is not passed, it
   * defaults to centering the share popup on screen without any arrows.
   */
  NSNumber *anchorViewTag = anchor;

  UIAlertController *alertController =
  [UIAlertController alertControllerWithTitle:title
                                      message:message
                               preferredStyle:UIAlertControllerStyleActionSheet];

  NSInteger index = 0;
  for (NSString *option in buttons) {
    UIAlertActionStyle style = UIAlertActionStyleDefault;
    if ([destructiveButtonIndices containsObject:@(index)]) {
      style = UIAlertActionStyleDestructive;
    } else if (index == cancelButtonIndex) {
      style = UIAlertActionStyleCancel;
    }

    NSInteger localIndex = index;
    [alertController addAction:[UIAlertAction actionWithTitle:option
                                                        style:style
                                                      handler:^(__unused UIAlertAction *action){
      callback(@[@(localIndex)]);
    }]];

    index++;
  }

  alertController.view.tintColor = tintColor;
  [self presentViewController:alertController onParentViewController:controller anchorViewTag:anchorViewTag];
}

ABI38_0_0RCT_EXPORT_METHOD(showShareActionSheetWithOptions:(JS::NativeActionSheetManager::SpecShowShareActionSheetWithOptionsOptions &)options
                  failureCallback:(ABI38_0_0RCTResponseSenderBlock)failureCallback
                  successCallback:(ABI38_0_0RCTResponseSenderBlock)successCallback)
{
  if (ABI38_0_0RCTRunningInAppExtension()) {
    ABI38_0_0RCTLogError(@"Unable to show action sheet from app extension");
    return;
  }

  NSMutableArray<id> *items = [NSMutableArray array];
  NSString *message = options.message();
  if (message) {
    [items addObject:message];
  }
  NSURL *URL = [ABI38_0_0RCTConvert NSURL:options.url()];
  if (URL) {
    if ([URL.scheme.lowercaseString isEqualToString:@"data"]) {
      NSError *error;
      NSData *data = [NSData dataWithContentsOfURL:URL
                                           options:(NSDataReadingOptions)0
                                             error:&error];
      if (!data) {
        failureCallback(@[ABI38_0_0RCTJSErrorFromNSError(error)]);
        return;
      }
      [items addObject:data];
    } else {
      [items addObject:URL];
    }
  }
  if (items.count == 0) {
    ABI38_0_0RCTLogError(@"No `url` or `message` to share");
    return;
  }

  UIActivityViewController *shareController = [[UIActivityViewController alloc] initWithActivityItems:items applicationActivities:nil];

  NSString *subject = options.subject();
  if (subject) {
    [shareController setValue:subject forKey:@"subject"];
  }

  NSArray *excludedActivityTypes = ABI38_0_0RCTConvertOptionalVecToArray(options.excludedActivityTypes(), ^id(NSString *element) { return element; });
  if (excludedActivityTypes) {
    shareController.excludedActivityTypes = excludedActivityTypes;
  }

  UIViewController *controller = ABI38_0_0RCTPresentedViewController();
  shareController.completionWithItemsHandler = ^(NSString *activityType, BOOL completed, __unused NSArray *returnedItems, NSError *activityError) {
    if (activityError) {
      failureCallback(@[ABI38_0_0RCTJSErrorFromNSError(activityError)]);
    } else if (completed || activityType == nil) {
      successCallback(@[@(completed), ABI38_0_0RCTNullIfNil(activityType)]);
    }
  };

  NSNumber *anchorViewTag = [ABI38_0_0RCTConvert NSNumber:options.anchor() ? @(*options.anchor()) : nil];
  shareController.view.tintColor = [ABI38_0_0RCTConvert UIColor:options.tintColor() ? @(*options.tintColor()) : nil];

  [self presentViewController:shareController onParentViewController:controller anchorViewTag:anchorViewTag];
}

- (std::shared_ptr<TurboModule>)getTurboModuleWithJsInvoker:(std::shared_ptr<CallInvoker>)jsInvoker
{
  return std::make_shared<NativeActionSheetManagerSpecJSI>(self, jsInvoker);
}

@end

Class ABI38_0_0RCTActionSheetManagerCls(void) {
  return ABI38_0_0RCTActionSheetManager.class;
}
