---
title: Using Expo Go in Bare Workflow
sidebar_title: Using Expo Go
---

It's not currently possible to include your own native code in Expo Go, so it might surprise you to learn that it is still possible to run your bare project in the Expo Go app.

Inside a freshly initialized bare project, run `expo start` and you can now run it in the client. Read on to learn more about the limitations, why you might want to still use the client in spite of the limitations, and patterns you can apply to make this work well for you.

## What are the limitations?

You will not be able to use the parts of your app that require custom native code. To run your bare app in Expo Go, you need to avoid calling any custom native code (native code that isn't included in the Expo SDK). For some apps this may mean that you won't be able to use the Expo Go app almost at all &mdash; for example, if your app depends on custom native code for something as fundamental as navigation or state management (eg: Realm or the Firebase native SDK) then not much of your app will be usable in the client. If your app only has some in app purchases, analytics, a custom map view, an AR view, and so on, then this may actually work great for you &mdash; that particular functionality would not be usable in Expo Go but the rest of the app still would be.

## Why might you want to do this?

There are a number of benefits to keeping your project runnable in the Expo Go app.

- Share your progress with stakeholders by publishing or sharing the development URL to see changes live
- Continuously deploy builds from pull requests
- No need to do native builds for iOS and Android in development because you use the Expo Go app instead
- Develop the JavaScript side of your app from any machine of your choice, eg: use Windows for iOS development if you have an iOS device
- Easily get new contributors set up on the project, only Node.js and a phone are required
- You can use `expo-cli` for a great development experience

## Practical patterns for client-compatible bare apps

### Prefer `expo install` over `npm install` to add Expo SDK packages

This will ensure that you get a version of the package that is compatible with the SDK version in your app. If you use `npm install` directly instead, you may end up with a newer version of the package that isn't supported in Expo Go yet.

### Use the `.expo.[js/json/ts/tsx]` extension to provide Expo Go specific fallbacks

> ⚠️ The `.expo` extension will be removed in SDK 41. [Learn more about alternatives](https://github.com/expo/fyi/blob/master/expo-extension-migration.md).

Picture this: you need a beautiful map in your app and Google Maps just won't cut it, so you add '@mapbox/react-native-mapbox-gl'. Expo doesn't include this in the SDK, so you can't run any code that imports it in the Expo Go app. You can handle this by making `MyMap.js` and `MyMap.expo.js` as follows:

```js
// MyMap.js
import * as React from 'react';
import Mapbox from '@mapbox/react-native-mapbox-gl';

export default class MyMap extends React.Component {
  render() {
    return (
      <Mapbox.MapView
        styleURL="mapbox://styles/jhuskey/cjabpqolp3lf02so534xe4q9g"
        style={{ flex: 1 }}
        {...this.props}
      />
    );
  }
}
```

```js
// MyMap.expo.js
import * as React from 'react';
import { Text, View } from 'react-native';

export default class MyMap extends React.Component {
  render() {
    return (
      <View
        style={{
          flex: 1,
          backgroundColor: '#000',
          alignItems: 'center',
          justifyContent: 'center',
        }}>
        <Text style={{ color: '#fff' }}>Mapbox map not available!</Text>
      </View>
    );
  }
}
```

```js
// App.js
import * as React from 'react';
import MyMap from './MyMap';

export default () => <MyMap />;
```

> **Note**: Sometimes the React Native JavaScript bundler, [Metro](https://github.com/facebook/metro), doesn't pick up on file extension changes as quickly as you may hope and you will end up with a red screen error. If you encounter this, you can remove the `MyMap` import (eg: from App.js in the above example), then reload the app, and finally re-add the import and reload again. Alternatively, you can also close and re-open `expo-cli`.

Problem solved! Now when we render the `<MyMap />` component in the client, we'll fallback to a plain `View`. When we build the app in Xcode/Android Studio, we'll use the Mapbox map. You could also alternatively fallback to a map provided from `react-native-maps`, since it's included in the Expo SDK.

As you may have already guessed, you can apply this concept for more than just third party dependencies. For example, maybe you wrote a native module to wrap your favorite analytics library and you want to provide a mock for it within the client.

```js
// MyAnalytics.js
import { NativeModules } from 'react-native';

export function logEvent(options) {
  NativeModules.MyAnalytics.logEvent(options);
}

export function setUser(options) {
  NativeModules.MyAnalytics.setUser(options);
}
```

```js
// MyAnalytics.expo.js
export function logEvent(options) {
  console.log(`Called MyAnalytics.logEvent with: ${JSON.stringify(options)}`);
}

export function setUser(user) {
  console.log(`Called MyAnalytics.setUser with: ${JSON.stringify(user)}`);
}
```

### Conditional inline requires

It may occasionally make for more self-descriptive code to explicitly switch out a module based on the environment. Let's use the map example from above to demonstrate how you could do this.

```js
import * as React from 'react';
import Constants from 'expo-constants';

let MyMap;
if (Constants.appOwnership === 'expo') {
  MyMap = <View />;
} else {
  MyMap = require('./MyMap').default;
}

export default class MapScreen extends React.Component {
  render() {
    return (
      <View style={{ height: 300, flex: 1 }}>
        <MyMap rotateEnabled={false} pinchEnabled={false} />
      </View>
    );
  }
}
```

By moving the `require` directive inline we only actually execute the `MyMap` module code when we enter the `else` clause, and so we prevent ever importing the `@mapbox/react-native-mapbox-gl` package, which would likely throw an error due to the native module being missing in the client runtime environment.

### Avoid importing the `expo` package in bare apps

Using the approaches above, you should avoid importing the `expo` package in bare environments. If you import anything from the `expo` package it will run code that assumes you are within the client runtime environment and throw an error.
