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
#include <bitcoin/node/protocols/protocol_observer.hpp>

#include <bitcoin/network.hpp>
#include <bitcoin/node/define.hpp>

namespace libbitcoin {
namespace node {

#define CLASS protocol_observer

using namespace std::placeholders;

// Shared pointers required for lifetime in handler parameters.
BC_PUSH_WARNING(SMART_PTR_NOT_NEEDED)
BC_PUSH_WARNING(NO_VALUE_OR_CONST_REF_SHARED_PTR)

void protocol_observer::start() NOEXCEPT
{
    BC_ASSERT(stranded());

    if (started())
        return;

    // Events subscription is asynchronous, events may be missed.
    subscribe_events(BIND(handle_event, _1, _2, _3),
        BIND(handle_complete, _1, _2));

    protocol::start();
}

// protected
void protocol_observer::handle_complete(const code& ec, object_key) NOEXCEPT
{
    POST(do_handle_complete, ec);
}

// private
void protocol_observer::do_handle_complete(const code& ec) NOEXCEPT
{
    BC_ASSERT(stranded());

    if (stopped(ec))
        unsubscribe_events();
}

// If this is invoked before do_handle_complete then it will unsubscribe.
void protocol_observer::stopping(const code& ec) NOEXCEPT
{
    BC_ASSERT(stranded());
    unsubscribe_events();
    protocol::stopping(ec);
}

bool protocol_observer::handle_event(const code&, chase event_,
    event_value) NOEXCEPT
{
    // Do not pass ec to stopped as it is not a call status.
    if (stopped())
        return false;

    switch (event_)
    {
        case chase::suspend:
        {
            stop(error::suspended_channel);
            break;
        }
        case chase::stop:
        {
            return false;
        }
        default:
        {
            break;
        }
    }

    return true;
}

BC_POP_WARNING()
BC_POP_WARNING()

} // namespace node
} // namespace libbitcoin
