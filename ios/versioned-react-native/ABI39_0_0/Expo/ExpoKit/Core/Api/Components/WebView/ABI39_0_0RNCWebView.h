/**
 * Copyright (c) 2015-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#import <ABI39_0_0React/ABI39_0_0RCTView.h>
#import <ABI39_0_0React/ABI39_0_0RCTDefines.h>
#import <WebKit/WebKit.h>

@class ABI39_0_0RNCWebView;

@protocol ABI39_0_0RNCWebViewDelegate <NSObject>

- (BOOL)webView:(ABI39_0_0RNCWebView *_Nonnull)webView
   shouldStartLoadForRequest:(NSMutableDictionary<NSString *, id> *_Nonnull)request
   withCallback:(ABI39_0_0RCTDirectEventBlock _Nonnull)callback;

@end

@interface ABI39_0_0RNCWeakScriptMessageDelegate : NSObject<WKScriptMessageHandler>
@property (nonatomic, weak) id<WKScriptMessageHandler> scriptDelegate;
- (instancetype)initWithDelegate:(id<WKScriptMessageHandler>)scriptDelegate;
@end

@interface ABI39_0_0RNCWebView : ABI39_0_0RCTView

@property (nonatomic, strong) NSString *experienceId;
@property (nonatomic, weak) id<ABI39_0_0RNCWebViewDelegate> _Nullable delegate;
@property (nonatomic, copy) NSDictionary * _Nullable source;
@property (nonatomic, assign) BOOL messagingEnabled;
@property (nonatomic, copy) NSString * _Nullable injectedJavaScript;
@property (nonatomic, copy) NSString * _Nullable injectedJavaScriptBeforeContentLoaded;
@property (nonatomic, assign) BOOL injectedJavaScriptForMainFrameOnly;
@property (nonatomic, assign) BOOL injectedJavaScriptBeforeContentLoadedForMainFrameOnly;
@property (nonatomic, assign) BOOL scrollEnabled;
@property (nonatomic, assign) BOOL sharedCookiesEnabled;
@property (nonatomic, assign) BOOL pagingEnabled;
@property (nonatomic, assign) CGFloat decelerationRate;
@property (nonatomic, assign) BOOL allowsInlineMediaPlayback;
@property (nonatomic, assign) BOOL bounces;
@property (nonatomic, assign) BOOL mediaPlaybackRequiresUserAction;
#if WEBKIT_IOS_10_APIS_AVAILABLE
@property (nonatomic, assign) WKDataDetectorTypes dataDetectorTypes;
#endif
@property (nonatomic, assign) UIEdgeInsets contentInset;
@property (nonatomic, assign) BOOL automaticallyAdjustContentInsets;
@property (nonatomic, assign) BOOL keyboardDisplayRequiresUserAction;
@property (nonatomic, assign) BOOL hideKeyboardAccessoryView;
@property (nonatomic, assign) BOOL allowsBackForwardNavigationGestures;
@property (nonatomic, assign) BOOL incognito;
@property (nonatomic, assign) BOOL useSharedProcessPool;
@property (nonatomic, copy) NSString * _Nullable userAgent;
@property (nonatomic, copy) NSString * _Nullable applicationNameForUserAgent;
@property (nonatomic, assign) BOOL cacheEnabled;
@property (nonatomic, assign) BOOL javaScriptEnabled;
@property (nonatomic, assign) BOOL javaScriptCanOpenWindowsAutomatically;
@property (nonatomic, assign) BOOL allowFileAccessFromFileURLs;
@property (nonatomic, assign) BOOL allowsLinkPreview;
@property (nonatomic, assign) BOOL showsHorizontalScrollIndicator;
@property (nonatomic, assign) BOOL showsVerticalScrollIndicator;
@property (nonatomic, assign) BOOL directionalLockEnabled;
@property (nonatomic, assign) BOOL ignoreSilentHardwareSwitch;
@property (nonatomic, copy) NSString * _Nullable allowingReadAccessToURL;
@property (nonatomic, assign) BOOL pullToRefreshEnabled;
@property (nonatomic, weak) UIRefreshControl * refreshControl;

#if defined(__IPHONE_OS_VERSION_MAX_ALLOWED) && __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000 /* iOS 13 */
@property (nonatomic, assign) WKContentMode contentMode;
#endif

+ (void)setClientAuthenticationCredential:(nullable NSURLCredential*)credential;
+ (void)setCustomCertificatesForHost:(nullable NSDictionary *)certificates;
- (void)postMessage:(NSString *_Nullable)message;
- (void)injectJavaScript:(NSString *_Nullable)script;
- (void)goForward;
- (void)goBack;
- (void)reload;
- (void)stopLoading;
- (void)addPullToRefreshControl;
- (void)pullToRefresh:(UIRefreshControl *)refreshControl;

@end