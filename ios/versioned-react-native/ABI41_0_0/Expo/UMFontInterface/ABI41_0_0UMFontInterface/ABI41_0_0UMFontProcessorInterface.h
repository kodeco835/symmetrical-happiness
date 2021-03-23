// Copyright 2018-present 650 Industries. All rights reserved.

#import <UIKit/UIKit.h>

@protocol ABI41_0_0UMFontProcessorInterface

- (UIFont *)updateFont:(UIFont *)uiFont
              withFamily:(NSString *)family
                    size:(NSNumber *)size
                  weight:(NSString *)weight
                   style:(NSString *)style
                 variant:(NSArray<NSDictionary *> *)variant
         scaleMultiplier:(CGFloat)scaleMultiplier;

@end