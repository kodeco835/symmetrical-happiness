// Copyright 2018-present 650 Industries. All rights reserved.

#import <ABI38_0_0EXHaptics/ABI38_0_0EXHapticsModule.h>

@interface ABI38_0_0EXHapticsModule ()

@end

@implementation ABI38_0_0EXHapticsModule

ABI38_0_0UM_EXPORT_MODULE(ExpoHaptics);

- (dispatch_queue_t)methodQueue
{
  return dispatch_get_main_queue();
}

ABI38_0_0UM_EXPORT_METHOD_AS(notificationAsync,
                    notifyWithType:(NSString *)inputType
                    resolve:(ABI38_0_0UMPromiseResolveBlock)resolve
                    reject:(ABI38_0_0UMPromiseRejectBlock)reject)
{
  NSDictionary* types = @{
                          @"success": @(UINotificationFeedbackTypeSuccess),
                          @"warning": @(UINotificationFeedbackTypeWarning),
                          @"error": @(UINotificationFeedbackTypeError),
                          };
  
  if (!types[inputType]) {
    return reject(@"E_HAPTICS_INVALID_ARG", [NSString stringWithFormat:@"'type' must be one of ['success', 'warning', 'error']. Obtained '%@'", inputType], nil);
  }
  
  UINotificationFeedbackGenerator *feedback = [UINotificationFeedbackGenerator new];
  [feedback prepare];
  [feedback notificationOccurred:[types[inputType] integerValue]];
  feedback = nil;
  
  resolve(nil);
}

ABI38_0_0UM_EXPORT_METHOD_AS(impactAsync,
                    impactWithStyle:(NSString *)inputStyle
                    resolve:(ABI38_0_0UMPromiseResolveBlock)resolve
                    reject:(ABI38_0_0UMPromiseRejectBlock)reject)
{
  NSDictionary* styles = @{
                           @"light": @(UIImpactFeedbackStyleLight),
                           @"medium": @(UIImpactFeedbackStyleMedium),
                           @"heavy": @(UIImpactFeedbackStyleHeavy),
                           };

  if (!styles[inputStyle]) {
    return reject(@"E_HAPTICS_INVALID_ARG", [NSString stringWithFormat:@"'style' must be one of ['light', 'medium', 'heavy']. Obtained '%@'", inputStyle], nil);
  }
  
  UIImpactFeedbackGenerator *feedback = [[UIImpactFeedbackGenerator alloc] initWithStyle:[styles[inputStyle] integerValue]];
  [feedback prepare];
  [feedback impactOccurred];
  feedback = nil;
  
  resolve(nil);
}

ABI38_0_0UM_EXPORT_METHOD_AS(selectionAsync,
                    resolve:(ABI38_0_0UMPromiseResolveBlock)resolve
                    reject:(ABI38_0_0UMPromiseRejectBlock)reject)
{
  UISelectionFeedbackGenerator *feedback = [UISelectionFeedbackGenerator new];
  [feedback prepare];
  [feedback selectionChanged];
  feedback = nil;
  
  resolve(nil);
}

@end
