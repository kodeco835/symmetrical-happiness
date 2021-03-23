// Copyright © 2018 650 Industries. All rights reserved.

#import <Foundation/Foundation.h>
#import <ABI38_0_0UMCore/ABI38_0_0UMDefines.h>
#import <ABI38_0_0UMCore/ABI38_0_0UMExportedModule.h>

@protocol ABI38_0_0UMEventEmitterService

- (void)sendEventWithName:(NSString *)name body:(id)body;

@end
