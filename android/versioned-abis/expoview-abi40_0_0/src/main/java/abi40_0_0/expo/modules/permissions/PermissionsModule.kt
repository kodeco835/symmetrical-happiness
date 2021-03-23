// Copyright 2015-present 650 Industries. All rights reserved.

package abi40_0_0.expo.modules.permissions

import android.Manifest
import android.content.Context
import android.os.Bundle
import abi40_0_0.expo.modules.permissions.requesters.LocationRequester
import abi40_0_0.expo.modules.permissions.requesters.NotificationRequester
import abi40_0_0.expo.modules.permissions.requesters.PermissionRequester
import abi40_0_0.expo.modules.permissions.requesters.RemindersRequester
import abi40_0_0.expo.modules.permissions.requesters.SimpleRequester
import abi40_0_0.org.unimodules.core.ExportedModule
import abi40_0_0.org.unimodules.core.ModuleRegistry
import abi40_0_0.org.unimodules.core.Promise
import abi40_0_0.org.unimodules.core.interfaces.ExpoMethod
import abi40_0_0.org.unimodules.interfaces.permissions.Permissions
import abi40_0_0.org.unimodules.interfaces.permissions.PermissionsResponse
import abi40_0_0.org.unimodules.interfaces.permissions.PermissionsResponseListener

class PermissionsModule(context: Context) : ExportedModule(context) {
  private lateinit var mPermissions: Permissions
  private lateinit var mRequesters: Map<String, PermissionRequester>

  @Throws(IllegalStateException::class)
  override fun onCreate(moduleRegistry: ModuleRegistry) {
    mPermissions = moduleRegistry.getModule(Permissions::class.java)
      ?: throw IllegalStateException("Couldn't find implementation for Permissions interface.")

    val notificationRequester = NotificationRequester(context)
    val contactsRequester = if (mPermissions.isPermissionPresentInManifest(Manifest.permission.WRITE_CONTACTS)) {
      SimpleRequester(Manifest.permission.WRITE_CONTACTS, Manifest.permission.READ_CONTACTS)
    } else {
      SimpleRequester(Manifest.permission.READ_CONTACTS)
    }
    mRequesters = mapOf(
      PermissionsTypes.LOCATION.type to LocationRequester(mPermissions.isPermissionPresentInManifest(Manifest.permission.ACCESS_BACKGROUND_LOCATION)),
      PermissionsTypes.CAMERA.type to SimpleRequester(Manifest.permission.CAMERA),
      PermissionsTypes.CONTACTS.type to contactsRequester,
      PermissionsTypes.AUDIO_RECORDING.type to SimpleRequester(Manifest.permission.RECORD_AUDIO),
      PermissionsTypes.MEDIA_LIBRARY_WRITE_ONLY.type to SimpleRequester(Manifest.permission.WRITE_EXTERNAL_STORAGE),
      PermissionsTypes.MEDIA_LIBRARY.type to SimpleRequester(Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE),
      PermissionsTypes.CALENDAR.type to SimpleRequester(Manifest.permission.READ_CALENDAR, Manifest.permission.WRITE_CALENDAR),
      PermissionsTypes.SMS.type to SimpleRequester(Manifest.permission.READ_SMS),
      PermissionsTypes.NOTIFICATIONS.type to notificationRequester,
      PermissionsTypes.USER_FACING_NOTIFICATIONS.type to notificationRequester,
      PermissionsTypes.SYSTEM_BRIGHTNESS.type to SimpleRequester(Manifest.permission.WRITE_SETTINGS),
      PermissionsTypes.REMINDERS.type to RemindersRequester()
    )
  }

  @Throws(IllegalStateException::class)
  private fun getRequester(permissionType: String): PermissionRequester {
    return mRequesters[permissionType]
      ?: throw IllegalStateException("Unrecognized permission type: $permissionType")
  }

  override fun getName(): String = "ExpoPermissions"

  @ExpoMethod
  fun getAsync(requestedPermissionsTypes: ArrayList<String>, promise: Promise) {
    try {
      delegateToPermissionsServiceIfNeeded(requestedPermissionsTypes, mPermissions::getPermissions, promise)
    } catch (e: IllegalStateException) {
      promise.reject(ERROR_TAG + "_UNKNOWN", "Failed to get permissions", e)
    }
  }

  @ExpoMethod
  fun askAsync(requestedPermissionsTypes: ArrayList<String>, promise: Promise) {
    try {
      delegateToPermissionsServiceIfNeeded(requestedPermissionsTypes, mPermissions::askForPermissions, promise)
    } catch (e: IllegalStateException) {
      promise.reject(ERROR_TAG + "_UNKNOWN", "Failed to get permissions", e)
    }
  }

  private fun createPermissionsResponseListener(requestedPermissionsTypes: ArrayList<String>, promise: Promise) =
    PermissionsResponseListener { permissionsNativeStatus ->
      promise.resolve(parsePermissionsResponse(requestedPermissionsTypes, permissionsNativeStatus))
    }

  private fun delegateToPermissionsServiceIfNeeded(permissionTypes: ArrayList<String>, permissionsServiceDelegate: (PermissionsResponseListener, Array<out String>) -> Unit, promise: Promise) {
    val androidPermissions = getAndroidPermissionsFromList(permissionTypes)
    // Some permissions like `NOTIFICATIONS` or `USER_FACING_NOTIFICATIONS` aren't supported by the android.
    // So, if the user asks/gets those permissions, we can return status immediately.
    if (androidPermissions.isEmpty()) {
      // We pass an empty map here cause those permissions don't depend on the system result.
      promise.resolve(parsePermissionsResponse(permissionTypes, emptyMap()))
      return;
    }

    permissionsServiceDelegate(createPermissionsResponseListener(permissionTypes, promise), androidPermissions)
  }

  @Throws(IllegalStateException::class)
  private fun parsePermissionsResponse(requestedPermissionsTypes: List<String>, permissionMap: Map<String, PermissionsResponse>): Bundle {
    return Bundle().apply {
      requestedPermissionsTypes.forEach {
        putBundle(it, getRequester(it).parseAndroidPermissions(permissionMap))
      }
    }
  }

  @Throws(IllegalStateException::class)
  private fun getAndroidPermissionsFromList(requestedPermissionsTypes: List<String>): Array<String> {
    return requestedPermissionsTypes
      .map { getRequester(it).getAndroidPermissions() }
      .reduce { acc, list -> acc + list }
      .toTypedArray()
  }
}