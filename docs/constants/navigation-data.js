// @preval

const fm = require('front-matter');
const fs = require('fs-extra');
const path = require('path');

const { isEasReleased } = require('./FeatureFlags');

// TODO(brentvatne): move this to navigation.js so it's all in one place!
// Map directories in a version directory to a section name
const DIR_MAPPING = {
  introduction: 'Conceptual Overview',
  guides: 'Assorted Guides',
  'managed-workflow': 'Managed Workflow',
  bare: 'Essentials',
  tutorials: 'Tutorials',
  sdk: 'Expo SDK',
  config: 'Configuration Files',
  'react-native': 'React Native',
  'get-started': 'Get Started',
  tutorial: 'Tutorial',
  'next-steps': 'Next Steps',
  workflow: 'Fundamentals',
  distribution: 'Distributing Your App',
  expokit: 'ExpoKit',
  'regulatory-compliance': 'Regulatory Compliance',
  'push-notifications': 'Push Notifications',
  preview: 'Preview',
  build: 'Start Building',
  eas: 'Feature Preview',
  'app-signing': 'App Signing',
  'build-reference': 'Reference',
  submit: 'EAS Submit',
  accounts: 'Expo Accounts',
  clients: 'Development Clients',
  archived: 'Archived',
  faq: 'FAQ',
};

const processUrl = path => {
  return path.replace(/^pages\//, '/').replace(/.mdx?$/, '');
};

const generateGeneralNavLinks = (path_, arr = null) => {
  const { name } = path.parse(path_);

  if (arr === null) {
    const initArr = [];
    // Make sure to add '/' at the end of index pages so that relative links in the markdown work correctly
    const href = fs.existsSync(path.join(path_, 'index.md')) ? processUrl(path_) + '/' : '';

    return {
      name: DIR_MAPPING[name.toLowerCase()],
      href,
      posts: generateGeneralNavLinks(path_, initArr),
    };
  }

  const items = fs.readdirSync(path_);
  for (var i = 0; i < items.length; i++) {
    const filePath = path.join(path_, items[i]);
    const { ext, name } = path.parse(filePath);
    // Only process markdown files that are not index pages
    if (ext === '.md' && name !== 'index') {
      try {
        const title = fm(fs.readFileSync(filePath, 'utf8')).attributes.title;
        const sidebarTitle = fm(fs.readFileSync(filePath, 'utf8')).attributes.sidebar_title;
        const obj = {
          name: title,
          sidebarTitle,
          href: processUrl(filePath),
        };
        arr.push(obj);
      } catch (e) {
        console.log(`Error reading frontmatter of ${filePath}`, e);
      }
    }
  }

  return arr;
};

const generateReferenceNavLinks = (path_, arr = []) => {
  const items = fs.readdirSync(path_);

  for (var i = 0; i < items.length; i++) {
    const filePath = path.join(path_, items[i]);
    if (fs.statSync(filePath).isDirectory()) {
      const { name } = path.parse(filePath);
      const initArr = [];

      // Make sure to add '/' at the end of index pages so that relative links in the markdown work correctly
      const href = fs.existsSync(path.join(filePath, 'index.md')) ? processUrl(filePath) + '/' : '';
      arr.push({
        name: DIR_MAPPING[name.toLowerCase()],
        href,
        posts: generateReferenceNavLinks(filePath, initArr),
      });
    } else {
      const { ext, name } = path.parse(filePath);
      // Only process markdown files that are not index pages
      if (ext === '.md' && name !== 'index') {
        try {
          const title = fm(fs.readFileSync(filePath, 'utf8')).attributes.title;
          const sidebarTitle = fm(fs.readFileSync(filePath, 'utf8')).attributes.sidebar_title;
          const obj = {
            name: title,
            sidebarTitle,
            href: processUrl(filePath),
          };
          arr.push(obj);
        } catch (e) {
          console.log(`Error reading frontmatter of ${filePath}`, e);
        }
      }
    }
  }

  return arr;
};

// Find directories within the versions subdirectory
const REFERENCE_PATH_PREFIX = './pages/versions';
const referenceDirectories = fs
  .readdirSync(REFERENCE_PATH_PREFIX, { withFileTypes: true })
  .filter(f => f.isDirectory())
  .map(f => f.name);

// A manual list of directories to pull in to the getting started tutorial
const startingDirectories = ['introduction', 'get-started', 'tutorial', 'next-steps'];

let previewDirectories, easDirectories;
if (isEasReleased) {
  easDirectories = ['eas', 'build', 'app-signing', 'build-reference', 'submit'];
  previewDirectories = ['preview', 'clients'];
} else {
  easDirectories = [];
  previewDirectories = [
    'eas',
    'preview',
    'build',
    'app-signing',
    'build-reference',
    'submit',
    'clients',
  ];
}

// Find any directories that aren't reference or starting directories. Also exclude the api
// directory, which is just a shortcut.
const ROOT_PATH_PREFIX = './pages';
const generalDirectories = fs
  .readdirSync(ROOT_PATH_PREFIX, { withFileTypes: true })
  .filter(f => f.isDirectory())
  .map(f => f.name)
  .filter(
    name =>
      name !== 'api' &&
      name !== 'versions' &&
      ![...startingDirectories, ...previewDirectories, ...easDirectories].includes(name)
  );

module.exports = {
  startingDirectories,
  generalDirectories,
  previewDirectories,
  easDirectories,
  starting: startingDirectories.map(directory =>
    generateGeneralNavLinks(`${ROOT_PATH_PREFIX}/${directory}`)
  ),
  general: generalDirectories.map(directory =>
    generateGeneralNavLinks(`${ROOT_PATH_PREFIX}/${directory}`)
  ),
  preview: previewDirectories.map(directory =>
    generateGeneralNavLinks(`${ROOT_PATH_PREFIX}/${directory}`)
  ),
  eas: easDirectories.map(directory => generateGeneralNavLinks(`${ROOT_PATH_PREFIX}/${directory}`)),
  reference: referenceDirectories.reduce(
    (obj, version) => ({
      ...obj,
      [version]: generateReferenceNavLinks(`${REFERENCE_PATH_PREFIX}/${version}`),
    }),
    {}
  ),
};
