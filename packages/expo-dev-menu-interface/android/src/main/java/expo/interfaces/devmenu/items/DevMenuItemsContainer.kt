package expo.interfaces.devmenu.items

import java.util.*
import kotlin.collections.ArrayList

open class DevMenuItemsContainer : DevMenuDSLItemsContainerInterface {
  private val items: ArrayList<DevMenuScreenItem> = ArrayList()

  override fun getRootItems(): List<DevMenuScreenItem> {
    items.sortedBy { it.importance }
    return items.toList()
  }

  override fun getAllItems(): List<DevMenuScreenItem> {
    val result = LinkedList<DevMenuScreenItem>()

    items.forEach {
      result.add(it)

      if (it is DevMenuDSLItemsContainerInterface) {
        result.addAll(it.getAllItems())
      }
    }
    return result
  }

  override fun addItem(item: DevMenuScreenItem) {
    items.add(item)
  }

  override fun group(init: DevMenuGroup.() -> Unit) = addItem(DevMenuGroup(), init)

  override fun action(actionId: String, action: () -> Unit, init: DevMenuAction.() -> Unit) =
    addItem(DevMenuAction(actionId, action), init)

  override fun link(target: String, init: DevMenuLink.() -> Unit) = addItem(DevMenuLink(target), init)

  override fun selectionList(init: DevMenuSelectionList.() -> Unit) = addItem(DevMenuSelectionList(), init)

  override fun serializeItems() =
    getRootItems()
      .map { it.serialize() }
      .toTypedArray()

  private fun <T : DevMenuScreenItem> addItem(item: T, init: T.() -> Unit): T {
    item.init()
    addItem(item)
    return item
  }

  companion object {
    fun export(init: DevMenuDSLItemsContainerInterface.() -> Unit): DevMenuItemsContainer {
      val container = DevMenuItemsContainer()
      container.init()
      return container
    }

  }
}
