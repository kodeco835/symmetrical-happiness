#import "ABI38_0_0RNSScreenStack.h"
#import "ABI38_0_0RNSScreen.h"
#import "ABI38_0_0RNSScreenStackHeaderConfig.h"

#import <ABI38_0_0React/ABI38_0_0RCTBridge.h>
#import <ABI38_0_0React/ABI38_0_0RCTUIManager.h>
#import <ABI38_0_0React/ABI38_0_0RCTUIManagerUtils.h>
#import <ABI38_0_0React/ABI38_0_0RCTShadowView.h>
#import <ABI38_0_0React/ABI38_0_0RCTRootContentView.h>
#import <ABI38_0_0React/ABI38_0_0RCTTouchHandler.h>

@interface ABI38_0_0RNSScreenStackView () <UINavigationControllerDelegate, UIAdaptivePresentationControllerDelegate, UIGestureRecognizerDelegate>

@property (nonatomic) NSMutableArray<UIViewController *> *presentedModals;
@property (nonatomic) BOOL updatingModals;
@property (nonatomic) BOOL scheduleModalsUpdate;

@end

@interface ABI38_0_0RNSScreenStackAnimator : NSObject <UIViewControllerAnimatedTransitioning>
- (instancetype)initWithOperation:(UINavigationControllerOperation)operation;
@end

@implementation ABI38_0_0RNSScreenStackView {
  UINavigationController *_controller;
  NSMutableArray<ABI38_0_0RNSScreenView *> *_ABI38_0_0ReactSubviews;
  __weak ABI38_0_0RNSScreenStackManager *_manager;
}

- (instancetype)initWithManager:(ABI38_0_0RNSScreenStackManager*)manager
{
  if (self = [super init]) {
    _manager = manager;
    _ABI38_0_0ReactSubviews = [NSMutableArray new];
    _presentedModals = [NSMutableArray new];
    _controller = [[UINavigationController alloc] init];
    _controller.delegate = self;

    // we have to initialize viewControllers with a non empty array for
    // largeTitle header to render in the opened state. If it is empty
    // the header will render in collapsed state which is perhaps a bug
    // in UIKit but ¯\_(ツ)_/¯
    [_controller setViewControllers:@[[UIViewController new]]];
  }
  return self;
}

- (UIViewController *)ABI38_0_0ReactViewController
{
  return _controller;
}

- (void)navigationController:(UINavigationController *)navigationController willShowViewController:(UIViewController *)viewController animated:(BOOL)animated
{
  UIView *view = viewController.view;
  ABI38_0_0RNSScreenStackHeaderConfig *config = nil;
  for (UIView *subview in view.ABI38_0_0ReactSubviews) {
    if ([subview isKindOfClass:[ABI38_0_0RNSScreenStackHeaderConfig class]]) {
      config = (ABI38_0_0RNSScreenStackHeaderConfig*) subview;
      break;
    }
  }
  [ABI38_0_0RNSScreenStackHeaderConfig willShowViewController:viewController animated:animated withConfig:config];
}

- (void)navigationController:(UINavigationController *)navigationController didShowViewController:(UIViewController *)viewController animated:(BOOL)animated
{
  if (self.onFinishTransitioning) {
    self.onFinishTransitioning(nil);
  }
}

- (void)presentationControllerDidDismiss:(UIPresentationController *)presentationController
{
  // We don't directly set presentation delegate but instead rely on the ScreenView's delegate to
  // forward certain calls to the container (Stack).
  UIView *screenView = presentationController.presentedViewController.view;
  if ([screenView isKindOfClass:[ABI38_0_0RNSScreenView class]]) {
    [_presentedModals removeObject:presentationController.presentedViewController];
    if (self.onFinishTransitioning) {
      // instead of directly triggering onFinishTransitioning this time we enqueue the event on the
      // main queue. We do that because onDismiss event is also enqueued and we want for the transition
      // finish event to arrive later than onDismiss (see ABI38_0_0RNSScreen#notifyDismiss)
      dispatch_async(dispatch_get_main_queue(), ^{
        if (self.onFinishTransitioning) {
          self.onFinishTransitioning(nil);
        }
      });
    }
  }
}

- (id<UIViewControllerAnimatedTransitioning>)navigationController:(UINavigationController *)navigationController animationControllerForOperation:(UINavigationControllerOperation)operation fromViewController:(UIViewController *)fromVC toViewController:(UIViewController *)toVC
{
  ABI38_0_0RNSScreenView *screen;
  if (operation == UINavigationControllerOperationPush) {
    screen = (ABI38_0_0RNSScreenView *) toVC.view;
  } else if (operation == UINavigationControllerOperationPop) {
    screen = (ABI38_0_0RNSScreenView *) fromVC.view;
  }
  if (screen != nil && (screen.stackAnimation == ABI38_0_0RNSScreenStackAnimationFade || screen.stackAnimation == ABI38_0_0RNSScreenStackAnimationNone)) {
    return  [[ABI38_0_0RNSScreenStackAnimator alloc] initWithOperation:operation];
  }
  return nil;
}

- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gestureRecognizer
{
  // cancel touches in parent, this is needed to cancel ABI38_0_0RN touch events. For example when Touchable
  // item is close to an edge and we start pulling from edge we want the Touchable to be cancelled.
  // Without the below code the Touchable will remain active (highlighted) for the duration of back
  // gesture and onPress may fire when we release the finger.
  UIView *parent = _controller.view;
  while (parent != nil && ![parent isKindOfClass:[ABI38_0_0RCTRootContentView class]]) parent = parent.superview;
  ABI38_0_0RCTRootContentView *rootView = (ABI38_0_0RCTRootContentView *)parent;
  [rootView.touchHandler cancel];

  ABI38_0_0RNSScreenView *topScreen = (ABI38_0_0RNSScreenView *)_controller.viewControllers.lastObject.view;

  return _controller.viewControllers.count > 1 && topScreen.gestureEnabled;
}

- (void)markChildUpdated
{
  // do nothing
}

- (void)didUpdateChildren
{
  // do nothing
}

- (void)insertABI38_0_0ReactSubview:(ABI38_0_0RNSScreenView *)subview atIndex:(NSInteger)atIndex
{
  if (![subview isKindOfClass:[ABI38_0_0RNSScreenView class]]) {
    ABI38_0_0RCTLogError(@"ScreenStack only accepts children of type Screen");
    return;
  }
  subview.ABI38_0_0ReactSuperview = self;
  [_ABI38_0_0ReactSubviews insertObject:subview atIndex:atIndex];
}

- (void)removeABI38_0_0ReactSubview:(ABI38_0_0RNSScreenView *)subview
{
  subview.ABI38_0_0ReactSuperview = nil;
  [_ABI38_0_0ReactSubviews removeObject:subview];
}

- (NSArray<UIView *> *)ABI38_0_0ReactSubviews
{
  return _ABI38_0_0ReactSubviews;
}

- (void)didUpdateABI38_0_0ReactSubviews
{
  // we need to wait until children have their layout set. At this point they don't have the layout
  // set yet, however the layout call is already enqueued on ui thread. Enqueuing update call on the
  // ui queue will guarantee that the update will run after layout.
  dispatch_async(dispatch_get_main_queue(), ^{
    [self updateContainer];
  });
}

- (void)didMoveToWindow
{
  [super didMoveToWindow];
  if (self.window) {
    // when stack is attached to a window we do two things:
    // 1) we run updateContainer – we do this because we want push view controllers to be installed
    // before the VC is mounted. If we do that after it is added to parent the push updates operations
    // are going to be blocked by UIKit.
    // 2) we add navigation VS to parent – this is needed for the VC lifecycle events to be dispatched
    // properly
    // 3) we again call updateContainer – this time we do this to open modal controllers. Modals
    // won't open in (1) because they require navigator to be added to parent. We handle that case
    // gracefully in setModalViewControllers and can retry opening at any point.
    [self updateContainer];
    [self ABI38_0_0ReactAddControllerToClosestParent:_controller];
    [self updateContainer];
  }
}

- (void)ABI38_0_0ReactAddControllerToClosestParent:(UIViewController *)controller
{
  if (!controller.parentViewController) {
    UIView *parentView = (UIView *)self.ABI38_0_0ReactSuperview;
    while (parentView) {
      if (parentView.ABI38_0_0ReactViewController) {
        [parentView.ABI38_0_0ReactViewController addChildViewController:controller];
        [self addSubview:controller.view];
        _controller.interactivePopGestureRecognizer.delegate = self;
        [controller didMoveToParentViewController:parentView.ABI38_0_0ReactViewController];
        // On iOS pre 12 we observed that `willShowViewController` delegate method does not always
        // get triggered when the navigation controller is instantiated. As the only thing we do in
        // that delegate method is ask nav header to update to the current state it does not hurt to
        // trigger that logic from here too such that we can be sure the header is properly updated.
        [self navigationController:_controller willShowViewController:_controller.topViewController animated:NO];
        break;
      }
      parentView = (UIView *)parentView.ABI38_0_0ReactSuperview;
    }
    return;
  }
}

- (void)setModalViewControllers:(NSArray<UIViewController *> *)controllers
{
  // prevent re-entry
  if (_updatingModals) {
    _scheduleModalsUpdate = YES;
    return;
  }

  // when there is no change we return immediately. This check is important because sometime we may
  // accidently trigger modal dismiss if we don't verify to run the below code only when an actual
  // change in the list of presented modal was made.
  if ([_presentedModals isEqualToArray:controllers]) {
    return;
  }

  // if view controller is not yet attached to window we skip updates now and run them when view
  // is attached
  if (self.window == nil && _presentedModals.lastObject.view.window == nil) {
    return;
  }

  _updatingModals = YES;

  NSMutableArray<UIViewController *> *newControllers = [NSMutableArray arrayWithArray:controllers];
  [newControllers removeObjectsInArray:_presentedModals];

  // find bottom-most controller that should stay on the stack for the duration of transition
  NSUInteger changeRootIndex = 0;
  UIViewController *changeRootController = _controller;
  for (NSUInteger i = 0; i < MIN(_presentedModals.count, controllers.count); i++) {
    if (_presentedModals[i] == controllers[i]) {
      changeRootController = controllers[i];
      changeRootIndex = i + 1;
    } else {
      break;
    }
  }

  // we verify that controllers added on top of changeRootIndex are all new. Unfortunately modal
  // VCs cannot be reshuffled (there are some visual glitches when we try to dismiss then show as
  // even non-animated dismissal has delay and updates the screen several times)
  for (NSUInteger i = changeRootIndex; i < controllers.count; i++) {
    if ([_presentedModals containsObject:controllers[i]]) {
      ABI38_0_0RCTAssert(false, @"Modally presented controllers are being reshuffled, this is not allowed");
    }
  }

  __weak ABI38_0_0RNSScreenStackView *weakSelf = self;

  void (^afterTransitions)(void) = ^{
    if (weakSelf.onFinishTransitioning) {
      weakSelf.onFinishTransitioning(nil);
    }
    weakSelf.updatingModals = NO;
    if (weakSelf.scheduleModalsUpdate) {
      // if modals update was requested during setModalViewControllers we set scheduleModalsUpdate
      // flag in order to perform updates at a later point. Here we are done with all modals
      // transitions and check this flag again. If it was set, we reset the flag and execute updates.
      weakSelf.scheduleModalsUpdate = NO;
      [weakSelf updateContainer];
    }
  };

  void (^finish)(void) = ^{
    NSUInteger oldCount = weakSelf.presentedModals.count;
    if (changeRootIndex < oldCount) {
      [weakSelf.presentedModals
       removeObjectsInRange:NSMakeRange(changeRootIndex, oldCount - changeRootIndex)];
    }
    BOOL isAttached = changeRootController.parentViewController != nil || changeRootController.presentingViewController != nil;
    if (!isAttached || changeRootIndex >= controllers.count) {
      // if change controller view is not attached, presenting modals will silently fail on iOS.
      // In such a case we trigger controllers update from didMoveToWindow.
      // We also don't run any present transitions if changeRootIndex is greater or equal to the size
      // of new controllers array. This means that no new controllers should be presented.
      afterTransitions();
      return;
    } else {
      UIViewController *previous = changeRootController;
      for (NSUInteger i = changeRootIndex; i < controllers.count; i++) {
        UIViewController *next = controllers[i];
        BOOL lastModal = (i == controllers.count - 1);

#ifdef __IPHONE_13_0
        if (@available(iOS 13.0, *)) {
          // Inherit UI style from its parent - solves an issue with incorrect style being applied to some UIKit views like date picker or segmented control.
          next.overrideUserInterfaceStyle = _controller.overrideUserInterfaceStyle;
        }
#endif

        BOOL shouldAnimate = lastModal && [next isKindOfClass:[ABI38_0_0RNSScreen class]] && ((ABI38_0_0RNSScreenView *) next.view).stackAnimation != ABI38_0_0RNSScreenStackAnimationNone;

        [previous presentViewController:next
                               animated:shouldAnimate
                             completion:^{
          [weakSelf.presentedModals addObject:next];
          if (lastModal) {
            afterTransitions();
          };
        }];
        previous = next;
      }
    }
  };

  if (changeRootController.presentedViewController != nil
      && [_presentedModals containsObject:changeRootController.presentedViewController]) {

    BOOL shouldAnimate = changeRootIndex == controllers.count && [changeRootController.presentedViewController isKindOfClass:[ABI38_0_0RNSScreen class]] && ((ABI38_0_0RNSScreenView *) changeRootController.presentedViewController.view).stackAnimation != ABI38_0_0RNSScreenStackAnimationNone;
    [changeRootController
     dismissViewControllerAnimated:shouldAnimate
     completion:finish];
  } else {
    finish();
  }
}

- (void)setPushViewControllers:(NSArray<UIViewController *> *)controllers
{
  // when there is no change we return immediately
  if ([_controller.viewControllers isEqualToArray:controllers]) {
    return;
  }

  // if view controller is not yet attached to window we skip updates now and run them when view
  // is attached
  if (self.window == nil) {
    return;
  }
  // when transition is ongoing, any updates made to the controller will not be reflected until the
  // transition is complete. In particular, when we push/pop view controllers we expect viewControllers
  // property to be updated immediately. Based on that property we then calculate future updates.
  // When the transition is ongoing the property won't be updated immediatly. We therefore avoid
  // making any updated when transition is ongoing and schedule updates for when the transition
  // is complete.
  if (_controller.transitionCoordinator != nil) {
    __weak ABI38_0_0RNSScreenStackView *weakSelf = self;
    [_controller.transitionCoordinator animateAlongsideTransition:^(id<UIViewControllerTransitionCoordinatorContext>  _Nonnull context) {
      // do nothing here, we only want to be notified when transition is complete
    } completion:^(id<UIViewControllerTransitionCoordinatorContext>  _Nonnull context) {
      [weakSelf updateContainer];
    }];
    return;
  }

  UIViewController *top = controllers.lastObject;
  UIViewController *lastTop = _controller.viewControllers.lastObject;

  // at the start we set viewControllers to contain a single UIVIewController
  // instance. This is a workaround for header height adjustment bug (see comment
  // in the init function). Here, we need to detect if the initial empty
  // controller is still there
  BOOL firstTimePush = ![lastTop isKindOfClass:[ABI38_0_0RNSScreen class]];

  BOOL shouldAnimate = !firstTimePush && ((ABI38_0_0RNSScreenView *) lastTop.view).stackAnimation != ABI38_0_0RNSScreenStackAnimationNone;

  if (firstTimePush) {
    // nothing pushed yet
    [_controller setViewControllers:controllers animated:NO];
  } else if (top != lastTop) {
    if (![controllers containsObject:lastTop]) {
      // last top controller is no longer on stack
      // in this case we set the controllers stack to the new list with
      // added the last top element to it and perform (animated) pop
      NSMutableArray *newControllers = [NSMutableArray arrayWithArray:controllers];
      [newControllers addObject:lastTop];
      [_controller setViewControllers:newControllers animated:NO];
      [_controller popViewControllerAnimated:shouldAnimate];
    } else if (![_controller.viewControllers containsObject:top]) {
      // new top controller is not on the stack
      // in such case we update the stack except from the last element with
      // no animation and do animated push of the last item
      NSMutableArray *newControllers = [NSMutableArray arrayWithArray:controllers];
      [newControllers removeLastObject];
      [_controller setViewControllers:newControllers animated:NO];
      [_controller pushViewController:top animated:shouldAnimate];
    } else {
      // don't really know what this case could be, but may need to handle it
      // somehow
      [_controller setViewControllers:controllers animated:shouldAnimate];
    }
  } else {
    // change wasn't on the top of the stack. We don't need animation.
    [_controller setViewControllers:controllers animated:NO];
  }
}

- (void)updateContainer
{
  NSMutableArray<UIViewController *> *pushControllers = [NSMutableArray new];
  NSMutableArray<UIViewController *> *modalControllers = [NSMutableArray new];
  for (ABI38_0_0RNSScreenView *screen in _ABI38_0_0ReactSubviews) {
    if (!screen.dismissed && screen.controller != nil) {
      if (pushControllers.count == 0) {
        // first screen on the list needs to be places as "push controller"
        [pushControllers addObject:screen.controller];
      } else {
        if (screen.stackPresentation == ABI38_0_0RNSScreenStackPresentationPush) {
          [pushControllers addObject:screen.controller];
        } else {
          [modalControllers addObject:screen.controller];
        }
      }
    }
  }

  [self setPushViewControllers:pushControllers];
  [self setModalViewControllers:modalControllers];
}

- (void)layoutSubviews
{
  [super layoutSubviews];
  _controller.view.frame = self.bounds;
}

- (void)invalidate
{
  for (UIViewController *controller in _presentedModals) {
    [controller dismissViewControllerAnimated:NO completion:nil];
  }
  [_presentedModals removeAllObjects];
  [_controller willMoveToParentViewController:nil];
  [_controller removeFromParentViewController];
}

- (void)dismissOnReload
{
  dispatch_async(dispatch_get_main_queue(), ^{
    [self invalidate];
  });
}

@end

@implementation ABI38_0_0RNSScreenStackManager {
  NSPointerArray *_stacks;
}

ABI38_0_0RCT_EXPORT_MODULE()

ABI38_0_0RCT_EXPORT_VIEW_PROPERTY(onFinishTransitioning, ABI38_0_0RCTDirectEventBlock);

- (UIView *)view
{
  ABI38_0_0RNSScreenStackView *view = [[ABI38_0_0RNSScreenStackView alloc] initWithManager:self];
  if (!_stacks) {
    _stacks = [NSPointerArray weakObjectsPointerArray];
  }
  [_stacks addPointer:(__bridge void *)view];
  return view;
}

- (void)invalidate
{
 for (ABI38_0_0RNSScreenStackView *stack in _stacks) {
   [stack dismissOnReload];
 }
 _stacks = nil;
}

@end

@implementation ABI38_0_0RNSScreenStackAnimator {
  UINavigationControllerOperation _operation;
}

- (instancetype)initWithOperation:(UINavigationControllerOperation)operation
{
  if (self = [super init]) {
    _operation = operation;
  }
  return self;
}

- (NSTimeInterval)transitionDuration:(id <UIViewControllerContextTransitioning>)transitionContext
{
  ABI38_0_0RNSScreenView *screen;
  if (_operation == UINavigationControllerOperationPush) {
    UIViewController* toViewController = [transitionContext viewControllerForKey:UITransitionContextToViewControllerKey];
    screen = (ABI38_0_0RNSScreenView *)toViewController.view;
  } else if (_operation == UINavigationControllerOperationPop) {
    UIViewController* fromViewController = [transitionContext viewControllerForKey:UITransitionContextFromViewControllerKey];
    screen = (ABI38_0_0RNSScreenView *)fromViewController.view;
  }

  if (screen != nil && screen.stackAnimation == ABI38_0_0RNSScreenStackAnimationNone) {
    return 0;
  }
  return 0.35; // default duration
}

- (void)animateTransition:(id<UIViewControllerContextTransitioning>)transitionContext
{
  UIViewController* toViewController = [transitionContext viewControllerForKey:UITransitionContextToViewControllerKey];
  UIViewController* fromViewController = [transitionContext viewControllerForKey:UITransitionContextFromViewControllerKey];

  if (_operation == UINavigationControllerOperationPush) {
    [[transitionContext containerView] addSubview:toViewController.view];
    toViewController.view.alpha = 0.0;
    [UIView animateWithDuration:[self transitionDuration:transitionContext] animations:^{
      toViewController.view.alpha = 1.0;
    } completion:^(BOOL finished) {
      [transitionContext completeTransition:![transitionContext transitionWasCancelled]];
    }];
  } else if (_operation == UINavigationControllerOperationPop) {
    [[transitionContext containerView] insertSubview:toViewController.view belowSubview:fromViewController.view];

    [UIView animateWithDuration:[self transitionDuration:transitionContext] animations:^{
      fromViewController.view.alpha = 0.0;
    } completion:^(BOOL finished) {
      [transitionContext completeTransition:![transitionContext transitionWasCancelled]];
    }];
  }
}

@end
