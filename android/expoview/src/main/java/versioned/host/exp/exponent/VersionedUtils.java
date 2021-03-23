// Copyright 2015-present 650 Industries. All rights reserved.

package versioned.host.exp.exponent;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.provider.Settings;

import com.facebook.common.logging.FLog;
import com.facebook.react.ReactInstanceManager;
import com.facebook.react.ReactInstanceManagerBuilder;
import com.facebook.react.common.LifecycleState;
import com.facebook.react.common.ReactConstants;
import com.facebook.react.devsupport.HMRClient;
import com.facebook.react.packagerconnection.NotificationOnlyHandler;
import com.facebook.react.packagerconnection.RequestHandler;
import com.facebook.react.shell.MainReactPackage;

import org.json.JSONObject;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import androidx.annotation.Nullable;
import host.exp.exponent.RNObject;
import host.exp.exponent.experience.ExperienceActivity;
import host.exp.exponent.experience.ReactNativeActivity;
import host.exp.expoview.Exponent;
import versioned.host.exp.exponent.modules.api.reanimated.ReanimatedJSIModulePackage;

public class VersionedUtils {

  private static void toggleExpoDevMenu() {
    Activity currentActivity = Exponent.getInstance().getCurrentActivity();
    if (currentActivity instanceof ExperienceActivity) {
      ExperienceActivity reactNativeActivity = (ExperienceActivity) currentActivity;
      reactNativeActivity.toggleDevMenu();
    } else {
      FLog.e(ReactConstants.TAG, "Unable to toggle the Expo dev menu because the current activity could not be found.");
    }
  }

  private static void reloadExpoApp() {
    Activity currentActivity = Exponent.getInstance().getCurrentActivity();
    if (currentActivity instanceof ReactNativeActivity) {
      ReactNativeActivity reactNativeActivity = (ReactNativeActivity) currentActivity;
      reactNativeActivity.getDevSupportManager().callRecursive("reloadExpoApp");
    } else {
      FLog.e(ReactConstants.TAG, "Unable to reload the app because the current activity could not be found.");
    }
  }

  private static void toggleElementInspector() {
    Activity currentActivity = Exponent.getInstance().getCurrentActivity();
    if (currentActivity instanceof ReactNativeActivity) {
      ReactNativeActivity reactNativeActivity = (ReactNativeActivity) currentActivity;
      reactNativeActivity.getDevSupportManager().callRecursive("toggleElementInspector");
    } else {
      FLog.e(ReactConstants.TAG, "Unable to toggle the element inspector because the current activity could not be found.");
    }
  }

  private static void requestOverlayPermission(Context context) {
    // From the unexposed DebugOverlayController static helper
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
      // Get permission to show debug overlay in dev builds.
      if (!Settings.canDrawOverlays(context)) {
        Intent intent =
          new Intent(
            Settings.ACTION_MANAGE_OVERLAY_PERMISSION,
            Uri.parse("package:" + context.getPackageName()));
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        FLog.w(
          ReactConstants.TAG,
          "Overlay permissions needs to be granted in order for React Native apps to run in development mode");
        if (intent.resolveActivity(context.getPackageManager()) != null) {
          context.startActivity(intent);
        }
      }
    }
  }

  private static void togglePerformanceMonitor() {
    Activity currentActivity = Exponent.getInstance().getCurrentActivity();
    if (currentActivity instanceof ReactNativeActivity) {
      ReactNativeActivity reactNativeActivity = (ReactNativeActivity) currentActivity;
      RNObject devSettings = reactNativeActivity.getDevSupportManager().callRecursive("getDevSettings");

      if (devSettings != null) {
        boolean isFpsDebugEnabled = (boolean) devSettings.call("isFpsDebugEnabled");
        if (!isFpsDebugEnabled) {
          // Request overlay permission if needed when "Show Perf Monitor" option is selected
          requestOverlayPermission(currentActivity);
        }
        devSettings.call("setFpsDebugEnabled", !isFpsDebugEnabled);
      }
    } else {
      FLog.e(ReactConstants.TAG, "Unable to toggle the performance monitor because the current activity could not be found.");
    }
  }

  private static void toggleRemoteJSDebugging() {
    Activity currentActivity = Exponent.getInstance().getCurrentActivity();
    if (currentActivity instanceof ReactNativeActivity) {
      ReactNativeActivity reactNativeActivity = (ReactNativeActivity) currentActivity;
      RNObject devSettings = reactNativeActivity.getDevSupportManager().callRecursive("getDevSettings");
      if (devSettings != null) {
        boolean isRemoteJSDebugEnabled = (boolean) devSettings.call("isRemoteJSDebugEnabled");
        devSettings.call("setRemoteJSDebugEnabled", !isRemoteJSDebugEnabled);
      }
    } else {
      FLog.e(ReactConstants.TAG, "Unable to toggle remote JS debugging because the current activity could not be found.");
    }
  }

  private static Map<String, RequestHandler> createPackagerCommandHelpers() {
    // Attach listeners to the bundler's dev server web socket connection.
    // This enables tools to automatically reload the client remotely (i.e. in expo-cli).
    Map<String, RequestHandler> packagerCommandHandlers = new HashMap<>();

    // Enable a lot of tools under the same command namespace
    packagerCommandHandlers.put("sendDevCommand", new NotificationOnlyHandler() {
      @Override
      public void onNotification(@Nullable Object params) {
        if (params != null && params instanceof JSONObject) {
          JSONObject _params = (JSONObject) params;
          String name = _params.optString("name");
          if (name != null) {
            if (name.equals("reload")) {
              reloadExpoApp();
            } else if (name.equals("toggleDevMenu")) {
              toggleExpoDevMenu();
            } else if (name.equals("toggleRemoteDebugging")) {
              toggleRemoteJSDebugging();
              // Reload the app after toggling debugging, this is based on what we do in DevSupportManagerBase.
              reloadExpoApp();
            } else if (name.equals("toggleElementInspector")) {
              toggleElementInspector();
            } else if (name.equals("togglePerformanceMonitor")) {
              togglePerformanceMonitor();
            }
          }
        }
      }
    });

    // These commands (reload and devMenu) are here to match RN dev tooling.

    // Reload the app on "reload"
    packagerCommandHandlers.put("reload", new NotificationOnlyHandler() {
      @Override
      public void onNotification(@Nullable Object params) {
        reloadExpoApp();
      }
    });

    // Open the dev menu on "devMenu"
    packagerCommandHandlers.put("devMenu", new NotificationOnlyHandler() {
      @Override
      public void onNotification(@Nullable Object params) {
        toggleExpoDevMenu();
      }
    });

    return packagerCommandHandlers;
  }

  public static ReactInstanceManagerBuilder getReactInstanceManagerBuilder(Exponent.InstanceManagerBuilderProperties instanceManagerBuilderProperties) {

    // Build the instance manager
    ReactInstanceManagerBuilder builder = ReactInstanceManager.builder()
        .setApplication(instanceManagerBuilderProperties.application)
        .setJSIModulesPackage((reactApplicationContext, jsContext) -> {
          Activity currentActivity = Exponent.getInstance().getCurrentActivity();
          if (currentActivity instanceof ReactNativeActivity) {
            ReactNativeActivity reactNativeActivity = (ReactNativeActivity) currentActivity;
            RNObject devSettings = reactNativeActivity.getDevSupportManager().callRecursive("getDevSettings");
            boolean isRemoteJSDebugEnabled = devSettings != null && (boolean) devSettings.call("isRemoteJSDebugEnabled");
            if (!isRemoteJSDebugEnabled) {
              return new ReanimatedJSIModulePackage().getJSIModules(reactApplicationContext, jsContext);
            }
          }
          return Collections.emptyList();
        })
        .addPackage(new MainReactPackage())
        .addPackage(new ExponentPackage(
                instanceManagerBuilderProperties.experienceProperties,
                instanceManagerBuilderProperties.manifest,
                // DO NOT EDIT THIS COMMENT - used by versioning scripts
                // When distributing change the following two arguments to nulls
                instanceManagerBuilderProperties.expoPackages,
                instanceManagerBuilderProperties.exponentPackageDelegate,
                instanceManagerBuilderProperties.singletonModules))
        .addPackage(new ExpoTurboPackage(
          instanceManagerBuilderProperties.experienceProperties,
          instanceManagerBuilderProperties.manifest))
        .setInitialLifecycleState(LifecycleState.BEFORE_CREATE)
        .setCustomPackagerCommandHandlers(createPackagerCommandHelpers());

    if (instanceManagerBuilderProperties.jsBundlePath != null && instanceManagerBuilderProperties.jsBundlePath.length() > 0) {
      builder = builder.setJSBundleFile(instanceManagerBuilderProperties.jsBundlePath);
    }

    return builder;
  }

}