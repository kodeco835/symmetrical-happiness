---
title: TypeScript
---

import TerminalBlock from '~/components/plugins/TerminalBlock';

> 💡 Example project: [with-typescript](https://github.com/expo/examples/tree/master/with-typescript)

Expo has first-class support for [TypeScript](https://www.typescriptlang.org/). The JavaScript interface of the Expo SDK is completely written in TypeScript.

To get started, create a `tsconfig.json` in your project root:

<TerminalBlock cmd={['touch tsconfig.json']} />

Running `expo start` will prompt you to install the required dependencies (`typescript`, `@types/react`, `@types/react-native`), and automatically configure your `tsconfig.json`.

Rename files to convert them to TypeScript. For example, you would rename `App.js` to `App.tsx`. Use the `.tsx` extension if the file includes React components (JSX). If the file did not include any JSX, you can use the `.ts` file extension.

<TerminalBlock cmd={['mv App.js App.tsx']} />

You can now run `yarn tsc` or `npx tsc` to typecheck the project.

## Base configuration

> 💡 You can disable the TypeScript setup in Expo CLI with the environment variable `EXPO_NO_TYPESCRIPT_SETUP=1`

An Expo app's `tsconfig.json` should extend the `expo/tsconfig.base` by default. This sets the following default [compiler options][tsc-compileroptions] (which can be overwritten in your project's `tsconfig.json`):

- [`jsx`][tsc-jsx]: -- `"react-native"`
  - Preserves JSX, and converts the extension `jsx` to `js`. This is optimized for bundlers that transform the JSX internally (like Metro).
- `allowJs`: -- `true`
  - Allow JavaScript files to be compiled. If you project requires more strictness, you can disable this.
- `resolveJsonModule`: -- `true`
  - Enables importing `.json` files. Metro's default behavior is to allow importing json files as JS objects.
- `noEmit`: -- `true`
  - Only typecheck, and skip generating transpiled code. Metro bundler is responsible for doing this.
- [`moduleResolution`][tsc-moduleresolution]: -- `"node"`
  - Emulates how Metro and Webpack resolve modules.
- `target`: -- `"esnext"`
  - The latest [TC39 proposed features](https://github.com/tc39/proposals).
- `lib`: -- `["dom", "es6", "es2016.array.include", "es2017.object"]`
  - List of library files to be included in the compilation.
- `skipLibCheck`: -- `true`
  - Skip type checking of all declaration files (`*.d.ts`).
- `allowSyntheticDefaultImports`: -- `true`
  - Allow default imports from modules with no default export. This does not affect code emit, just typechecking.
- `esModuleInterop`: -- `true`
  - Improves Babel ecosystem compatibility.

[tsc-jsx]: https://www.typescriptlang.org/docs/handbook/jsx.html
[tsc-compileroptions]: https://www.typescriptlang.org/docs/handbook/compiler-options.html
[tsc-moduleresolution]: https://www.typescriptlang.org/docs/handbook/module-resolution.html

## Project configuration

Expo CLI will automatically modify your `tsconfig.json` to the preferred default which is optimized for universal React development:

```json
{
  "extends": "expo/tsconfig.base",
  "compilerOptions": {}
}
```

The default configuration is forgiving and makes it easier to adopt TypeScript. If you'd like to opt-in to more strict type checking, you can add `"strict": true` to the `compilerOptions`. We recommend enabling this to minimize the chance of introducing runtime errors.

Certain language features may require additional configuration, for example if you'd like to use decorators you will need to add the `experimentalDecorators` option. For more information on the available properties see the [TypeScript compiler options documentation](https://www.typescriptlang.org/docs/handbook/compiler-options.html) documentation.

## Starting from scratch: using a TypeScript template

<TerminalBlock cmd={['expo init -t expo-template-blank-typescript']} />

The easiest way to get started is to initialize your new project using a TypeScript template. When you run `expo init` choose one of the templates with TypeScript in the name and then run `yarn tsc` or `npx tsc` to typecheck the project.

When you create new source files in your project you should use the `.ts` extension or the `.tsx` if the file includes React components.

## Learning how to use TypeScript

A good place to start learning TypeScript is the official [TypeScript Handbook](https://www.typescriptlang.org/docs/handbook/basic-types.html).

### TypeScript and React components

We recommend reading over and referring to the [React TypeScript CheatSheet](https://github.com/typescript-cheatsheets/react-typescript-cheatsheet) to learn how to type your React components in a variety of common situations.

### Advanced types

If you would like to go deeper and learn how to create more expressive and powerful types, we recommend the [Advanced Static Types in TypeScript course](https://egghead.io/courses/advanced-static-types-in-typescript) (this requires an egghead.io subscription).
