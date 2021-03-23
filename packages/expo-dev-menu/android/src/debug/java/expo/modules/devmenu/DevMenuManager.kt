package expo.modules.devmenu

import android.app.Activity
import android.app.Application
import android.content.Context
import android.content.Intent
import android.hardware.SensorManager
import android.os.Bundle
import android.util.Log
import android.view.KeyEvent
import android.view.MotionEvent
import com.facebook.react.ReactInstanceManager
import com.facebook.react.ReactNativeHost
import com.facebook.react.bridge.LifecycleEventListener
import com.facebook.react.bridge.ReactContext
import com.facebook.react.bridge.UiThreadUtil
import com.facebook.react.modules.core.DeviceEventManagerModule
import expo.interfaces.devmenu.DevMenuDelegateInterface
import expo.interfaces.devmenu.DevMenuExtensionInterface
import expo.interfaces.devmenu.DevMenuManagerInterface
import expo.interfaces.devmenu.DevMenuSettingsInterface
import expo.interfaces.devmenu.items.DevMenuAction
import expo.interfaces.devmenu.items.DevMenuItemsContainerInterface
import expo.interfaces.devmenu.items.DevMenuScreen
import expo.interfaces.devmenu.items.DevMenuScreenItem
import expo.interfaces.devmenu.items.KeyCommand
import expo.interfaces.devmenu.items.getItemsOfType
import expo.modules.devmenu.detectors.ShakeDetector
import expo.modules.devmenu.detectors.ThreeFingerLongPressDetector
import expo.modules.devmenu.modules.DevMenuSettings
import java.lang.ref.WeakReference

object DevMenuManager : DevMenuManagerInterface, LifecycleEventListener {
  private var shakeDetector: ShakeDetector? = null
  private var threeFingerLongPressDetector: ThreeFingerLongPressDetector? = null
  private var session: DevMenuSession? = null
  private var settings: DevMenuSettingsInterface? = null
  private var delegate: DevMenuDelegateInterface? = null
  private var shouldLaunchDevMenuOnStart: Boolean = false
  private lateinit var devMenuHost: DevMenuHost
  private var currentReactInstanceManager: WeakReference<ReactInstanceManager?> = WeakReference(null)
  private var currentScreenName: String? = null

  //region helpers

  private val delegateReactContext: ReactContext?
    get() = delegate?.reactInstanceManager()?.currentReactContext

  private val delegateActivity: Activity?
    get() = delegateReactContext?.currentActivity

  private val hostReactContext: ReactContext?
    get() = devMenuHost.reactInstanceManager.currentReactContext

  private val hostActivity: Activity?
    get() = hostReactContext?.currentActivity

  /**
   * Returns an collection of modules conforming to [DevMenuExtensionInterface].
   * Bridge may register multiple modules with the same name – in this case it returns only the one that overrides the others.
   */
  private val delegateExtensions: Collection<DevMenuExtensionInterface>
    get() {
      val catalystInstance = delegateReactContext?.catalystInstance ?: return emptyList()
      val uniqueExtensionNames = catalystInstance
        .nativeModules
        .filterIsInstance<DevMenuExtensionInterface>()
        .map { it.getName() }
        .toSet()

      return uniqueExtensionNames
        .map { extensionName ->
          catalystInstance.getNativeModule(extensionName) as DevMenuExtensionInterface
        }
    }

  private val cachedDevMenuScreens by KeyValueCachedProperty<ReactInstanceManager, List<DevMenuScreen>> {
    delegateExtensions
      .map { it.devMenuScreens() ?: emptyList() }
      .flatten()
  }

  private val delegateScreens: List<DevMenuScreen>
    get() {
      val delegateBridge = delegate?.reactInstanceManager() ?: return emptyList()
      return cachedDevMenuScreens[delegateBridge]
    }

  private val cachedDevMenuItems by KeyValueCachedProperty<ReactInstanceManager, List<DevMenuItemsContainerInterface>> {
    delegateExtensions
      .mapNotNull { it.devMenuItems() }
  }

  private val delegateMenuItemsContainers: List<DevMenuItemsContainerInterface>
    get() {
      val delegateBridge = delegate?.reactInstanceManager() ?: return emptyList()
      return cachedDevMenuItems[delegateBridge]
    }

  private val delegateRootMenuItems: List<DevMenuScreenItem>
    get() =
      delegateMenuItemsContainers
        .map { it.getRootItems() }
        .flatten()
        .sortedBy { -it.importance }

  private fun getActions(): List<DevMenuAction> {
    if (currentScreenName == null) {
      return delegateMenuItemsContainers
        .map {
          it.getItemsOfType<DevMenuAction>()
        }
        .flatten()
    }

    val screen = delegateScreens.find { it.screenName == currentScreenName } ?: return emptyList()
    return screen.getItemsOfType()
  }

  //endregion

  //region init

  @Suppress("UNCHECKED_CAST")
  private fun maybeInitDevMenuHost(application: Application) {
    if (!this::devMenuHost.isInitialized) {
      devMenuHost = DevMenuHost(application)
      UiThreadUtil.runOnUiThread {
        devMenuHost.reactInstanceManager.createReactContextInBackground()
      }
    }
  }

  private fun setUpReactInstanceManager(reactInstanceManager: ReactInstanceManager) {
    currentReactInstanceManager = WeakReference(reactInstanceManager)
    if (reactInstanceManager.currentReactContext == null) {
      reactInstanceManager.addReactInstanceEventListener(object : ReactInstanceManager.ReactInstanceEventListener {
        override fun onReactContextInitialized(context: ReactContext) {
          if (currentReactInstanceManager.get() === reactInstanceManager) {
            handleLoadedDelegateContext(context)
          }
          reactInstanceManager.removeReactInstanceEventListener(this)
        }
      })
    } else {
      handleLoadedDelegateContext(reactInstanceManager.currentReactContext!!)
    }
  }

  /**
   * Starts dev menu if wasn't initialized, prepares for opening menu at launch if needed and gets [DevMenuSettings].
   * We can't open dev menu here, cause then the app will crash - two react instance try to render.
   * So we wait until the [reactContext] activity will be ready.
   */
  private fun handleLoadedDelegateContext(reactContext: ReactContext) {
    Log.i(DEV_MENU_TAG, "Delegate's context was loaded.")

    maybeInitDevMenuHost(reactContext.currentActivity?.application
      ?: reactContext.applicationContext as Application)
    maybeStartDetectors(devMenuHost.getContext())

    settings = if (reactContext.hasNativeModule(DevMenuSettings::class.java)) {
      reactContext.getNativeModule(DevMenuSettings::class.java)
    } else {
      DevMenuDefaultSettings()
    }.also {
      shouldLaunchDevMenuOnStart = it.showsAtLaunch || !it.isOnboardingFinished
      if (shouldLaunchDevMenuOnStart) {
        reactContext.addLifecycleEventListener(this)
      }
    }
  }

  //endregion

  //region shake detector

  /**
   * Starts [ShakeDetector] and [ThreeFingerLongPressDetector] if they aren't running yet.
   */
  private fun maybeStartDetectors(context: Context) {
    if (shakeDetector == null) {
      shakeDetector = ShakeDetector(this::onShakeGesture).apply {
        start(context.getSystemService(Context.SENSOR_SERVICE) as SensorManager)
      }
    }

    if (threeFingerLongPressDetector == null) {
      threeFingerLongPressDetector = ThreeFingerLongPressDetector(this::onThreeFingerLongPress)
    }
  }

  /**
   * Handles shake gesture which simply toggles the dev menu.
   */
  private fun onShakeGesture() {
    if (settings?.motionGestureEnabled == true) {
      delegateActivity?.let {
        toggleMenu(it)
      }
    }
  }

  /**
   * Handles three finger long press which simply toggles the dev menu.
   */
  private fun onThreeFingerLongPress() {
    if (settings?.touchGestureEnabled == true) {
      delegateActivity?.let {
        toggleMenu(it)
      }
    }
  }

  //endregion

  //region delegate's LifecycleEventListener

  override fun onHostResume() {
    delegateReactContext?.removeLifecycleEventListener(this)

    if (shouldLaunchDevMenuOnStart) {
      shouldLaunchDevMenuOnStart = false
      delegateActivity?.let { activity ->
        activity.runOnUiThread {
          openMenu(activity)
        }
      }
    }
  }

  override fun onHostPause() = Unit

  override fun onHostDestroy() = Unit

  //endregion

  //region DevMenuManagerProtocol

  override fun openMenu(activity: Activity, screen: String?) {
    setCurrentScreen(null)
    session = DevMenuSession(
      initReactInstanceManager = delegate!!.reactInstanceManager(),
      initAppInfo = delegate!!.appInfo(),
      screen = screen
    )

    activity.startActivity(Intent(activity, DevMenuActivity::class.java))
  }

  /**
   * Sends an event to JS triggering the animation that collapses the dev menu.
   */
  override fun closeMenu() {
    hostReactContext
      ?.getJSModule(DeviceEventManagerModule.RCTDeviceEventEmitter::class.java)
      ?.emit("closeDevMenu", null)
  }

  override fun hideMenu() {
    setCurrentScreen(null)
    hostActivity?.finish()
  }

  override fun toggleMenu(activity: Activity) {
    if (hostActivity?.isDestroyed == false) {
      closeMenu()
    } else {
      openMenu(activity)
    }
  }

  override fun onTouchEvent(ev: MotionEvent?) {
    threeFingerLongPressDetector?.onTouchEvent(ev)
  }

  override fun onKeyEvent(keyCode: Int, event: KeyEvent): Boolean {
    if (keyCode == KeyEvent.KEYCODE_MENU) {
      delegateActivity?.let { openMenu(it) }
      return true
    }

    if (settings?.keyCommandsEnabled != true) {
      return false
    }

    val keyCommand = KeyCommand(
      code = keyCode,
      withShift = event.modifiers and KeyEvent.META_SHIFT_MASK > 0
    )
    return getActions()
      .find { it.keyCommand == keyCommand }
      ?.run {
        action()
        true
      } ?: false
  }

  override fun setDelegate(newDelegate: DevMenuDelegateInterface) {
    Log.i(DEV_MENU_TAG, "Set new dev-menu delegate: ${newDelegate.javaClass}")
    // removes event listener for old delegate
    delegateReactContext?.removeLifecycleEventListener(this)

    delegate = newDelegate.apply {
      setUpReactInstanceManager(this.reactInstanceManager())
    }
  }

  override fun initializeWithReactNativeHost(reactNativeHost: ReactNativeHost) {
    setDelegate(DevMenuDefaultDelegate(reactNativeHost))
  }

  override fun dispatchAction(actionId: String) {
    getActions()
      .find { it.actionId == actionId }
      ?.run { action() }
  }

  override fun serializedItems(): List<Bundle> = delegateRootMenuItems.map { it.serialize() }

  override fun serializedScreens(): List<Bundle> = delegateScreens.map { it.serialize() }

  override fun getSession(): DevMenuSession? = session

  override fun getSettings(): DevMenuSettingsInterface? = settings

  override fun getMenuHost() = devMenuHost

  override fun synchronizeDelegate() {
    val newReactInstanceManager = requireNotNull(delegate).reactInstanceManager()
    if (newReactInstanceManager != currentReactInstanceManager.get()) {
      setUpReactInstanceManager(newReactInstanceManager)
    }
  }

  override fun setCurrentScreen(screen: String?) {
    currentScreenName = screen
  }

  //endregion
}

