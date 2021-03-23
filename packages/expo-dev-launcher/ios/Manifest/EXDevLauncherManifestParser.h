// Copyright 2015-present 650 Industries. All rights reserved.


NS_ASSUME_NONNULL_BEGIN
@class EXDevLauncherManifest;

typedef void (^OnManifestParsed)(EXDevLauncherManifest *manifest);
typedef void (^OnInvalidManifestURL)();
typedef void (^OnManifestError)(NSError *error);

@interface EXDevLauncherManifestParser : NSObject

- (instancetype)initWithURL:(NSURL *)url session:(NSURLSession *)session;

- (void)tryToParseManifest:(OnManifestParsed)onParsed onInvalidManifestURL:(OnInvalidManifestURL)onInalidURL onError:(OnManifestError)onError;

@end

NS_ASSUME_NONNULL_END
