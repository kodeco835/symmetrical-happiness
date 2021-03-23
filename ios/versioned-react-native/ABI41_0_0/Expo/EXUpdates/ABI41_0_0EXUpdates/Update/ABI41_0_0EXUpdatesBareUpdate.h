//  Copyright © 2019 650 Industries. All rights reserved.

#import <ABI41_0_0EXUpdates/ABI41_0_0EXUpdatesUpdate.h>

NS_ASSUME_NONNULL_BEGIN

@interface ABI41_0_0EXUpdatesBareUpdate : NSObject

+ (ABI41_0_0EXUpdatesUpdate *)updateWithBareManifest:(NSDictionary *)manifest
                                     config:(ABI41_0_0EXUpdatesConfig *)config
                                   database:(ABI41_0_0EXUpdatesDatabase *)database;

@end

NS_ASSUME_NONNULL_END
