import 'react-native-url-polyfill/auto';

import React, { useEffect, useState } from 'react';
import {
  Text,
  View,
  ScrollView,
  NativeModules,
  Alert,
  SafeAreaView,
  StatusBar,
  StyleSheet,
  TextInput,
  Platform,
  NativeEventEmitter,
  RefreshControl,
} from 'react-native';

import { isDevMenuAvailable } from './DevMenu';
import BottomTabs from './components/BottomTabs';
import Button from './components/Button';
import ListItem from './components/ListItem';

const DevLauncher = NativeModules.EXDevLauncherInternal;

// Use development client native module to load app at given URL, notifying of
// errors

const loadAppFromUrl = async (urlString: string, setLoading: (boolean) => void) => {
  try {
    setLoading(true);
    await DevLauncher.loadApp(urlString);
  } catch (e) {
    setLoading(false);
    Alert.alert('Error loading app', e.message);
  }
};

const ON_NEW_DEEP_LINK_EVENT = 'expo.modules.devlauncher.onnewdeeplink';

const baseAddress = Platform.select({
  ios: 'http://localhost',
  android: 'http://10.0.2.2',
});
const statusPage = 'status';
const portsToCheck = [
  8081,
  19000,
  19001,
  19002,
  19003,
  19004,
  19005,
  19006,
  19007,
  19008,
  19009,
  19010,
];

const bottomContainerHeight = isDevMenuAvailable() ? 40 : 0;

const App = ({ isSimulator }) => {
  const [loading, setLoading] = useState(false);
  const [textInputUrl, setTextInputUrl] = useState('');
  const [recentlyOpenedApps, setRecentlyOpenedApps] = useState({});
  const [pendingDeepLink, setPendingDeepLink] = useState<string | null>(null);
  const [refreshing, setRefreshing] = React.useState(false);
  const [localPackagers, setLocalPackagers] = useState([]);

  const detectLocalPackagers = async setLocalPackagers => {
    if (!isSimulator) {
      return [];
    }

    const onlinePackagers = [];
    for (const port of portsToCheck) {
      try {
        const address = `${baseAddress}:${port}`;
        const { status } = await fetch(`${address}/${statusPage}`);
        if (status === 200) {
          onlinePackagers.push(address);
        }
      } catch (e) {}
    }

    setLocalPackagers(onlinePackagers);
  };

  const onRefresh = React.useCallback(() => {
    setRefreshing(true);
    detectLocalPackagers(setLocalPackagers);
    setRefreshing(false);
  }, []);

  useEffect(() => {
    const getPendingDeepLink = async () => {
      setPendingDeepLink(await DevLauncher.getPendingDeepLink());
    };

    const getRecentlyOpenedApps = async () => {
      setRecentlyOpenedApps(await DevLauncher.getRecentlyOpenedApps());
    };

    const onNewDeepLinkListener = new NativeEventEmitter(DevLauncher).addListener(
      ON_NEW_DEEP_LINK_EVENT,
      (deepLink: string) => {
        setPendingDeepLink(deepLink);
      }
    );

    getRecentlyOpenedApps();
    getPendingDeepLink();
    detectLocalPackagers(setLocalPackagers);

    return () => {
      onNewDeepLinkListener.remove();
    };
  }, []);

  const onPressScanAndroid = async () => {
    try {
      await DevLauncher.openCamera();
    } catch (e) {
      Alert.alert(
        "Couldn't open the camera app. Please, open the system camera and scan the QR code.",
        e.toString()
      );
    }
  };

  const onPressGoToUrl = () => {
    loadAppFromUrl(textInputUrl, setLoading);
  };

  const recentlyProjects = Object.entries(recentlyOpenedApps).map(([url, name]) => {
    const title = (name ?? url) as string;
    return <ListItem key={url} title={title} onPress={() => loadAppFromUrl(url, setLoading)} />;
  });

  if (loading) {
    return (
      <>
        <StatusBar barStyle="dark-content" />
        <View style={styles.loadingContainer}>
          <Text style={styles.loadingText}>Loading...</Text>
        </View>
      </>
    );
  }

  return (
    <SafeAreaView style={styles.safeArea}>
      <StatusBar barStyle="dark-content" />
      <ScrollView
        style={styles.container}
        refreshControl={<RefreshControl refreshing={refreshing} onRefresh={onRefresh} />}>
        <>
          {pendingDeepLink && (
            <View style={styles.pendingDeepLinkContainer}>
              <View style={styles.pendingDeepLinkTextBox}>
                <Text style={styles.pendingDeepLinkInfo}>
                  The application received a deep link. However, the development client couldn't
                  decide where it should be dispatched. The next loaded project will handle the
                  received deep link.
                </Text>
                <Text style={styles.pendingDeepLink}> {pendingDeepLink}</Text>
              </View>
            </View>
          )}
          <View style={styles.homeContainer}>
            <Text style={styles.headingText}>Connect to a development server</Text>
            <Text style={styles.infoText}>Start a local server with:</Text>
            <View style={styles.codeBox}>
              <Text style={styles.codeText}>expo start --dev-client</Text>
            </View>

            {Platform.select({
              ios: (
                <Text style={styles.infoText}>
                  Open your camera app and scan the QR generated by expo start
                </Text>
              ),
              android: (
                <>
                  <Text style={styles.connectText}>Connect this client</Text>
                  <Button onPress={onPressScanAndroid} label="Scan QR code" />
                </>
              ),
            })}

            <Text style={[styles.infoText, { marginTop: 12 }]}>
              Or, enter the URL of a local bundler manually:
            </Text>
            <TextInput
              style={styles.urlTextInput}
              placeholder="exp://192..."
              placeholderTextColor="#b0b0ba"
              value={textInputUrl}
              onChangeText={text => setTextInputUrl(text)}
            />
            <Button onPress={onPressGoToUrl} label="Connect to URL" />
            {localPackagers.length > 0 && (
              <>
                <Text style={[styles.infoText, { marginTop: 12 }]}>Running packagers:</Text>
                {localPackagers.map(url => (
                  <ListItem key={url} title={url} onPress={() => loadAppFromUrl(url, setLoading)} />
                ))}
              </>
            )}
            {recentlyProjects.length > 0 && (
              <>
                <Text style={[styles.infoText, { marginTop: 12 }]}>Recently opened projects:</Text>
                {recentlyProjects}
              </>
            )}
          </View>
        </>
      </ScrollView>
      {bottomContainerHeight > 0 && <BottomTabs height={bottomContainerHeight} />}
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  safeArea: {
    flex: 1,
    backgroundColor: '#fff',
  },
  container: {
    flex: 1,
    marginBottom: bottomContainerHeight,
  },

  homeContainer: {
    paddingHorizontal: 24,
    paddingBottom: 10,
  },

  pendingDeepLinkContainer: {
    paddingHorizontal: -24,
    backgroundColor: '#4630eb',
  },
  pendingDeepLinkTextBox: {
    padding: 10,
  },
  pendingDeepLinkInfo: {
    color: '#f5f5f7',
  },
  pendingDeepLink: {
    marginTop: 10,
    color: '#fff',
    fontWeight: '700',
    fontSize: 16,
  },

  loadingContainer: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
  loadingText: {
    fontSize: 24,
  },

  headingText: {
    fontSize: 22,
    fontWeight: '600',
    marginBottom: 20,
    paddingTop: 24,
  },
  infoText: {
    fontSize: 16,
    marginBottom: 10,
  },

  codeBox: {
    backgroundColor: '#f5f5f7',
    borderWidth: 1,
    borderColor: '#dddde1',
    padding: 14,
    borderRadius: 4,
    marginBottom: 20,
  },
  codeText: {
    fontFamily: Platform.OS === 'ios' ? 'Menlo' : 'monospace',
    fontSize: 14,
  },

  connectText: {
    fontSize: 16,
    fontWeight: '600',
  },

  urlTextInput: {
    width: '100%',

    fontSize: 16,
    padding: 8,

    borderWidth: 1,
    borderColor: '#dddde1',
    borderRadius: 4,
  },
});

export default App;
