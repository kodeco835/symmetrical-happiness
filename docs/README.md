# Expo Documentation

This is the public documentation for **Expo**, its SDK, client and services.

You can access this documentation online at https://docs.expo.io/. It's built using next.js on top of the https://github.com/vercel/docs codebase.

> **Contributors:** Please make sure that you edit the docs in the `pages/versions/unversioned` directory if you want your changes to apply to the next SDK version too!

## Running Locally

Download the copy of this repository.

```sh
git clone https://github.com/expo/expo.git
```

Then `cd` into the `docs` directory and install dependencies with:

```sh
yarn
```

Then you can run the app with (make sure you have no server running on port `3002`):

```sh
yarn run dev
```

Now the documentation is running at http://localhost:3002

## Running in production mode

```sh
yarn run export
yarn run export-server
```

## Editing Docs Content

You can find the source of the documentation inside the `pages/versions` directory. Documentation is mostly written in markdown with the help of some React components (for Snack embeds, etc). The routes and navbar are automatically inferred from the directory structure within `versions`.

## Editing Code

The docs are written with Next.js and TypeScript. If you need to make code changes, follow steps from the [Running locally](#running-locally) section, then open a separate terminal and run the TypeScript compiler in watch mode - it will watch your code changes and notify you about errors.

```sh
yarn watch
```

When you are done, you should run _prettier_ to format your code. Also, don't forget to run tests and linter before committing your changes.

```sh
yarn prettier
yarn test
yarn lint
```

## Internal linking

If you need to link from one MDX file to another, please use the path-reference to this file including extension.
This allows us to automatically validate these links and see if the file and/or headers still exists.

- from: `tutorial/button.md`, to: `/workflow/guides/` -> `../workflow/guides.md`
- from: `index.md`, to: `/guides/errors/#tracking-js-errors` -> `./guides/errors.md#tracking-js-errors` (or without `./`)

You can validate all current links by running `$ yarn lint-links`.

## Redirects

### Server-side redirects

These redirects are limited in their expressiveness - you can map a path to another path, but no regular expressions or anything are supported. See client-side redirects for more of that. Server-side redirects are re-created on each run of `deploy.sh`.

We currently do two client-side redirects, using meta tags with `http-equiv="refresh"`:

- `/` -> `/versions/latest/`
- `/versions` -> `/versions/latest`

This method is not great for accessibility and should be avoided where possible.

### Client-side redirects

Use these for more complex rules than one-to-one path-to-path redirect mapping. For example, we use client-side redirects to strip the `.html` extension off, and to identify if the request is for a version of the documentation that we no longer support.

You can add your own client-side redirect rules in `pages/_error.js`.

## Adding Images and Assets

You can add images and assets to the `public/static` directory. They'll be served by the production and staging servers at `/static`.

## New Components

Always try to use the existing components and features in markdown. Create a new component or use a component from NPM, unless there is no other option.

## Algolia Docsearch

We use Algolia Docsearch as the search engine for our docs. Right now, it's searching for any keywords with the proper `version` tag based on the current location. This is set in the `components/DocumentationPage` header.

In `components/plugins/AlgoliaSearch`, you can see the `facetFilters` set to `[['version:none', 'version:{currentVersion}']]`. Translated to English, this means "Search on all pages where `version` is `none`, or the currently selected version.".

- All unversioned pages use the version tag `none`.
- All versioned pages use the SDK version (e.g. `v40.0.0` or `v39.0.0`).
- All `hideFromSearch: true` pages don't have the version tag.

### Excluding pages from Docsearch

To ignore a page from the search result, use `hideFromSearch: true` on that page. This removes the `<meta name="docsearch:version">` tag from that page and filters it from our facet-based search.

## Quirks

- You can't have curly brace without quotes: \`{}\` -> `{}`
- Make sure to leave an empty newline between a table and following content

# A note about versioning

Expo's SDK is versioned so that apps made on old SDKs are still supported
when new SDKs are released. The website documents previous SDK versions too.

Version names correspond to directory names under `versions`.

`unversioned` is a special version for the next SDK release. It is not included in production output. Additionally, any versions greater than the package.json `version` number are not included in production output, so that it's possible to generate, test, and make changes to new SDK version docs during the release process.

`latest` is an untracked folder which duplicates the contents of the folder matching the version number in `package.json`.

Sometimes you want to make an edit in version `X` and have that edit also
be applied in versions `Y, Z, ...` (say, when you're fixing documentation for an
API call that existed in old versions too). You can use the
`./scripts/versionpatch.sh` utility to apply your `git diff` in one version in
other versions. For example, to update the docs in `unversioned` then apply it
on `v8.0.0` and `v7.0.0`, you'd do the following after editing the docs in
`unversioned` such that it shows up in `git diff`:

`./scripts/versionpatch.sh unversioned v8.0.0 v7.0.0`

Any changes in your `git diff` outside the `unversioned` directory are ignored
so don't worry if you have code changes or such elsewhere.

## Updating latest version of docs

When we release a new SDK, we copy the `unversioned` directory, and rename it to the new version. Latest version of docs is read from `package.json` so make sure to update the `version` key there as well. However, if you update the `version` key there, you need to `rm -rf node_modules/.cache/` before the change is picked up (why? [read this](https://github.com/vercel/next.js/blob/4.0.0/examples/with-universal-configuration/README.md#caveats)).

Make sure to also grab the upgrade instructions from the release notes blog post and put them in `upgrading-expo-sdk-walkthrough.md`.

That's all you need to do. The `versions` directory is listed on server start to find all available versions. The routes and navbar contents are automatically inferred from the directory structure within `versions`.

Because the navbar is automatically generated from the directory structure, the default ordering of the links under each section is alphabetical. However, for many sections, this is not ideal UX. So, if you wish to override the alphabetical ordering, manipulate page titles in `navigation.js`.

### Syncing app.json / app.config.js with the schema

To render the app.json / app.config.js properties table, we currently store a local copy of the appropriate version of the schema.

If the schema is updated, in order to sync and rewrite our local copy, run `yarn run schema-sync 39` (or relevant version number) or `yarn run schema-sync unversioned`.

### Importing from the React Native docs

You can import the React Native docs in an automated way into these docs.

1. Update the react-native-website submodule here
2. `yarn run import-react-native-docs`

This will write all the relevant RN doc stuff into the unversioned version directory.
You may need to tweak the script as the source docs change; the script hackily translates between the different forms of markdown that have different quirks.

The React Native docs are actually versioned but we currently read off of master.

### Adding video

- Record the video using QuickTime
- Install `ffmpeg` (`brew install ffmpeg`)
- Run `ffmpeg -i your-video-name.mov -vcodec h264 -acodec mp2 your-video-name.mp4` to convert to mp4.
- If the width of the video is larger than ~1200px, then run this to shrink it: `ffmpeg -i your-video.mp4 -filter:v scale="1280:trunc(ow/a/2)*2" your-video-smaller.mp4`
- Put the video in the appropriate location in `public/static/videos` and use it in your docs page MDX like this:

```js
import Video from '~/components/plugins/Video'

// Change the path to point to the relative path to your video from within the `static/videos` directory
<Video file="guides/color-schemes.mp4" />
```

#### TODOs: 
- Handle image sizing in imports better 
- Read from the appropriate version (configurable) of the React Native docs, not just master 
- Make Snack embeds work; these are marked in some of the React Native docs but they are just imported as plain JS code blocks
