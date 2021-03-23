// Copyright 2015-present 650 Industries. All rights reserved.

@objc
open class DevMenuAction: DevMenuScreenItem {
  @objc
  public let actionId: String

  @objc
  public var action: () -> () = {}

  @objc
  public var keyCommand: UIKeyCommand?
  
  @objc
  open var isAvailable: () -> Bool = { true }

  @objc
  open var isEnabled: () -> Bool = { false }

  @objc
  open var label: () -> String = { "" }

  @objc
  open var detail: () -> String = { "" }

  @objc
  open var glyphName: () -> String = { "" }

  @objc
  public init(withId id: String) {
    self.actionId = id
    super.init(type: .Action)
  }

  @objc
  public convenience init(withId id: String, action: @escaping () -> ()) {
    self.init(withId: id)
    self.action = action
  }

  @objc
  open func registerKeyCommand(input: String, modifiers: UIKeyModifierFlags) {
    keyCommand = UIKeyCommand(input: input, modifierFlags: modifiers, action: #selector(DevMenuUIResponderExtensionProtocol.EXDevMenu_handleKeyCommand(_:)))
  }
  
  @objc
  open override func serialize() -> [String : Any] {
    var dict = super.serialize()
    dict["actionId"] = actionId
    dict["keyCommand"] = keyCommand == nil ? nil : [
      "input": keyCommand!.input,
      "modifiers": exportKeyCommandModifiers()
    ]
    
    dict["isAvailable"] = isAvailable()
    dict["isEnabled"] = isAvailable()
    dict["label"] = label()
    dict["detail"] = detail()
    dict["glyphName"] = glyphName()

    return dict
  }
  
  private func exportKeyCommandModifiers() -> Int {
    var exportedValue = 0;
    if keyCommand!.modifierFlags.contains(.control) {
      exportedValue += 1 << 0;
    }
    
    if keyCommand!.modifierFlags.contains(.alternate) {
      exportedValue += 1 << 1;
    }
    
    if keyCommand!.modifierFlags.contains(.command) {
      exportedValue += 1 << 2;
    }
    
    if keyCommand!.modifierFlags.contains(.shift) {
      exportedValue += 1 << 3;
    }
  
    return exportedValue
  }
}
