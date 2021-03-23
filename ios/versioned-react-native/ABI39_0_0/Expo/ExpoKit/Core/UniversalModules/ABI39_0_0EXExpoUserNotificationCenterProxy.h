//  Copyright © 2018 650 Industries. All rights reserved.

#import <Foundation/Foundation.h>
#import <ABI39_0_0UMCore/ABI39_0_0UMInternalModule.h>
#import <ABI39_0_0UMPermissionsInterface/ABI39_0_0UMUserNotificationCenterProxyInterface.h>

@interface ABI39_0_0EXExpoUserNotificationCenterProxy : NSObject <ABI39_0_0UMInternalModule, ABI39_0_0UMUserNotificationCenterProxyInterface>

- (instancetype)initWithUserNotificationCenter:(id<ABI39_0_0UMUserNotificationCenterProxyInterface>)userNotificationCenter;

@end
