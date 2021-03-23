// Copyright 2015-present 650 Industries. All rights reserved.

#if __has_include(<ABI38_0_0EXFacebook/ABI38_0_0EXFacebook.h>)
#import "ABI38_0_0EXScopedFacebook.h"
#import <FBSDKCoreKit/FBSDKSettings.h>
#import <ABI38_0_0UMCore/ABI38_0_0UMAppLifecycleService.h>
#import <FBSDKCoreKit/FBSDKApplicationDelegate.h>

@interface ABI38_0_0EXFacebook (ExportedMethods)

- (void)initializeWithAppId:(NSString *)appId
                    appName:(NSString *)appName
                   resolver:(ABI38_0_0UMPromiseResolveBlock)resolve
                   rejecter:(ABI38_0_0UMPromiseRejectBlock)reject;

- (void)logInWithReadPermissionsWithConfig:(NSDictionary *)config
                                  resolver:(ABI38_0_0UMPromiseResolveBlock)resolve
                                  rejecter:(ABI38_0_0UMPromiseRejectBlock)reject;

- (void)setAutoInitEnabled:(BOOL)enabled
                  resolver:(ABI38_0_0UMPromiseResolveBlock)resolve
                  rejecter:(ABI38_0_0UMPromiseRejectBlock)reject;

@end

static NSString *AUTO_INIT_KEY = @"autoInitEnabled";

@interface ABI38_0_0EXScopedFacebook ()

@property (nonatomic, assign) BOOL isInitialized;
@property (nonatomic, strong) NSString *appId;
@property (nonatomic, strong) NSString *displayName;
@property (nonatomic, strong) NSUserDefaults *settings;

@end

// Expo client-only ABI38_0_0EXFacebook module, which ensures that Facebook SDK configurations
// of different experiences don't collide.

@implementation ABI38_0_0EXScopedFacebook : ABI38_0_0EXFacebook

- (instancetype)initWithExperienceId:(NSString *)experienceId andParams:(NSDictionary *)params
{
  if (self = [super init]) {
    NSString *suiteName = [NSString stringWithFormat:@"%@#%@", NSStringFromClass(self.class), experienceId];
    _settings = [[NSUserDefaults alloc] initWithSuiteName:suiteName];

    BOOL hasPreviouslySetAutoInitEnabled = [_settings boolForKey:AUTO_INIT_KEY];
    BOOL manifestDefinesAutoInitEnabled = [params[@"manifest"][@"facebookAutoInitEnabled"] boolValue];
    
    NSString *scopedFacebookAppId = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"FacebookAppID"];
    NSString *manifestFacebookAppId = params[@"manifest"][@"facebookAppId"];

    if (hasPreviouslySetAutoInitEnabled || manifestDefinesAutoInitEnabled) {
      // This happens even before the app foregrounds, which mimics
      // the mechanism behind ABI38_0_0EXFacebookAppDelegate.
      // Check for FacebookAppId in case this is a custom client build
      if (scopedFacebookAppId) {
        [FBSDKApplicationDelegate initializeSDK:nil];
        _isInitialized = YES;
        if (manifestFacebookAppId) {
          ABI38_0_0UMLogInfo(@"Overriding Facebook App ID with Expo Go. To test your own Facebook App ID, you'll need to build a standalone app. Refer to our documentation for more info- https://docs.expo.io/versions/latest/sdk/facebook/");
        }
      } else {
        ABI38_0_0UMLogWarn(@"FacebookAutoInit is enabled, but no FacebookAppId has been provided. Facebook SDK initialization aborted.");
      }
    }
  }
  return self;
}

- (void)initializeWithAppId:(NSString *)appId
                    appName:(NSString *)appName
                   resolver:(ABI38_0_0UMPromiseResolveBlock)resolve
                   rejecter:(ABI38_0_0UMPromiseRejectBlock)reject
{
  _isInitialized = YES;
  if (appId) {
    ABI38_0_0UMLogInfo(@"Overriding Facebook App ID with Expo Go. To test your own Facebook App ID, you'll need to build a standalone app. Refer to our documentation for more info- https://docs.expo.io/versions/latest/sdk/facebook/");
  }
  NSString *scopedFacebookAppId = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"FacebookAppID"];
  [super initializeWithAppId:scopedFacebookAppId appName:appName resolver:resolve rejecter:reject];
}

- (void)setAutoInitEnabled:(BOOL)enabled resolver:(ABI38_0_0UMPromiseResolveBlock)resolve rejecter:(ABI38_0_0UMPromiseRejectBlock)reject
{
  if (enabled) {
    [_settings setBool:enabled forKey:AUTO_INIT_KEY];
    // Facebook SDK on iOS is initialized when `setAutoInitEnabled` is called with `YES`.
    _isInitialized = YES;
  }
  [super setAutoInitEnabled:enabled resolver:resolve rejecter:reject];
}

- (void)logInWithReadPermissionsWithConfig:(NSDictionary *)config resolver:(ABI38_0_0UMPromiseResolveBlock)resolve rejecter:(ABI38_0_0UMPromiseRejectBlock)reject
{
  // If the developer didn't initialize the SDK, let them know.
  if (!_isInitialized) {
    reject(@"E_NO_INIT", @"Facebook SDK has not been initialized yet.", nil);
    return;
  }
  [super logInWithReadPermissionsWithConfig:config resolver:resolve rejecter:reject];
}

# pragma mark - ABI38_0_0UMModuleRegistryConsumer

- (void)setModuleRegistry:(ABI38_0_0UMModuleRegistry *)moduleRegistry
{
  id<ABI38_0_0UMAppLifecycleService> appLifecycleService = [moduleRegistry getModuleImplementingProtocol:@protocol(ABI38_0_0UMAppLifecycleService)];
  [appLifecycleService registerAppLifecycleListener:self];
}

# pragma mark - ABI38_0_0UMAppLifecycleListener

- (void)onAppBackgrounded {
  // Save SDK settings state
  _appId = [FBSDKSettings appID];
  _displayName = [FBSDKSettings displayName];
  [FBSDKSettings setAppID:nil];
  [FBSDKSettings setDisplayName:nil];
}

- (void)onAppForegrounded {
  // Restore SDK settings state
  if (_appId) {
    [FBSDKSettings setAppID:_appId];
  }
  if (_displayName) {
    [FBSDKSettings setDisplayName:_displayName];
  }
}

@end
#endif
