#import "EXDevLauncherController+Private.h"

#import <React/RCTBundleURLProvider.h>
#import <React/RCTRootView.h>
#import <React/RCTDevMenu.h>
#import <React/RCTAsyncLocalStorage.h>
#import <React/RCTDevSettings.h>
#import <React/RCTRootContentView.h>
#import <React/RCTAppearance.h>
#import <React/RCTConstants.h>

#import "EXDevLauncherBundle.h"
#import "EXDevLauncherBundleSource.h"
#import "EXDevLauncherRCTBridge.h"
#import "EXDevLauncherManifestParser.h"
#import "EXDevLauncherLoadingView.h"

#import <EXDevLauncher-Swift.h>

@import EXDevMenuInterface;

// Uncomment the below and set it to a React Native bundler URL to develop the launcher JS
//#define DEV_LAUNCHER_URL "http://10.0.0.176:8090/index.bundle?platform=ios&dev=true&minify=false"

NSString *fakeLauncherBundleUrl = @"embedded://EXDevLauncher/dummy";

@implementation EXDevLauncherController

+ (instancetype)sharedInstance
{
  static EXDevLauncherController *theController;
  static dispatch_once_t once;
  dispatch_once(&once, ^{
    if (!theController) {
      theController = [[EXDevLauncherController alloc] init];
    }
  });
  return theController;
}

- (instancetype)init {
  if (self = [super init]) {
    self.recentlyOpenedAppsRegistry = [EXDevLauncherRecentlyOpenedAppsRegistry new];
    self.pendingDeepLinkRegistry = [EXDevLauncherPendingDeepLinkRegistry new];
  }
  return self;
}

- (NSArray<id<RCTBridgeModule>> *)extraModulesForBridge:(RCTBridge *)bridge
{
  return @[
    [[RCTDevMenu alloc] init],
    [[RCTAsyncLocalStorage alloc] init],
    [[EXDevLauncherLoadingView alloc] init]
  ];
}

#ifdef DEV_LAUNCHER_URL

- (NSURL *)sourceURLForBridge:(RCTBridge *)bridge
{
  // LAN url for developing launcher JS
  return [NSURL URLWithString:@(DEV_LAUNCHER_URL)];
}

#else

- (NSURL *)sourceURLForBridge:(RCTBridge *)bridge
{
  return [NSURL URLWithString:fakeLauncherBundleUrl];
}

- (void)loadSourceForBridge:(RCTBridge *)bridge withBlock:(RCTSourceLoadBlock)loadCallback
{
  NSData *data = [NSData dataWithBytesNoCopy:EXDevLauncherBundle
                                      length:EXDevLauncherBundleLength
                                freeWhenDone:NO];
  loadCallback(nil, EXDevLauncherBundleSourceCreate([NSURL URLWithString:fakeLauncherBundleUrl],
                                                          data,
                                                          EXDevLauncherBundleLength));
}

#endif

- (NSDictionary *)recentlyOpenedApps
{
  return [_recentlyOpenedAppsRegistry recentlyOpenedApps];
}

- (NSDictionary<UIApplicationLaunchOptionsKey, NSObject*> *)getLaunchOptions;
{
  NSURL *deepLink = [self.pendingDeepLinkRegistry consumePendingDeepLink];
  if (!deepLink) {
    return nil;
  }
  
  return @{
    UIApplicationLaunchOptionsURLKey: deepLink
  };
}

- (EXDevLauncherManifest *)appManifest
{
  return self.manifest;
}

- (void)startWithWindow:(UIWindow *)window delegate:(id<EXDevLauncherControllerDelegate>)delegate launchOptions:(NSDictionary *)launchOptions
{
  _delegate = delegate;
  _launchOptions = launchOptions;
  _window = window;

  [self navigateToLauncher];
}

- (void)navigateToLauncher
{
  [_appBridge invalidate];
  self.manifest = nil;

  if (@available(iOS 12, *)) {
    [self _applyUserInterfaceStyle:UIUserInterfaceStyleUnspecified];
  }
  
  _launcherBridge = [[EXDevLauncherRCTBridge alloc] initWithDelegate:self launchOptions:_launchOptions];
  
  id<DevMenuManagerProviderProtocol> devMenuManagerProvider = [_launcherBridge modulesConformingToProtocol:@protocol(DevMenuManagerProviderProtocol)].firstObject;
  
  if (devMenuManagerProvider) {
    id<DevMenuManagerProtocol> devMenuManager = [devMenuManagerProvider getDevMenuManager];
    devMenuManager.delegate = [[EXDevLauncherMenuDelegate alloc] initWithBridge:_launcherBridge];
  }
  RCTRootView *rootView = [[RCTRootView alloc] initWithBridge:_launcherBridge
                                                   moduleName:@"main"
                                            initialProperties:@{
                                              @"isSimulator":
                                                              #if TARGET_IPHONE_SIMULATOR
                                                              @YES
                                                              #else
                                                              @NO
                                                              #endif
                                            }];

  [self _ensureUserInterfaceStyleIsInSyncWithViewController:rootView];
  
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(onAppContentDidAppear)
                                               name:RCTContentDidAppearNotification
                                             object:rootView];

  rootView.backgroundColor = [[UIColor alloc] initWithRed:1.0f green:1.0f blue:1.0f alpha:1];

  UIViewController *rootViewController = [UIViewController new];
  rootViewController.view = rootView;
  _window.rootViewController = rootViewController;

  [_window makeKeyAndVisible];
}

- (BOOL)onDeepLink:(NSURL *)url options:(NSDictionary *)options {
  if (![EXDevLauncherURLHelper isDevLauncherURL:url]) {
    return [self _handleExternalDeepLink:url options:options];
  }
  
  NSURL *appUrl = [EXDevLauncherURLHelper getAppURLFromDevLauncherURL:url];
  if (appUrl) {
    [self loadApp:appUrl onSuccess:nil onError:^(NSError *error) {
      NSLog(error.description);
    }];
    return true;
  }
  
  [self navigateToLauncher];
  return true;
}

- (BOOL)_handleExternalDeepLink:(NSURL *)url options:(NSDictionary *)options
{
  if ([self _isAppRunning]) {
    return false;
  }
  
  self.pendingDeepLinkRegistry.pendingDeepLink = url;
  return true;
}

- (void)loadApp:(NSURL *)expoUrl onSuccess:(void (^)())onSuccess onError:(void (^)(NSError *error))onError
{
  NSURL *url = [EXDevLauncherURLHelper changeURLScheme:expoUrl to:@"http"];
  
  if (@available(iOS 14, *)) {
    // Try to detect if we're trying to open a local network URL so we can preemptively show the
    // Local Network permission prompt -- otherwise the network request will fail before the user
    // has time to accept or reject the permission.
    NSString *host = url.host;
    if ([host hasPrefix:@"192.168."] || [host hasPrefix:@"172."] || [host hasPrefix:@"10."]) {
      // We want to trigger the local network permission dialog. However, the iOS API doesn't expose a way to do it.
      // But we can use system functionality that needs this permission to trigger prompt.
      // See https://stackoverflow.com/questions/63940427/ios-14-how-to-trigger-local-network-dialog-and-check-user-answer
      static dispatch_once_t once;
      dispatch_once(&once, ^{
        [[NSProcessInfo processInfo] hostName];
      });
    }
  }
    
  EXDevLauncherManifestParser *manifestParser = [[EXDevLauncherManifestParser alloc] initWithURL:url session:[NSURLSession sharedSession]];
  [manifestParser tryToParseManifest:^(EXDevLauncherManifest * _Nullable manifest) {
    NSURL *bundleUrl = [NSURL URLWithString:manifest.bundleUrl];
    
    [_recentlyOpenedAppsRegistry appWasOpened:[expoUrl absoluteString] name:manifest.name];
    [self _initApp:bundleUrl manifest:manifest];
    if (onSuccess) {
      onSuccess();
    }
  } onInvalidManifestURL:^{
    [_recentlyOpenedAppsRegistry appWasOpened:[expoUrl absoluteString] name:nil];
    if ([url.path isEqual:@"/"] || [url.path isEqual:@""]) {
      [self _initApp:[NSURL URLWithString:@"index.bundle?platform=ios&dev=true&minify=false" relativeToURL:url] manifest:nil];
    } else {
      [self _initApp:url manifest:nil];
    }
    
    if (onSuccess) {
      onSuccess();
    }
  } onError:onError];
}

- (void)_initApp:(NSURL *)bundleUrl manifest:(EXDevLauncherManifest * _Nullable)manifest
{
  self.manifest = manifest;
  __block UIInterfaceOrientation orientation = manifest.orientation;
  __block UIColor *backgroundColor = manifest.backgroundColor;
  
  dispatch_async(dispatch_get_main_queue(), ^{
    self.sourceUrl = bundleUrl;
    
    if (@available(iOS 12, *)) {
      [self _applyUserInterfaceStyle:manifest.userInterfaceStyle];
      
      // Fix for the community react-native-appearance.
      // RNC appearance checks the global trait collection and doesn't have another way to override the user interface.
      // So we swap `currentTraitCollection` with one from the root view controller.
      // Note that the root view controller will have the correct value of `userInterfaceStyle`.
      if (manifest.userInterfaceStyle != UIUserInterfaceStyleUnspecified) {
        UITraitCollection.currentTraitCollection = [_window.rootViewController.traitCollection copy];
      }
    }
    
    [self.delegate devLauncherController:self didStartWithSuccess:YES];
    
    [self _ensureUserInterfaceStyleIsInSyncWithViewController:_window.rootViewController];

    [[UIDevice currentDevice] setValue:@(orientation) forKey:@"orientation"];
    [UIViewController attemptRotationToDeviceOrientation];
    
    if (backgroundColor) {
      _window.rootViewController.view.backgroundColor = backgroundColor;
      _window.backgroundColor = backgroundColor;
    }
  });
}

- (BOOL)_isAppRunning
{
  return [_appBridge isValid];
}

/**
 * Temporary `expo-splash-screen` fix.
 *
 * The dev-launcher's bridge doesn't contain unimodules. So the module shows a splash screen but never hides.
 * For now, we just remove the splash screen view when the launcher is loaded.
 */
- (void)onAppContentDidAppear
{
  [[NSNotificationCenter defaultCenter] removeObserver:self];

  dispatch_async(dispatch_get_main_queue(), ^{
    NSArray<UIView *> *views = [[[self->_window rootViewController] view] subviews];
    for (UIView *view in views) {
      if (![view isKindOfClass:[RCTRootContentView class]]) {
        [view removeFromSuperview];
      }
    }
  });
}

/**
 * We need that function to sync the dev-menu user interface with the main application.
 */
- (void)_ensureUserInterfaceStyleIsInSyncWithViewController:(UIViewController *)controller
{
  [[NSNotificationCenter defaultCenter] postNotificationName:RCTUserInterfaceStyleDidChangeNotification
                                                      object:controller
                                                    userInfo:@{
                                                      RCTUserInterfaceStyleDidChangeNotificationTraitCollectionKey : controller.traitCollection
                                                    }];
}

- (void)_applyUserInterfaceStyle:(UIUserInterfaceStyle)userInterfaceStyle API_AVAILABLE(ios(12.0))
{
  NSString *colorSchema = nil;
  if (userInterfaceStyle == UIUserInterfaceStyleDark) {
    colorSchema = @"dark";
  } else if (userInterfaceStyle == UIUserInterfaceStyleLight) {
    colorSchema = @"light";
  }
  
  // change RN appearance
  RCTOverrideAppearancePreference(colorSchema);
}

@end
