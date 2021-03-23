// Copyright 2015-present 650 Industries. All rights reserved.

#import <ABI38_0_0EXSensors/ABI38_0_0EXMagnetometerUncalibrated.h>
#import <ABI38_0_0UMSensorsInterface/ABI38_0_0UMMagnetometerUncalibratedInterface.h>

@implementation ABI38_0_0EXMagnetometerUncalibrated

ABI38_0_0UM_EXPORT_MODULE(ExponentMagnetometerUncalibrated);

- (const NSString *)updateEventName
{
  return @"magnetometerUncalibratedDidUpdate";
}

- (id)getSensorServiceFromModuleRegistry:(ABI38_0_0UMModuleRegistry *)moduleRegistry
{
  return [moduleRegistry getModuleImplementingProtocol:@protocol(ABI38_0_0UMMagnetometerUncalibratedInterface)];
}

- (void)setUpdateInterval:(double)updateInterval onSensorService:(id)sensorService
{
  [sensorService setMagnetometerUncalibratedUpdateInterval:updateInterval];
}

- (BOOL)isAvailable:(id)sensorService
{
  return [sensorService isMagnetometerUncalibratedAvailable];
}

- (void)subscribeToSensorService:(id)sensorService withHandler:(void (^)(NSDictionary *event))handlerBlock
{
  [sensorService sensorModuleDidSubscribeForMagnetometerUncalibratedUpdates:self withHandler:handlerBlock];
}

- (void)unsubscribeFromSensorService:(id)sensorService
{
  [sensorService sensorModuleDidUnsubscribeForMagnetometerUncalibratedUpdates:self];
}

@end
