package expo.modules.devlauncher.launcher

import android.app.Application
import com.facebook.react.ReactNativeHost
import com.facebook.react.shell.MainReactPackage
import expo.modules.devlauncher.DevLauncherPackage
import expo.modules.devlauncher.R
import expo.modules.devlauncher.helpers.findDevMenuPackage
import expo.modules.devlauncher.helpers.injectDebugServerHost
import org.apache.commons.io.IOUtils
import java.io.File
import java.io.FileOutputStream
import java.io.IOException

class DevLauncherClientHost(
  application: Application,
  private val launcherIp: String?
) : ReactNativeHost(application) {

  init {
    if (useDeveloperSupport) {
      injectDebugServerHost(application.applicationContext, this, launcherIp!!)
    }
  }

  override fun getUseDeveloperSupport() = launcherIp != null

  override fun getPackages() = listOfNotNull(
    MainReactPackage(null),
    DevLauncherPackage(),
    findDevMenuPackage()
  )

  override fun getJSMainModuleName() = "index"
  
  override fun getJSBundleFile(): String? {
    if (useDeveloperSupport) {
      return null
    }
    // React Native needs an actual file path, while the embedded bundle is a 'raw resource' which
    // doesn't have a true file path. So we write it out to a temporary file then return a path
    // to that file.
    val bundle = File(application.cacheDir.absolutePath + "/expo_dev_launcher_android.bundle")
    return try {
      // TODO(nikki): We could cache this? Biasing toward always using latest for now...
      val output = FileOutputStream(bundle)
      val input = application.resources.openRawResource(R.raw.expo_dev_launcher_android)
      IOUtils.copy(input, output)
      output.close()
      bundle.absolutePath
    } catch (e: IOException) {
      null
    }
  }
}
