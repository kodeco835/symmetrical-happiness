#import <React/RCTBridgeModule.h>
#import <React/RCTBridgeDelegate.h>

#import <UIKit/UIKit.h>

@class EXDevLauncherPendingDeepLinkRegistry;
@class EXDevLauncherController;
@class EXDevLauncherManifest;

@protocol EXDevLauncherControllerDelegate <NSObject>

- (void)devLauncherController:(EXDevLauncherController *)developmentClientController
                didStartWithSuccess:(BOOL)success;

@end

@interface EXDevLauncherController : NSObject <RCTBridgeDelegate>

@property (nonatomic, weak) RCTBridge *appBridge;
@property (nonatomic, strong) EXDevLauncherPendingDeepLinkRegistry *pendingDeepLinkRegistry;

+ (instancetype)sharedInstance;

- (void)startWithWindow:(UIWindow *)window delegate:(id<EXDevLauncherControllerDelegate>)delegate launchOptions:(NSDictionary *)launchOptions;

- (NSURL *)sourceUrl;

- (void)navigateToLauncher;

- (BOOL)onDeepLink:(NSURL *)url options:(NSDictionary *)options;

- (void)loadApp:(NSURL *)url onSuccess:(void (^)())onSuccess onError:(void (^)(NSError *error))onError;

- (NSDictionary *)recentlyOpenedApps;

- (NSDictionary<UIApplicationLaunchOptionsKey, NSObject*> *)getLaunchOptions;

- (EXDevLauncherManifest *)appManifest;

@end
