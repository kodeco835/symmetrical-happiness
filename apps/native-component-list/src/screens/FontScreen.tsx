import { MaterialIcons } from '@expo/vector-icons';
import React from 'react';
import { Platform, ScrollView, Text, View } from 'react-native';

export default class FontScreen extends React.Component {
  static navigationOptions = {
    title: 'Font',
  };

  render() {
    return (
      <ScrollView style={{ flex: 1 }}>
        <View
          style={{
            paddingVertical: 10,
            paddingHorizontal: 15,
            flexDirection: 'row',
            justifyContent: 'space-between',
            flex: 1,
          }}>
          <MaterialIcons name="airplay" size={25} />
          <MaterialIcons name="airport-shuttle" size={25} />
          <MaterialIcons name="alarm" size={25} />
          <MaterialIcons name="alarm-add" size={25} />
          <MaterialIcons name="alarm-off" size={25} />
          <MaterialIcons name="all-inclusive" size={25} />
        </View>

        <View style={{ paddingVertical: 10, paddingHorizontal: 15, flex: 1 }}>
          <Text style={{ fontFamily: 'space-mono', fontSize: 16 }}>
            Font icons sets and other custom fonts can be loaded from the web
          </Text>
          <Text style={{ fontFamily: 'Roboto', fontSize: 16 }}>
            Font icons sets and other custom fonts can be loaded by providing remote uri as well.
          </Text>
          {Platform.OS === 'ios' && (
            <Text
              adjustsFontSizeToFit
              numberOfLines={2}
              style={{
                fontFamily: 'space-mono',
                fontSize: 420,
              }}>
              Custom font with `adjustsFontSizeToFit` on iOS
            </Text>
          )}
          {Platform.OS === 'ios' && (
            <Text
              adjustsFontSizeToFit
              numberOfLines={1}
              style={{
                fontFamily: 'Roboto',
                fontSize: 420,
              }}>
              Custom remote uri font with `adjustsFontSizeToFit` on iOS
            </Text>
          )}
        </View>
      </ScrollView>
    );
  }
}
