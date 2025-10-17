/**
 * Copyright (c) 2011-2025 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_NODE_SESSIONS_SESSIONS_HPP
#define LIBBITCOIN_NODE_SESSIONS_SESSIONS_HPP

#include <bitcoin/node/sessions/session.hpp>
#include <bitcoin/node/sessions/session_inbound.hpp>
#include <bitcoin/node/sessions/session_manual.hpp>
#include <bitcoin/node/sessions/session_outbound.hpp>
#include <bitcoin/node/sessions/session_peer.hpp>
#include <bitcoin/node/sessions/session_server.hpp>
#include <bitcoin/node/sessions/session_tcp.hpp>

namespace libbitcoin {
namespace node {

// Alias server sessions derived from session_tcp.
using session_web = session_server<protocol_web>;
using session_explore = session_server<protocol_explore>;
using session_websocket = session_server<protocol_websocket>;
using session_bitcoind = session_server<protocol_bitcoind>;
using session_electrum = session_server<protocol_electrum>;
using session_stratum_v1 = session_server<protocol_stratum_v1>;
using session_stratum_v2 = session_server<protocol_stratum_v2>;

} // namespace network
} // namespace libbitcoin

#endif
