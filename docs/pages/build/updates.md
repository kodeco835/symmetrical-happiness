---
title: Over-the-air updates
---

> We're currently working on EAS Update, a service that integrates deeply with other EAS services and builds on top of `expo-updates` to provide more power, flexibility, and tools to help you work better with your team.

EAS Build includes some special affordances for Expo's over-the-air updates library, [`expo-updates`](/versions/latest/sdk/updates.md). In particular, you can configure the release channel in `eas.json` and EAS Build will take care of updating it in your native project at build time. Not sure what a release channel is? [Learn more about release channels](/distribution/release-channels.md).

## Setting the release channel for a build profile

Each [build profile](./eas-json.md#build-profiles) can be assigned to a release channel, so updates for builds produced for a given profile will pull only those releases that are published to its release channel. If a release channel is not specified, the value will be `"default"`.

The following example demonstrates how you might use the `"production"` release channel for release builds, and the `"staging"` release channel for test builds distributed with [internal distribution](internal-distribution.md).

```json
{
  "builds": {
    "android": {
      "release": {
        "workflow": "generic",
        "releaseChannel": "production"
      },
      "team": {
        "workflow": "generic",
        "releaseChannel": "staging",
        "distribution": "internal"
      }
    },
    "ios": {
      "release": {
        "workflow": "generic",
        "releaseChannel": "production"
      },
      "team": {
        "workflow": "generic",
        "releaseChannel": "staging",
        "distribution": "internal"
      }
    }
  }
}
```

## Binary compatibility and other usage concerns

Your native runtime may change on each build, depending on whether you modify the code in a way that changes the API contract with JavaScript. If you publish a JavaScript bundle to a binary with an incompatible native runtime (for example, a function that the JavaScript bundle expects to exist does not exist) then your app may not work as expected or it may crash.

Please refer to the ["Updating your app over-the-air"](/bare/updating-your-app.md) guide to learn more about update compatibility and more.

## Updating managed apps built with EAS Build

Support for managed apps on EAS Build is still very early, and you may encounter unexpected issues. Please note that if you are publishing an update to a managed app built with EAS Build, you currently need to use the `--target bare` flag: `expo publish --release-channel your-channel --target bare`. If you do not, your app may crash.
