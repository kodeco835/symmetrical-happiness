/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <ABI38_0_0jsi/ABI38_0_0test/testlib.h>
#include <gtest/gtest.h>
#include <ABI38_0_0jsi/ABI38_0_0decorator.h>
#include <ABI38_0_0jsi/ABI38_0_0jsi.h>

#include <stdlib.h>
#include <chrono>
#include <functional>
#include <thread>
#include <unordered_map>
#include <unordered_set>

using namespace ABI38_0_0facebook::jsi;

class JSITest : public JSITestBase {};

TEST_P(JSITest, RuntimeTest) {
  auto v = rt.evaluateJavaScript(std::make_unique<StringBuffer>("1"), "");
  ABI38_0_0EXPECT_EQ(v.getNumber(), 1);

  rt.evaluateJavaScript(std::make_unique<StringBuffer>("x = 1"), "");
  ABI38_0_0EXPECT_EQ(rt.global().getProperty(rt, "x").getNumber(), 1);
}

TEST_P(JSITest, PropNameIDTest) {
  // This is a little weird to test, because it doesn't really exist
  // in JS yet.  All I can do is create them, compare them, and
  // receive one as an argument to a HostObject.

  PropNameID quux = PropNameID::forAscii(rt, "quux1", 4);
  PropNameID movedQuux = std::move(quux);
  ABI38_0_0EXPECT_EQ(movedQuux.utf8(rt), "quux");
  movedQuux = PropNameID::forAscii(rt, "quux2");
  ABI38_0_0EXPECT_EQ(movedQuux.utf8(rt), "quux2");
  PropNameID copiedQuux = PropNameID(rt, movedQuux);
  ABI38_0_0EXPECT_TRUE(PropNameID::compare(rt, movedQuux, copiedQuux));

  ABI38_0_0EXPECT_TRUE(PropNameID::compare(rt, movedQuux, movedQuux));
  ABI38_0_0EXPECT_TRUE(PropNameID::compare(
      rt, movedQuux, PropNameID::forAscii(rt, std::string("quux2"))));
  ABI38_0_0EXPECT_FALSE(PropNameID::compare(
      rt, movedQuux, PropNameID::forAscii(rt, std::string("foo"))));
  uint8_t utf8[] = {0xF0, 0x9F, 0x86, 0x97};
  PropNameID utf8PropNameID = PropNameID::forUtf8(rt, utf8, sizeof(utf8));
  ABI38_0_0EXPECT_EQ(utf8PropNameID.utf8(rt), u8"\U0001F197");
  ABI38_0_0EXPECT_TRUE(PropNameID::compare(
      rt, utf8PropNameID, PropNameID::forUtf8(rt, utf8, sizeof(utf8))));
  PropNameID nonUtf8PropNameID = PropNameID::forUtf8(rt, "meow");
  ABI38_0_0EXPECT_TRUE(PropNameID::compare(
      rt, nonUtf8PropNameID, PropNameID::forAscii(rt, "meow")));
  ABI38_0_0EXPECT_EQ(nonUtf8PropNameID.utf8(rt), "meow");
  PropNameID strPropNameID =
      PropNameID::forString(rt, String::createFromAscii(rt, "meow"));
  ABI38_0_0EXPECT_TRUE(PropNameID::compare(rt, nonUtf8PropNameID, strPropNameID));

  auto names = PropNameID::names(
      rt, "Ala", std::string("ma"), PropNameID::forAscii(rt, "kota"));
  ABI38_0_0EXPECT_EQ(names.size(), 3);
  ABI38_0_0EXPECT_TRUE(
      PropNameID::compare(rt, names[0], PropNameID::forAscii(rt, "Ala")));
  ABI38_0_0EXPECT_TRUE(
      PropNameID::compare(rt, names[1], PropNameID::forAscii(rt, "ma")));
  ABI38_0_0EXPECT_TRUE(
      PropNameID::compare(rt, names[2], PropNameID::forAscii(rt, "kota")));
}

TEST_P(JSITest, StringTest) {
  ABI38_0_0EXPECT_TRUE(checkValue(String::createFromAscii(rt, "foobar", 3), "'foo'"));
  ABI38_0_0EXPECT_TRUE(checkValue(String::createFromAscii(rt, "foobar"), "'foobar'"));

  std::string baz = "baz";
  ABI38_0_0EXPECT_TRUE(checkValue(String::createFromAscii(rt, baz), "'baz'"));

  uint8_t utf8[] = {0xF0, 0x9F, 0x86, 0x97};
  ABI38_0_0EXPECT_TRUE(checkValue(
      String::createFromUtf8(rt, utf8, sizeof(utf8)), "'\\uD83C\\uDD97'"));

  ABI38_0_0EXPECT_EQ(eval("'quux'").getString(rt).utf8(rt), "quux");
  ABI38_0_0EXPECT_EQ(eval("'\\u20AC'").getString(rt).utf8(rt), "\xe2\x82\xac");

  String quux = String::createFromAscii(rt, "quux");
  String movedQuux = std::move(quux);
  ABI38_0_0EXPECT_EQ(movedQuux.utf8(rt), "quux");
  movedQuux = String::createFromAscii(rt, "quux2");
  ABI38_0_0EXPECT_EQ(movedQuux.utf8(rt), "quux2");
}

TEST_P(JSITest, ObjectTest) {
  eval("x = {1:2, '3':4, 5:'six', 'seven':['eight', 'nine']}");
  Object x = rt.global().getPropertyAsObject(rt, "x");
  ABI38_0_0EXPECT_EQ(x.getPropertyNames(rt).size(rt), 4);
  ABI38_0_0EXPECT_TRUE(x.hasProperty(rt, "1"));
  ABI38_0_0EXPECT_TRUE(x.hasProperty(rt, PropNameID::forAscii(rt, "1")));
  ABI38_0_0EXPECT_FALSE(x.hasProperty(rt, "2"));
  ABI38_0_0EXPECT_FALSE(x.hasProperty(rt, PropNameID::forAscii(rt, "2")));
  ABI38_0_0EXPECT_TRUE(x.hasProperty(rt, "3"));
  ABI38_0_0EXPECT_TRUE(x.hasProperty(rt, PropNameID::forAscii(rt, "3")));
  ABI38_0_0EXPECT_TRUE(x.hasProperty(rt, "seven"));
  ABI38_0_0EXPECT_TRUE(x.hasProperty(rt, PropNameID::forAscii(rt, "seven")));
  ABI38_0_0EXPECT_EQ(x.getProperty(rt, "1").getNumber(), 2);
  ABI38_0_0EXPECT_EQ(x.getProperty(rt, PropNameID::forAscii(rt, "1")).getNumber(), 2);
  ABI38_0_0EXPECT_EQ(x.getProperty(rt, "3").getNumber(), 4);
  Value five = 5;
  ABI38_0_0EXPECT_EQ(
      x.getProperty(rt, PropNameID::forString(rt, five.toString(rt)))
          .getString(rt)
          .utf8(rt),
      "six");

  x.setProperty(rt, "ten", 11);
  ABI38_0_0EXPECT_EQ(x.getPropertyNames(rt).size(rt), 5);
  ABI38_0_0EXPECT_TRUE(eval("x.ten == 11").getBool());

  x.setProperty(rt, "e_as_float", 2.71f);
  ABI38_0_0EXPECT_TRUE(eval("Math.abs(x.e_as_float - 2.71) < 0.001").getBool());

  x.setProperty(rt, "e_as_double", 2.71);
  ABI38_0_0EXPECT_TRUE(eval("x.e_as_double == 2.71").getBool());

  uint8_t utf8[] = {0xF0, 0x9F, 0x86, 0x97};
  String nonAsciiName = String::createFromUtf8(rt, utf8, sizeof(utf8));
  x.setProperty(rt, PropNameID::forString(rt, nonAsciiName), "emoji");
  ABI38_0_0EXPECT_EQ(x.getPropertyNames(rt).size(rt), 8);
  ABI38_0_0EXPECT_TRUE(eval("x['\\uD83C\\uDD97'] == 'emoji'").getBool());

  Object seven = x.getPropertyAsObject(rt, "seven");
  ABI38_0_0EXPECT_TRUE(seven.isArray(rt));
  Object evalf = rt.global().getPropertyAsObject(rt, "eval");
  ABI38_0_0EXPECT_TRUE(evalf.isFunction(rt));

  Object movedX = Object(rt);
  movedX = std::move(x);
  ABI38_0_0EXPECT_EQ(movedX.getPropertyNames(rt).size(rt), 8);
  ABI38_0_0EXPECT_EQ(movedX.getProperty(rt, "1").getNumber(), 2);

  Object obj = Object(rt);
  obj.setProperty(rt, "roses", "red");
  obj.setProperty(rt, "violets", "blue");
  Object oprop = Object(rt);
  obj.setProperty(rt, "oprop", oprop);
  obj.setProperty(rt, "aprop", Array(rt, 1));

  ABI38_0_0EXPECT_TRUE(function("function (obj) { return "
                       "obj.roses == 'red' && "
                       "obj['violets'] == 'blue' && "
                       "typeof obj.oprop == 'object' && "
                       "Array.isArray(obj.aprop); }")
                  .call(rt, obj)
                  .getBool());

  // Check that getPropertyNames doesn't return non-enumerable
  // properties.
  obj = function(
            "function () {"
            "  obj = {};"
            "  obj.a = 1;"
            "  Object.defineProperty(obj, 'b', {"
            "    enumerable: false,"
            "    value: 2"
            "  });"
            "  return obj;"
            "}")
            .call(rt)
            .getObject(rt);
  ABI38_0_0EXPECT_EQ(obj.getProperty(rt, "a").getNumber(), 1);
  ABI38_0_0EXPECT_EQ(obj.getProperty(rt, "b").getNumber(), 2);
  Array names = obj.getPropertyNames(rt);
  ABI38_0_0EXPECT_EQ(names.size(rt), 1);
  ABI38_0_0EXPECT_EQ(names.getValueAtIndex(rt, 0).getString(rt).utf8(rt), "a");
}

TEST_P(JSITest, HostObjectTest) {
  class ConstantHostObject : public HostObject {
    Value get(Runtime&, const PropNameID& sym) override {
      return 9000;
    }

    void set(Runtime&, const PropNameID&, const Value&) override {}
  };

  Object cho =
      Object::createFromHostObject(rt, std::make_shared<ConstantHostObject>());
  ABI38_0_0EXPECT_TRUE(function("function (obj) { return obj.someRandomProp == 9000; }")
                  .call(rt, cho)
                  .getBool());
  ABI38_0_0EXPECT_TRUE(cho.isHostObject(rt));
  ABI38_0_0EXPECT_TRUE(cho.getHostObject<ConstantHostObject>(rt).get() != nullptr);

  struct SameRuntimeHostObject : HostObject {
    SameRuntimeHostObject(Runtime& rt) : rt_(rt){};

    Value get(Runtime& rt, const PropNameID& sym) override {
      ABI38_0_0EXPECT_EQ(&rt, &rt_);
      return Value();
    }

    void set(Runtime& rt, const PropNameID& name, const Value& value) override {
      ABI38_0_0EXPECT_EQ(&rt, &rt_);
    }

    std::vector<PropNameID> getPropertyNames(Runtime& rt) override {
      ABI38_0_0EXPECT_EQ(&rt, &rt_);
      return {};
    }

    Runtime& rt_;
  };

  Object srho = Object::createFromHostObject(
      rt, std::make_shared<SameRuntimeHostObject>(rt));
  // Test get's Runtime is as expected
  function("function (obj) { return obj.isSame; }").call(rt, srho);
  // ... and set
  function("function (obj) { obj['k'] = 'v'; }").call(rt, srho);
  // ... and getPropertyNames
  function("function (obj) { for (k in obj) {} }").call(rt, srho);

  class TwiceHostObject : public HostObject {
    Value get(Runtime& rt, const PropNameID& sym) override {
      return String::createFromUtf8(rt, sym.utf8(rt) + sym.utf8(rt));
    }

    void set(Runtime&, const PropNameID&, const Value&) override {}
  };

  Object tho =
      Object::createFromHostObject(rt, std::make_shared<TwiceHostObject>());
  ABI38_0_0EXPECT_TRUE(function("function (obj) { return obj.abc == 'abcabc'; }")
                  .call(rt, tho)
                  .getBool());
  ABI38_0_0EXPECT_TRUE(function("function (obj) { return obj['def'] == 'defdef'; }")
                  .call(rt, tho)
                  .getBool());
  ABI38_0_0EXPECT_TRUE(function("function (obj) { return obj[12] === '1212'; }")
                  .call(rt, tho)
                  .getBool());
  ABI38_0_0EXPECT_TRUE(tho.isHostObject(rt));
  ABI38_0_0EXPECT_TRUE(
      std::dynamic_pointer_cast<ConstantHostObject>(tho.getHostObject(rt)) ==
      nullptr);
  ABI38_0_0EXPECT_TRUE(tho.getHostObject<TwiceHostObject>(rt).get() != nullptr);

  class PropNameIDHostObject : public HostObject {
    Value get(Runtime& rt, const PropNameID& sym) override {
      if (PropNameID::compare(rt, sym, PropNameID::forAscii(rt, "undef"))) {
        return Value::undefined();
      } else {
        return PropNameID::compare(
            rt, sym, PropNameID::forAscii(rt, "somesymbol"));
      }
    }

    void set(Runtime&, const PropNameID&, const Value&) override {}
  };

  Object sho = Object::createFromHostObject(
      rt, std::make_shared<PropNameIDHostObject>());
  ABI38_0_0EXPECT_TRUE(sho.isHostObject(rt));
  ABI38_0_0EXPECT_TRUE(function("function (obj) { return obj.undef; }")
                  .call(rt, sho)
                  .isUndefined());
  ABI38_0_0EXPECT_TRUE(function("function (obj) { return obj.somesymbol; }")
                  .call(rt, sho)
                  .getBool());
  ABI38_0_0EXPECT_FALSE(function("function (obj) { return obj.notsomuch; }")
                   .call(rt, sho)
                   .getBool());

  class BagHostObject : public HostObject {
   public:
    const std::string& getThing() {
      return bag_["thing"];
    }

   private:
    Value get(Runtime& rt, const PropNameID& sym) override {
      if (sym.utf8(rt) == "thing") {
        return String::createFromUtf8(rt, bag_[sym.utf8(rt)]);
      }
      return Value::undefined();
    }

    void set(Runtime& rt, const PropNameID& sym, const Value& val) override {
      std::string key(sym.utf8(rt));
      if (key == "thing") {
        bag_[key] = val.toString(rt).utf8(rt);
      }
    }

    std::unordered_map<std::string, std::string> bag_;
  };

  std::shared_ptr<BagHostObject> shbho = std::make_shared<BagHostObject>();
  Object bho = Object::createFromHostObject(rt, shbho);
  ABI38_0_0EXPECT_TRUE(bho.isHostObject(rt));
  ABI38_0_0EXPECT_TRUE(function("function (obj) { return obj.undef; }")
                  .call(rt, bho)
                  .isUndefined());
  ABI38_0_0EXPECT_EQ(
      function("function (obj) { obj.thing = 'hello'; return obj.thing; }")
          .call(rt, bho)
          .toString(rt)
          .utf8(rt),
      "hello");
  ABI38_0_0EXPECT_EQ(shbho->getThing(), "hello");

  class ThrowingHostObject : public HostObject {
    Value get(Runtime& rt, const PropNameID& sym) override {
      throw std::runtime_error("Cannot get");
    }

    void set(Runtime& rt, const PropNameID& sym, const Value& val) override {
      throw std::runtime_error("Cannot set");
    }
  };

  Object thro =
      Object::createFromHostObject(rt, std::make_shared<ThrowingHostObject>());
  ABI38_0_0EXPECT_TRUE(thro.isHostObject(rt));
  std::string exc;
  try {
    function("function (obj) { return obj.thing; }").call(rt, thro);
  } catch (const JSError& ex) {
    exc = ex.what();
  }
  ABI38_0_0EXPECT_NE(exc.find("Cannot get"), std::string::npos);
  exc = "";
  try {
    function("function (obj) { obj.thing = 'hello'; }").call(rt, thro);
  } catch (const JSError& ex) {
    exc = ex.what();
  }
  ABI38_0_0EXPECT_NE(exc.find("Cannot set"), std::string::npos);

  class NopHostObject : public HostObject {};
  Object nopHo =
      Object::createFromHostObject(rt, std::make_shared<NopHostObject>());
  ABI38_0_0EXPECT_TRUE(nopHo.isHostObject(rt));
  ABI38_0_0EXPECT_TRUE(function("function (obj) { return obj.thing; }")
                  .call(rt, nopHo)
                  .isUndefined());

  std::string nopExc;
  try {
    function("function (obj) { obj.thing = 'pika'; }").call(rt, nopHo);
  } catch (const JSError& ex) {
    nopExc = ex.what();
  }
  ABI38_0_0EXPECT_NE(nopExc.find("TypeError: "), std::string::npos);

  class HostObjectWithPropertyNames : public HostObject {
    std::vector<PropNameID> getPropertyNames(Runtime& rt) override {
      return PropNameID::names(
          rt, "a_prop", "1", "false", "a_prop", "3", "c_prop");
    }
  };

  Object howpn = Object::createFromHostObject(
      rt, std::make_shared<HostObjectWithPropertyNames>());
  ABI38_0_0EXPECT_TRUE(
      function(
          "function (o) { return Object.getOwnPropertyNames(o).length == 5 }")
          .call(rt, howpn)
          .getBool());

  auto hasOwnPropertyName = function(
      "function (o, p) {"
      "  return Object.getOwnPropertyNames(o).indexOf(p) >= 0"
      "}");
  ABI38_0_0EXPECT_TRUE(
      hasOwnPropertyName.call(rt, howpn, String::createFromAscii(rt, "a_prop"))
          .getBool());
  ABI38_0_0EXPECT_TRUE(
      hasOwnPropertyName.call(rt, howpn, String::createFromAscii(rt, "1"))
          .getBool());
  ABI38_0_0EXPECT_TRUE(
      hasOwnPropertyName.call(rt, howpn, String::createFromAscii(rt, "false"))
          .getBool());
  ABI38_0_0EXPECT_TRUE(
      hasOwnPropertyName.call(rt, howpn, String::createFromAscii(rt, "3"))
          .getBool());
  ABI38_0_0EXPECT_TRUE(
      hasOwnPropertyName.call(rt, howpn, String::createFromAscii(rt, "c_prop"))
          .getBool());
  ABI38_0_0EXPECT_FALSE(hasOwnPropertyName
                   .call(rt, howpn, String::createFromAscii(rt, "not_existing"))
                   .getBool());
}

TEST_P(JSITest, ArrayTest) {
  eval("x = {1:2, '3':4, 5:'six', 'seven':['eight', 'nine']}");

  Object x = rt.global().getPropertyAsObject(rt, "x");
  Array names = x.getPropertyNames(rt);
  ABI38_0_0EXPECT_EQ(names.size(rt), 4);
  std::unordered_set<std::string> strNames;
  for (size_t i = 0; i < names.size(rt); ++i) {
    Value n = names.getValueAtIndex(rt, i);
    ABI38_0_0EXPECT_TRUE(n.isString());
    strNames.insert(n.getString(rt).utf8(rt));
  }

  ABI38_0_0EXPECT_EQ(strNames.size(), 4);
  ABI38_0_0EXPECT_EQ(strNames.count("1"), 1);
  ABI38_0_0EXPECT_EQ(strNames.count("3"), 1);
  ABI38_0_0EXPECT_EQ(strNames.count("5"), 1);
  ABI38_0_0EXPECT_EQ(strNames.count("seven"), 1);

  Object seven = x.getPropertyAsObject(rt, "seven");
  Array arr = seven.getArray(rt);

  ABI38_0_0EXPECT_EQ(arr.size(rt), 2);
  ABI38_0_0EXPECT_EQ(arr.getValueAtIndex(rt, 0).getString(rt).utf8(rt), "eight");
  ABI38_0_0EXPECT_EQ(arr.getValueAtIndex(rt, 1).getString(rt).utf8(rt), "nine");
  // TODO: test out of range

  ABI38_0_0EXPECT_EQ(x.getPropertyAsObject(rt, "seven").getArray(rt).size(rt), 2);

  // Check that property access with both symbols and strings can access
  // array values.
  ABI38_0_0EXPECT_EQ(seven.getProperty(rt, "0").getString(rt).utf8(rt), "eight");
  ABI38_0_0EXPECT_EQ(seven.getProperty(rt, "1").getString(rt).utf8(rt), "nine");
  seven.setProperty(rt, "1", "modified");
  ABI38_0_0EXPECT_EQ(seven.getProperty(rt, "1").getString(rt).utf8(rt), "modified");
  ABI38_0_0EXPECT_EQ(arr.getValueAtIndex(rt, 1).getString(rt).utf8(rt), "modified");
  ABI38_0_0EXPECT_EQ(
      seven.getProperty(rt, PropNameID::forAscii(rt, "0"))
          .getString(rt)
          .utf8(rt),
      "eight");
  seven.setProperty(rt, PropNameID::forAscii(rt, "0"), "modified2");
  ABI38_0_0EXPECT_EQ(arr.getValueAtIndex(rt, 0).getString(rt).utf8(rt), "modified2");

  Array alpha = Array(rt, 4);
  ABI38_0_0EXPECT_TRUE(alpha.getValueAtIndex(rt, 0).isUndefined());
  ABI38_0_0EXPECT_TRUE(alpha.getValueAtIndex(rt, 3).isUndefined());
  ABI38_0_0EXPECT_EQ(alpha.size(rt), 4);
  alpha.setValueAtIndex(rt, 0, "a");
  alpha.setValueAtIndex(rt, 1, "b");
  ABI38_0_0EXPECT_EQ(alpha.length(rt), 4);
  alpha.setValueAtIndex(rt, 2, "c");
  alpha.setValueAtIndex(rt, 3, "d");
  ABI38_0_0EXPECT_EQ(alpha.size(rt), 4);

  ABI38_0_0EXPECT_TRUE(
      function(
          "function (arr) { return "
          "arr.length == 4 && "
          "['a','b','c','d'].every(function(v,i) { return v === arr[i]}); }")
          .call(rt, alpha)
          .getBool());

  Array alpha2 = Array(rt, 1);
  alpha2 = std::move(alpha);
  ABI38_0_0EXPECT_EQ(alpha2.size(rt), 4);
}

TEST_P(JSITest, FunctionTest) {
  // test move ctor
  Function fmove = function("function() { return 1 }");
  {
    Function g = function("function() { return 2 }");
    fmove = std::move(g);
  }
  ABI38_0_0EXPECT_EQ(fmove.call(rt).getNumber(), 2);

  // This tests all the function argument converters, and all the
  // non-lvalue overloads of call().
  Function f = function(
      "function(n, b, d, df, i, s1, s2, s3, o, a, f, v) { return "
      "n === null && "
      "b === true && "
      "d === 3.14 && "
      "Math.abs(df - 2.71) < 0.001 && "
      "i === 17 && "
      "s1 == 's1' && "
      "s2 == 's2' && "
      "s3 == 's3' && "
      "typeof o == 'object' && "
      "Array.isArray(a) && "
      "typeof f == 'function' && "
      "v == 42 }");
  std::string s3 = "s3";
  ABI38_0_0EXPECT_TRUE(f.call(
                   rt,
                   nullptr,
                   true,
                   3.14,
                   2.71f,
                   17,
                   "s1",
                   String::createFromAscii(rt, "s2"),
                   s3,
                   Object(rt),
                   Array(rt, 1),
                   function("function(){}"),
                   Value(42))
                  .getBool());

  // lvalue overloads of call()
  Function flv = function(
      "function(s, o, a, f, v) { return "
      "s == 's' && "
      "typeof o == 'object' && "
      "Array.isArray(a) && "
      "typeof f == 'function' && "
      "v == 42 }");

  String s = String::createFromAscii(rt, "s");
  Object o = Object(rt);
  Array a = Array(rt, 1);
  Value v = 42;
  ABI38_0_0EXPECT_TRUE(flv.call(rt, s, o, a, f, v).getBool());

  Function f1 = function("function() { return 1; }");
  Function f2 = function("function() { return 2; }");
  f2 = std::move(f1);
  ABI38_0_0EXPECT_EQ(f2.call(rt).getNumber(), 1);
}

TEST_P(JSITest, FunctionThisTest) {
  Function checkPropertyFunction =
      function("function() { return this.a === 'a_property' }");

  Object jsObject = Object(rt);
  jsObject.setProperty(rt, "a", String::createFromUtf8(rt, "a_property"));

  class APropertyHostObject : public HostObject {
    Value get(Runtime& rt, const PropNameID& sym) override {
      return String::createFromUtf8(rt, "a_property");
    }

    void set(Runtime&, const PropNameID&, const Value&) override {}
  };
  Object hostObject =
      Object::createFromHostObject(rt, std::make_shared<APropertyHostObject>());

  ABI38_0_0EXPECT_TRUE(checkPropertyFunction.callWithThis(rt, jsObject).getBool());
  ABI38_0_0EXPECT_TRUE(checkPropertyFunction.callWithThis(rt, hostObject).getBool());
  ABI38_0_0EXPECT_FALSE(checkPropertyFunction.callWithThis(rt, Array(rt, 5)).getBool());
  ABI38_0_0EXPECT_FALSE(checkPropertyFunction.call(rt).getBool());
}

TEST_P(JSITest, FunctionConstructorTest) {
  Function ctor = function(
      "function (a) {"
      "  if (typeof a !== 'undefined') {"
      "   this.pika = a;"
      "  }"
      "}");
  ctor.getProperty(rt, "prototype")
      .getObject(rt)
      .setProperty(rt, "pika", "chu");
  auto empty = ctor.callAsConstructor(rt);
  ASSERT_TRUE(empty.isObject());
  auto emptyObj = std::move(empty).getObject(rt);
  ABI38_0_0EXPECT_EQ(emptyObj.getProperty(rt, "pika").getString(rt).utf8(rt), "chu");
  auto who = ctor.callAsConstructor(rt, "who");
  ASSERT_TRUE(who.isObject());
  auto whoObj = std::move(who).getObject(rt);
  ABI38_0_0EXPECT_EQ(whoObj.getProperty(rt, "pika").getString(rt).utf8(rt), "who");

  auto instanceof = function("function (o, b) { return o instanceof b; }");
  ABI38_0_0EXPECT_TRUE(instanceof.call(rt, emptyObj, ctor).getBool());
  ABI38_0_0EXPECT_TRUE(instanceof.call(rt, whoObj, ctor).getBool());

  auto dateCtor = rt.global().getPropertyAsFunction(rt, "Date");
  auto date = dateCtor.callAsConstructor(rt);
  ABI38_0_0EXPECT_TRUE(date.isObject());
  ABI38_0_0EXPECT_TRUE(instanceof.call(rt, date, dateCtor).getBool());
  // Sleep for 50 milliseconds
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  ABI38_0_0EXPECT_GE(
      function("function (d) { return (new Date()).getTime() - d.getTime(); }")
          .call(rt, date)
          .getNumber(),
      50);
}

TEST_P(JSITest, InstanceOfTest) {
  auto ctor = function("function Rick() { this.say = 'wubalubadubdub'; }");
  auto newObj = function("function (ctor) { return new ctor(); }");
  auto instance = newObj.call(rt, ctor).getObject(rt);
  ABI38_0_0EXPECT_TRUE(instance.instanceOf(rt, ctor));
  ABI38_0_0EXPECT_EQ(
      instance.getProperty(rt, "say").getString(rt).utf8(rt), "wubalubadubdub");
  ABI38_0_0EXPECT_FALSE(Object(rt).instanceOf(rt, ctor));
  ABI38_0_0EXPECT_TRUE(ctor.callAsConstructor(rt, nullptr, 0)
                  .getObject(rt)
                  .instanceOf(rt, ctor));
}

TEST_P(JSITest, HostFunctionTest) {
  auto one = std::make_shared<int>(1);
  Function plusOne = Function::createFromHostFunction(
      rt,
      PropNameID::forAscii(rt, "plusOne"),
      2,
      [one, savedRt = &rt](
          Runtime& rt, const Value& thisVal, const Value* args, size_t count) {
        ABI38_0_0EXPECT_EQ(savedRt, &rt);
        // We don't know if we're in strict mode or not, so it's either global
        // or undefined.
        ABI38_0_0EXPECT_TRUE(
            Value::strictEquals(rt, thisVal, rt.global()) ||
            thisVal.isUndefined());
        return *one + args[0].getNumber() + args[1].getNumber();
      });

  ABI38_0_0EXPECT_EQ(plusOne.call(rt, 1, 2).getNumber(), 4);
  ABI38_0_0EXPECT_TRUE(checkValue(plusOne.call(rt, 3, 5), "9"));
  rt.global().setProperty(rt, "plusOne", plusOne);
  ABI38_0_0EXPECT_TRUE(eval("plusOne(20, 300) == 321").getBool());

  Function dot = Function::createFromHostFunction(
      rt,
      PropNameID::forAscii(rt, "dot"),
      2,
      [](Runtime& rt, const Value& thisVal, const Value* args, size_t count) {
        ABI38_0_0EXPECT_TRUE(
            Value::strictEquals(rt, thisVal, rt.global()) ||
            thisVal.isUndefined());
        if (count != 2) {
          throw std::runtime_error("expected 2 args");
        }
        std::string ret = args[0].getString(rt).utf8(rt) + "." +
            args[1].getString(rt).utf8(rt);
        return String::createFromUtf8(
            rt, reinterpret_cast<const uint8_t*>(ret.data()), ret.size());
      });

  rt.global().setProperty(rt, "cons", dot);
  ABI38_0_0EXPECT_TRUE(eval("cons('left', 'right') == 'left.right'").getBool());
  ABI38_0_0EXPECT_TRUE(eval("cons.name == 'dot'").getBool());
  ABI38_0_0EXPECT_TRUE(eval("cons.length == 2").getBool());
  ABI38_0_0EXPECT_TRUE(eval("cons instanceof Function").getBool());

  ABI38_0_0EXPECT_TRUE(eval("(function() {"
                   "  try {"
                   "    cons('fail'); return false;"
                   "  } catch (e) {"
                   "    return ((e instanceof Error) &&"
                   "            (e.message == 'Exception in HostFunction: ' +"
                   "                          'expected 2 args'));"
                   "  }})()")
                  .getBool());

  Function coolify = Function::createFromHostFunction(
      rt,
      PropNameID::forAscii(rt, "coolify"),
      0,
      [](Runtime& rt, const Value& thisVal, const Value* args, size_t count) {
        ABI38_0_0EXPECT_EQ(count, 0);
        std::string ret = thisVal.toString(rt).utf8(rt) + " is cool";
        return String::createFromUtf8(
            rt, reinterpret_cast<const uint8_t*>(ret.data()), ret.size());
      });
  rt.global().setProperty(rt, "coolify", coolify);
  ABI38_0_0EXPECT_TRUE(eval("coolify.name == 'coolify'").getBool());
  ABI38_0_0EXPECT_TRUE(eval("coolify.length == 0").getBool());
  ABI38_0_0EXPECT_TRUE(eval("coolify.bind('R&M')() == 'R&M is cool'").getBool());
  ABI38_0_0EXPECT_TRUE(eval("(function() {"
                   "  var s = coolify.bind(function(){})();"
                   "  return s.lastIndexOf(' is cool') == (s.length - 8);"
                   "})()")
                  .getBool());

  Function lookAtMe = Function::createFromHostFunction(
      rt,
      PropNameID::forAscii(rt, "lookAtMe"),
      0,
      [](Runtime& rt, const Value& thisVal, const Value* args, size_t count)
          -> Value {
        ABI38_0_0EXPECT_TRUE(thisVal.isObject());
        ABI38_0_0EXPECT_EQ(
            thisVal.getObject(rt)
                .getProperty(rt, "name")
                .getString(rt)
                .utf8(rt),
            "mr.meeseeks");
        return Value();
      });
  rt.global().setProperty(rt, "lookAtMe", lookAtMe);
  ABI38_0_0EXPECT_TRUE(eval("lookAtMe.bind({'name': 'mr.meeseeks'})()").isUndefined());

  struct Callable {
    Callable(std::string s) : str(s) {}

    Value
    operator()(Runtime& rt, const Value&, const Value* args, size_t count) {
      if (count != 1) {
        return Value();
      }
      return String::createFromUtf8(
          rt, args[0].toString(rt).utf8(rt) + " was called with " + str);
    }

    std::string str;
  };

  Function callable = Function::createFromHostFunction(
      rt,
      PropNameID::forAscii(rt, "callable"),
      1,
      Callable("std::function::target"));
  ABI38_0_0EXPECT_EQ(
      function("function (f) { return f('A cat'); }")
          .call(rt, callable)
          .getString(rt)
          .utf8(rt),
      "A cat was called with std::function::target");
  ABI38_0_0EXPECT_TRUE(callable.isHostFunction(rt));
  ABI38_0_0EXPECT_NE(callable.getHostFunction(rt).target<Callable>(), nullptr);

  std::string strval = "strval1";
  auto getter = Object(rt);
  getter.setProperty(
      rt,
      "get",
      Function::createFromHostFunction(
          rt,
          PropNameID::forAscii(rt, "getter"),
          1,
          [&strval](
              Runtime& rt,
              const Value& thisVal,
              const Value* args,
              size_t count) -> Value {
            return String::createFromUtf8(rt, strval);
          }));
  auto obj = Object(rt);
  rt.global()
      .getPropertyAsObject(rt, "Object")
      .getPropertyAsFunction(rt, "defineProperty")
      .call(rt, obj, "prop", getter);
  ABI38_0_0EXPECT_TRUE(function("function(value) { return value.prop == 'strval1'; }")
                  .call(rt, obj)
                  .getBool());
  strval = "strval2";
  ABI38_0_0EXPECT_TRUE(function("function(value) { return value.prop == 'strval2'; }")
                  .call(rt, obj)
                  .getBool());
}

TEST_P(JSITest, ValueTest) {
  ABI38_0_0EXPECT_TRUE(checkValue(Value::undefined(), "undefined"));
  ABI38_0_0EXPECT_TRUE(checkValue(Value(), "undefined"));
  ABI38_0_0EXPECT_TRUE(checkValue(Value::null(), "null"));
  ABI38_0_0EXPECT_TRUE(checkValue(nullptr, "null"));

  ABI38_0_0EXPECT_TRUE(checkValue(Value(false), "false"));
  ABI38_0_0EXPECT_TRUE(checkValue(false, "false"));
  ABI38_0_0EXPECT_TRUE(checkValue(true, "true"));

  ABI38_0_0EXPECT_TRUE(checkValue(Value(1.5), "1.5"));
  ABI38_0_0EXPECT_TRUE(checkValue(2.5, "2.5"));

  ABI38_0_0EXPECT_TRUE(checkValue(Value(10), "10"));
  ABI38_0_0EXPECT_TRUE(checkValue(20, "20"));
  ABI38_0_0EXPECT_TRUE(checkValue(30, "30"));

  // rvalue implicit conversion
  ABI38_0_0EXPECT_TRUE(checkValue(String::createFromAscii(rt, "one"), "'one'"));
  // lvalue explicit copy
  String s = String::createFromAscii(rt, "two");
  ABI38_0_0EXPECT_TRUE(checkValue(Value(rt, s), "'two'"));

  {
    // rvalue assignment of trivial value
    Value v1 = 100;
    Value v2 = String::createFromAscii(rt, "hundred");
    v2 = std::move(v1);
    ABI38_0_0EXPECT_TRUE(v2.isNumber());
    ABI38_0_0EXPECT_EQ(v2.getNumber(), 100);
  }

  {
    // rvalue assignment of js heap value
    Value v1 = String::createFromAscii(rt, "hundred");
    Value v2 = 100;
    v2 = std::move(v1);
    ABI38_0_0EXPECT_TRUE(v2.isString());
    ABI38_0_0EXPECT_EQ(v2.getString(rt).utf8(rt), "hundred");
  }

  Object o = Object(rt);
  ABI38_0_0EXPECT_TRUE(function("function(value) { return typeof(value) == 'object'; }")
                  .call(rt, Value(rt, o))
                  .getBool());

  uint8_t utf8[] = "[null, 2, \"c\", \"emoji: \xf0\x9f\x86\x97\", {}]";

  ABI38_0_0EXPECT_TRUE(
      function("function (arr) { return "
               "Array.isArray(arr) && "
               "arr.length == 5 && "
               "arr[0] === null && "
               "arr[1] == 2 && "
               "arr[2] == 'c' && "
               "arr[3] == 'emoji: \\uD83C\\uDD97' && "
               "typeof arr[4] == 'object'; }")
          .call(rt, Value::createFromJsonUtf8(rt, utf8, sizeof(utf8) - 1))
          .getBool());

  ABI38_0_0EXPECT_TRUE(eval("undefined").isUndefined());
  ABI38_0_0EXPECT_TRUE(eval("null").isNull());
  ABI38_0_0EXPECT_TRUE(eval("true").isBool());
  ABI38_0_0EXPECT_TRUE(eval("false").isBool());
  ABI38_0_0EXPECT_TRUE(eval("123").isNumber());
  ABI38_0_0EXPECT_TRUE(eval("123.4").isNumber());
  ABI38_0_0EXPECT_TRUE(eval("'str'").isString());
  // "{}" returns undefined.  empty code block?
  ABI38_0_0EXPECT_TRUE(eval("({})").isObject());
  ABI38_0_0EXPECT_TRUE(eval("[]").isObject());
  ABI38_0_0EXPECT_TRUE(eval("(function(){})").isObject());

  ABI38_0_0EXPECT_EQ(eval("123").getNumber(), 123);
  ABI38_0_0EXPECT_EQ(eval("123.4").getNumber(), 123.4);
  ABI38_0_0EXPECT_EQ(eval("'str'").getString(rt).utf8(rt), "str");
  ABI38_0_0EXPECT_TRUE(eval("[]").getObject(rt).isArray(rt));

  ABI38_0_0EXPECT_EQ(eval("456").asNumber(), 456);
  ABI38_0_0EXPECT_THROW(eval("'word'").asNumber(), JSIException);
  ABI38_0_0EXPECT_EQ(
      eval("({1:2, 3:4})").asObject(rt).getProperty(rt, "1").getNumber(), 2);
  ABI38_0_0EXPECT_THROW(eval("'oops'").asObject(rt), JSIException);

  ABI38_0_0EXPECT_EQ(eval("['zero',1,2,3]").toString(rt).utf8(rt), "zero,1,2,3");
}

TEST_P(JSITest, EqualsTest) {
  ABI38_0_0EXPECT_TRUE(Object::strictEquals(rt, rt.global(), rt.global()));
  ABI38_0_0EXPECT_TRUE(Value::strictEquals(rt, 1, 1));
  ABI38_0_0EXPECT_FALSE(Value::strictEquals(rt, true, 1));
  ABI38_0_0EXPECT_FALSE(Value::strictEquals(rt, true, false));
  ABI38_0_0EXPECT_TRUE(Value::strictEquals(rt, false, false));
  ABI38_0_0EXPECT_FALSE(Value::strictEquals(rt, nullptr, 1));
  ABI38_0_0EXPECT_TRUE(Value::strictEquals(rt, nullptr, nullptr));
  ABI38_0_0EXPECT_TRUE(Value::strictEquals(rt, Value::undefined(), Value()));
  ABI38_0_0EXPECT_TRUE(Value::strictEquals(rt, rt.global(), Value(rt.global())));
  ABI38_0_0EXPECT_FALSE(Value::strictEquals(
      rt,
      std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::quiet_NaN()));
  ABI38_0_0EXPECT_FALSE(Value::strictEquals(
      rt,
      std::numeric_limits<double>::signaling_NaN(),
      std::numeric_limits<double>::signaling_NaN()));
  ABI38_0_0EXPECT_TRUE(Value::strictEquals(rt, +0.0, -0.0));
  ABI38_0_0EXPECT_TRUE(Value::strictEquals(rt, -0.0, +0.0));

  Function noop = Function::createFromHostFunction(
      rt,
      PropNameID::forAscii(rt, "noop"),
      0,
      [](const Runtime&, const Value&, const Value*, size_t) {
        return Value();
      });
  auto noopDup = Value(rt, noop).getObject(rt);
  ABI38_0_0EXPECT_TRUE(Object::strictEquals(rt, noop, noopDup));
  ABI38_0_0EXPECT_TRUE(Object::strictEquals(rt, noopDup, noop));
  ABI38_0_0EXPECT_FALSE(Object::strictEquals(rt, noop, rt.global()));
  ABI38_0_0EXPECT_TRUE(Object::strictEquals(rt, noop, noop));
  ABI38_0_0EXPECT_TRUE(Value::strictEquals(rt, Value(rt, noop), Value(rt, noop)));

  String str = String::createFromAscii(rt, "rick");
  String strDup = String::createFromAscii(rt, "rick");
  String otherStr = String::createFromAscii(rt, "morty");
  ABI38_0_0EXPECT_TRUE(String::strictEquals(rt, str, str));
  ABI38_0_0EXPECT_TRUE(String::strictEquals(rt, str, strDup));
  ABI38_0_0EXPECT_TRUE(String::strictEquals(rt, strDup, str));
  ABI38_0_0EXPECT_FALSE(String::strictEquals(rt, str, otherStr));
  ABI38_0_0EXPECT_TRUE(Value::strictEquals(rt, Value(rt, str), Value(rt, str)));
  ABI38_0_0EXPECT_FALSE(Value::strictEquals(rt, Value(rt, str), Value(rt, noop)));
  ABI38_0_0EXPECT_FALSE(Value::strictEquals(rt, Value(rt, str), 1.0));
}

TEST_P(JSITest, ExceptionStackTraceTest) {
  static const char invokeUndefinedScript[] =
      "function hello() {"
      "  var a = {}; a.log(); }"
      "function world() { hello(); }"
      "world()";
  std::string stack;
  try {
    rt.evaluateJavaScript(
        std::make_unique<StringBuffer>(invokeUndefinedScript), "");
  } catch (JSError& e) {
    stack = e.getStack();
  }
  ABI38_0_0EXPECT_NE(stack.find("world"), std::string::npos);
}

TEST_P(JSITest, PreparedJavaScriptSourceTest) {
  rt.evaluateJavaScript(std::make_unique<StringBuffer>("var q = 0;"), "");
  auto prep = rt.prepareJavaScript(std::make_unique<StringBuffer>("q++;"), "");
  ABI38_0_0EXPECT_EQ(rt.global().getProperty(rt, "q").getNumber(), 0);
  rt.evaluatePreparedJavaScript(prep);
  ABI38_0_0EXPECT_EQ(rt.global().getProperty(rt, "q").getNumber(), 1);
  rt.evaluatePreparedJavaScript(prep);
  ABI38_0_0EXPECT_EQ(rt.global().getProperty(rt, "q").getNumber(), 2);
}

TEST_P(JSITest, PreparedJavaScriptURLInBacktrace) {
  std::string sourceURL = "//PreparedJavaScriptURLInBacktrace/Test/URL";
  std::string throwingSource =
      "function thrower() { throw new Error('oops')}"
      "thrower();";
  auto prep = rt.prepareJavaScript(
      std::make_unique<StringBuffer>(throwingSource), sourceURL);
  try {
    rt.evaluatePreparedJavaScript(prep);
    FAIL() << "prepareJavaScript should have thrown an exception";
  } catch (ABI38_0_0facebook::jsi::JSError err) {
    ABI38_0_0EXPECT_NE(std::string::npos, err.getStack().find(sourceURL))
        << "Backtrace should contain source URL";
  }
}

namespace {

unsigned countOccurences(const std::string& of, const std::string& in) {
  unsigned occurences = 0;
  std::string::size_type lastOccurence = -1;
  while ((lastOccurence = in.find(of, lastOccurence + 1)) !=
         std::string::npos) {
    occurences++;
  }
  return occurences;
}

} // namespace

TEST_P(JSITest, JSErrorsArePropagatedNicely) {
  unsigned callsBeoreError = 5;

  Function sometimesThrows = function(
      "function sometimesThrows(shouldThrow, callback) {"
      "  if (shouldThrow) {"
      "    throw Error('Omg, what a nasty exception')"
      "  }"
      "  callback(callback);"
      "}");

  Function callback = Function::createFromHostFunction(
      rt,
      PropNameID::forAscii(rt, "callback"),
      0,
      [&sometimesThrows, &callsBeoreError](
          Runtime& rt, const Value& thisVal, const Value* args, size_t count) {
        return sometimesThrows.call(rt, --callsBeoreError == 0, args[0]);
      });

  try {
    sometimesThrows.call(rt, false, callback);
  } catch (JSError& error) {
    ABI38_0_0EXPECT_EQ(error.getMessage(), "Omg, what a nasty exception");
    ABI38_0_0EXPECT_EQ(countOccurences("sometimesThrows", error.getStack()), 6);

    // system JSC JSI does not implement host function names
    // ABI38_0_0EXPECT_EQ(countOccurences("callback", error.getStack(rt)), 5);
  }
}

TEST_P(JSITest, JSErrorsCanBeConstructedWithStack) {
  auto err = JSError(rt, "message", "stack");
  ABI38_0_0EXPECT_EQ(err.getMessage(), "message");
  ABI38_0_0EXPECT_EQ(err.getStack(), "stack");
}

TEST_P(JSITest, ScopeDoesNotCrashTest) {
  Scope scope(rt);
  Object o(rt);
}

TEST_P(JSITest, ScopeDoesNotCrashWhenValueEscapes) {
  Value v;
  Scope::callInNewScope(rt, [&]() {
    Object o(rt);
    o.setProperty(rt, "a", 5);
    v = std::move(o);
  });
  ABI38_0_0EXPECT_EQ(v.getObject(rt).getProperty(rt, "a").getNumber(), 5);
}

// Verifies you can have a host object that emulates a normal object
TEST_P(JSITest, HostObjectWithValueMembers) {
  class Bag : public HostObject {
   public:
    Bag() = default;

    const Value& operator[](const std::string& name) const {
      auto iter = data_.find(name);
      if (iter == data_.end()) {
        return undef_;
      }
      return iter->second;
    }

   protected:
    Value get(Runtime& rt, const PropNameID& name) override {
      return Value(rt, (*this)[name.utf8(rt)]);
    }

    void set(Runtime& rt, const PropNameID& name, const Value& val) override {
      data_.emplace(name.utf8(rt), Value(rt, val));
    }

    Value undef_;
    std::map<std::string, Value> data_;
  };

  auto sharedBag = std::make_shared<Bag>();
  auto& bag = *sharedBag;
  Object jsbag = Object::createFromHostObject(rt, std::move(sharedBag));
  auto set = function(
      "function (o) {"
      "  o.foo = 'bar';"
      "  o.count = 37;"
      "  o.nul = null;"
      "  o.iscool = true;"
      "  o.obj = { 'foo': 'bar' };"
      "}");
  set.call(rt, jsbag);
  auto checkFoo = function("function (o) { return o.foo === 'bar'; }");
  auto checkCount = function("function (o) { return o.count === 37; }");
  auto checkNul = function("function (o) { return o.nul === null; }");
  auto checkIsCool = function("function (o) { return o.iscool === true; }");
  auto checkObj = function(
      "function (o) {"
      "  return (typeof o.obj) === 'object' && o.obj.foo === 'bar';"
      "}");
  // Check this looks good from js
  ABI38_0_0EXPECT_TRUE(checkFoo.call(rt, jsbag).getBool());
  ABI38_0_0EXPECT_TRUE(checkCount.call(rt, jsbag).getBool());
  ABI38_0_0EXPECT_TRUE(checkNul.call(rt, jsbag).getBool());
  ABI38_0_0EXPECT_TRUE(checkIsCool.call(rt, jsbag).getBool());
  ABI38_0_0EXPECT_TRUE(checkObj.call(rt, jsbag).getBool());

  // Check this looks good from c++
  ABI38_0_0EXPECT_EQ(bag["foo"].getString(rt).utf8(rt), "bar");
  ABI38_0_0EXPECT_EQ(bag["count"].getNumber(), 37);
  ABI38_0_0EXPECT_TRUE(bag["nul"].isNull());
  ABI38_0_0EXPECT_TRUE(bag["iscool"].getBool());
  ABI38_0_0EXPECT_EQ(
      bag["obj"].getObject(rt).getProperty(rt, "foo").getString(rt).utf8(rt),
      "bar");
}

TEST_P(JSITest, DecoratorTest) {
  struct Count {
    // init here is just to show that a With type does not need to be
    // default constructible.
    explicit Count(int init) : count(init) {}

    // Test optional before method.

    void after() {
      ++count;
    }

    int count;
  };

  static constexpr int kInit = 17;

  class CountRuntime final : public WithRuntimeDecorator<Count> {
   public:
    explicit CountRuntime(std::unique_ptr<Runtime> rt)
        : WithRuntimeDecorator<Count>(*rt, count_),
          rt_(std::move(rt)),
          count_(kInit) {}

    int count() {
      return count_.count;
    }

   private:
    std::unique_ptr<Runtime> rt_;
    Count count_;
  };

  CountRuntime crt(factory());

  crt.description();
  ABI38_0_0EXPECT_EQ(crt.count(), kInit + 1);

  crt.global().setProperty(crt, "o", Object(crt));
  ABI38_0_0EXPECT_EQ(crt.count(), kInit + 6);
}

TEST_P(JSITest, MultiDecoratorTest) {
  struct Inc {
    void before() {
      ++count;
    }

    // Test optional after method.

    int count = 0;
  };

  struct Nest {
    void before() {
      ++nest;
    }

    void after() {
      --nest;
    }

    int nest = 0;
  };

  class MultiRuntime final : public WithRuntimeDecorator<WithTuple<Inc, Nest>> {
   public:
    explicit MultiRuntime(std::unique_ptr<Runtime> rt)
        : WithRuntimeDecorator<WithTuple<Inc, Nest>>(*rt, tuple_),
          rt_(std::move(rt)) {}

    int count() {
      return std::get<0>(tuple_).count;
    }
    int nest() {
      return std::get<1>(tuple_).nest;
    }

   private:
    std::unique_ptr<Runtime> rt_;
    WithTuple<Inc, Nest> tuple_;
  };

  MultiRuntime mrt(factory());

  Function expectNestOne = Function::createFromHostFunction(
      mrt,
      PropNameID::forAscii(mrt, "expectNestOne"),
      0,
      [](Runtime& rt, const Value& thisVal, const Value* args, size_t count) {
        MultiRuntime* funcmrt = dynamic_cast<MultiRuntime*>(&rt);
        ABI38_0_0EXPECT_NE(funcmrt, nullptr);
        ABI38_0_0EXPECT_EQ(funcmrt->count(), 3);
        ABI38_0_0EXPECT_EQ(funcmrt->nest(), 1);
        return Value::undefined();
      });

  expectNestOne.call(mrt);

  ABI38_0_0EXPECT_EQ(mrt.count(), 3);
  ABI38_0_0EXPECT_EQ(mrt.nest(), 0);
}

TEST_P(JSITest, SymbolTest) {
  if (!rt.global().hasProperty(rt, "Symbol")) {
    // Symbol is an es6 feature which doesn't exist in older VMs.  So
    // the tests which might be elsewhere are all here so they can be
    // skipped.
    return;
  }

  // ObjectTest
  eval("x = {1:2, 'three':Symbol('four')}");
  Object x = rt.global().getPropertyAsObject(rt, "x");
  ABI38_0_0EXPECT_EQ(x.getPropertyNames(rt).size(rt), 2);
  ABI38_0_0EXPECT_TRUE(x.hasProperty(rt, "three"));
  ABI38_0_0EXPECT_EQ(
      x.getProperty(rt, "three").getSymbol(rt).toString(rt), "Symbol(four)");

  // ValueTest
  ABI38_0_0EXPECT_TRUE(eval("Symbol('sym')").isSymbol());
  ABI38_0_0EXPECT_EQ(eval("Symbol('sym')").getSymbol(rt).toString(rt), "Symbol(sym)");

  // EqualsTest
  ABI38_0_0EXPECT_FALSE(Symbol::strictEquals(
      rt,
      eval("Symbol('a')").getSymbol(rt),
      eval("Symbol('a')").getSymbol(rt)));
  ABI38_0_0EXPECT_TRUE(Symbol::strictEquals(
      rt,
      eval("Symbol.for('a')").getSymbol(rt),
      eval("Symbol.for('a')").getSymbol(rt)));
  ABI38_0_0EXPECT_FALSE(
      Value::strictEquals(rt, eval("Symbol('a')"), eval("Symbol('a')")));
  ABI38_0_0EXPECT_TRUE(Value::strictEquals(
      rt, eval("Symbol.for('a')"), eval("Symbol.for('a')")));
  ABI38_0_0EXPECT_FALSE(Value::strictEquals(rt, eval("Symbol('a')"), eval("'a'")));
}

INSTANTIATE_TEST_CASE_P(
    Runtimes,
    JSITest,
    ::testing::ValuesIn(runtimeGenerators()));