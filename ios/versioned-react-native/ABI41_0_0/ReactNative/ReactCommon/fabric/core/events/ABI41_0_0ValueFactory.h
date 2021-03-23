/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>

#include <ABI41_0_0jsi/ABI41_0_0jsi.h>

namespace ABI41_0_0facebook {
namespace ABI41_0_0React {

using ValueFactory = std::function<jsi::Value(jsi::Runtime &runtime)>;

} // namespace ABI41_0_0React
} // namespace ABI41_0_0facebook