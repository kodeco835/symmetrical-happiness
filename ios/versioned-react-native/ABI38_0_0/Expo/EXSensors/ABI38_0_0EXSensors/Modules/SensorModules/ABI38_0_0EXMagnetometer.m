// Copyright 2015-present 650 Industries. All rights reserved.

#import <ABI38_0_0EXSensors/ABI38_0_0EXMagnetometer.h>
#import <ABI38_0_0UMSensorsInterface/ABI38_0_0UMMagnetometerInterface.h>

@implementation ABI38_0_0EXMagnetometer

ABI38_0_0UM_EXPORT_MODULE(ExponentMagnetometer);

- (const NSString *)updateEventName
{
  return @"magnetometerDidUpdate";
}

- (id)getSensorServiceFromModuleRegistry:(ABI38_0_0UMModuleRegistry *)moduleRegistry
{
  return [moduleRegistry getModuleImplementingProtocol:@protocol(ABI38_0_0UMMagnetometerInterface)];
}

- (void)setUpdateInterval:(double)updateInterval onSensorService:(id)sensorService
{
  [sensorService setMagnetometerUpdateInterval:updateInterval];
}

- (BOOL)isAvailable:(id)sensorService
{
  return [sensorService isMagnetometerAvailable];
}

- (void)subscribeToSensorService:(id)sensorService withHandler:(void (^)(NSDictionary *event))handlerBlock
{
  [sensorService sensorModuleDidSubscribeForMagnetometerUpdates:self withHandler:handlerBlock];
}

- (void)unsubscribeFromSensorService:(id)sensorService
{
  [sensorService sensorModuleDidUnsubscribeForMagnetometerUpdates:self];
}

@end
