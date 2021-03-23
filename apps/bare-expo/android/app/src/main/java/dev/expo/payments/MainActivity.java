package dev.expo.payments;

import android.content.Intent;

import android.app.Activity;
import android.os.Bundle;

import com.facebook.react.ReactActivityDelegate;
import com.facebook.react.ReactRootView;
import com.swmansion.gesturehandler.react.RNGestureHandlerEnabledRootView;

import expo.modules.devlauncher.DevLauncherController;
import expo.modules.devmenu.react.DevMenuAwareReactActivity;

import expo.modules.splashscreen.singletons.SplashScreen;
import expo.modules.splashscreen.SplashScreenImageResizeMode;

public class MainActivity extends DevMenuAwareReactActivity {

  /**
   * Returns the name of the main component registered from JavaScript.
   * This is used to schedule rendering of the component.
   */
  @Override
  protected String getMainComponentName() {
    return "main";
  }

  @Override
  protected ReactActivityDelegate createReactActivityDelegate() {
    Activity activity = this;
    ReactActivityDelegate delegate = new ReactActivityDelegate(this, getMainComponentName()) {
      @Override
      protected ReactRootView createRootView() {
        return new RNGestureHandlerEnabledRootView(MainActivity.this);
      }

      @Override
      protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // SplashScreen.show(...) has to be called after super.onCreate(...)
        // Below line is handled by '@expo/configure-splash-screen' command and it's discouraged to modify it manually
        SplashScreen.show(activity, SplashScreenImageResizeMode.COVER, ReactRootView.class, false);
      }
    };

    if (MainApplication.USE_DEV_CLIENT) {
      return DevLauncherController.wrapReactActivityDelegate(this, () -> delegate);
    }

    return delegate;
  }

  @Override
  public void onNewIntent(Intent intent) {
    if (MainApplication.USE_DEV_CLIENT) {
      if (DevLauncherController.tryToHandleIntent(this, intent)) {
        return;
      }
    }
    super.onNewIntent(intent);
  }
}
