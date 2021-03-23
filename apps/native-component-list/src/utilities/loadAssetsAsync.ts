import { Entypo, Ionicons, MaterialIcons } from '@expo/vector-icons';
import { Assets as StackAssets } from '@react-navigation/stack';
import { Asset } from 'expo-asset';
import * as Font from 'expo-font';
import { Platform } from 'react-native';

import Icons from '../constants/Icons';

async function loadAssetsAsync() {
  const iconRequires = Object.keys(Icons).map(key => Icons[key]);

  const assetPromises: Promise<any>[] = [
    Asset.loadAsync(iconRequires),
    Asset.loadAsync(StackAssets),
    // @ts-ignore
    Font.loadAsync(Ionicons.font),
    // @ts-ignore
    Font.loadAsync(Entypo.font),
    // @ts-ignore
    Font.loadAsync(MaterialIcons.font),
    Font.loadAsync({
      'space-mono': require('../../assets/fonts/SpaceMono-Regular.ttf'),
    }),
  ];
  if (Platform.OS !== 'web') {
    assetPromises.push(
      Font.loadAsync({
        Roboto: 'https://github.com/google/fonts/raw/d1a2e0f/ofl/roboto/static/Roboto-Regular.ttf',
      })
    );
  }
  await Promise.all(assetPromises);
}

let oncePromise: Promise<void>;
export default function loadAssetsAsyncOnce() {
  oncePromise = oncePromise || loadAssetsAsync();
  return oncePromise;
}
