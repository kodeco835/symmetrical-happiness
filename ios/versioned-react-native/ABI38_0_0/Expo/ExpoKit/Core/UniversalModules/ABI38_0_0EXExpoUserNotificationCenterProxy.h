//  Copyright © 2018 650 Industries. All rights reserved.

#import <Foundation/Foundation.h>
#import <ABI38_0_0UMCore/ABI38_0_0UMInternalModule.h>
#import <ABI38_0_0UMPermissionsInterface/ABI38_0_0UMUserNotificationCenterProxyInterface.h>

@interface ABI38_0_0EXExpoUserNotificationCenterProxy : NSObject <ABI38_0_0UMInternalModule, ABI38_0_0UMUserNotificationCenterProxyInterface>

- (instancetype)initWithUserNotificationCenter:(id<ABI38_0_0UMUserNotificationCenterProxyInterface>)userNotificationCenter;

@end
