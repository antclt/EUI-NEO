#pragma once

#include "core/platform/network.h"

namespace eui::network {

using core::network::TextResult;
using core::network::cacheFilePath;
using core::network::consumeAnyTextReady;
using core::network::downloadUrlToFile;
using core::network::downloadUrlToString;
using core::network::isHttpUrl;
using core::network::postNetworkReadyEvent;
using core::network::requestText;
using core::network::shutdown;
using core::network::textResult;

} // namespace eui::network
