import { DeviceEventEmitter, NativeModules, EventSubscription } from 'react-native';

const DevMenu = NativeModules.ExpoDevMenuInternal;

export type DevMenuAppInfoType = {
  appName: string;
  appVersion: string;
  appIcon?: string;
  hostUrl?: string;
  expoSdkVersion?: string;
};

export enum DevMenuItemEnum {
  ACTION = 1,
  GROUP = 2,
  SCREEN = 3,
  LINK = 4,
  SELECTION_LIST = 5,
}

type DevMenuItemBaseType<T extends DevMenuItemEnum> = {
  type: T;
};

export type DevMenuScreenType = DevMenuItemBaseType<DevMenuItemEnum.SCREEN> & {
  screenName: string;
  items: DevMenuItemAnyType[];
};

export type DevMenuItemActionType = DevMenuItemBaseType<DevMenuItemEnum.ACTION> & {
  actionId: string;

  isAvailable: boolean;
  isEnabled: boolean;
  label: string;
  detail?: string | null;
  glyphName?: string | null;

  keyCommand: DevMenuKeyCommand;
};

export type DevMenuItemLinkType = DevMenuItemBaseType<DevMenuItemEnum.LINK> & {
  target: string;
  label: string;
  glyphName?: string | null;
};

export type DevMenuItemGroupType = DevMenuItemBaseType<DevMenuItemEnum.GROUP> & {
  groupName: string | null;
  items: DevMenuItemAnyType[];
};

export type DevMenuSelectionListItemTag = {
  glyphName?: string | null;
  text: string;
};

export type DevMenuSelectionListItem = {
  title: string;
  warning?: string;
  isChecked: boolean;
  tags: DevMenuSelectionListItemTag[];
};

export type DevMenuSelectionListType = DevMenuItemBaseType<DevMenuItemEnum.SELECTION_LIST> & {
  items: DevMenuSelectionListItem[];
};

export type DevMenuItemAnyType =
  | DevMenuItemActionType
  | DevMenuItemGroupType
  | DevMenuItemLinkType
  | DevMenuSelectionListType;

export type DevMenuItemProps<ItemType = DevMenuItemAnyType> = {
  item: ItemType;
};

export type DevMenuSettingsType = Partial<{
  motionGestureEnabled: boolean;
  touchGestureEnabled: boolean;
  keyCommandsEnabled: boolean;
  showsAtLaunch: boolean;
}>;

export type DevMenuKeyCommand = null | {
  input: string;
  modifiers: DevMenuKeyCommandsEnum;
};

export enum DevMenuKeyCommandsEnum {
  CONTROL = 1 << 0,
  ALT = 1 << 1,
  COMMAND = 1 << 2,
  SHIFT = 1 << 3,
}

export const doesDeviceSupportKeyCommands = DevMenu.doesDeviceSupportKeyCommands;

export async function dispatchActionAsync(actionId: string): Promise<void> {
  return await DevMenu.dispatchActionAsync(actionId);
}

export function hideMenu(): void {
  DevMenu.hideMenu();
}

export function setOnboardingFinished(finished: boolean): void {
  DevMenu.setOnboardingFinished(finished);
}

export async function getSettingsAsync(): Promise<DevMenuSettingsType> {
  return await DevMenu.getSettingsAsync();
}

export async function setSettingsAsync(settings: DevMenuSettingsType) {
  return await DevMenu.setSettingsAsync(settings);
}

export function subscribeToCloseEvents(listener: () => void): EventSubscription {
  return DeviceEventEmitter.addListener('closeDevMenu', listener);
}

export async function loadFontsAsync(): Promise<void> {
  return await DevMenu.loadFontsAsync();
}

export function openDevMenuFromReactNative() {
  DevMenu.openDevMenuFromReactNative();
}

export async function onScreenChangeAsync(currentScreen: string | null): Promise<void> {
  return await DevMenu.onScreenChangeAsync(currentScreen);
}

export async function saveAsync(key: string, data: string): Promise<void> {
  return await DevMenu.saveAsync(key, data);
}

export async function getAsync(key: string): Promise<string | null> {
  return await DevMenu.getAsync(key);
}
