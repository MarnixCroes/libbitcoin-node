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
#include "executor.hpp"
#include "localize.hpp"

#include <algorithm>
#include <atomic>
#include <csignal>
#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <boost/format.hpp>
#include <bitcoin/node.hpp>

namespace libbitcoin {
namespace node {

using boost::format;
using system::config::printer;
using namespace network;
using namespace system;
using namespace std::chrono;
using namespace std::placeholders;

// Runner.
// ----------------------------------------------------------------------------

void executor::subscribe_connect()
{
    node_->subscribe_connect([&](const code&, const channel::ptr&)
        {
            log_.write(levels::verbose) <<
                "{in:" << node_->inbound_channel_count() << "}"
                "{ch:" << node_->channel_count() << "}"
                "{rv:" << node_->reserved_count() << "}"
                "{nc:" << node_->nonces_count() << "}"
                "{ad:" << node_->address_count() << "}"
                "{ss:" << node_->stop_subscriber_count() << "}"
                "{cs:" << node_->connect_subscriber_count() << "}."
                << std::endl;

            return true;
        },
        [&](const code&, uintptr_t)
        {
            // By not handling it is possible stop could fire before complete.
            // But the handler is not required for termination, so this is ok.
            // The error code in the handler can be used to differentiate.
        });
}

void executor::subscribe_close()
{
    node_->subscribe_close([&](const code&)
        {
            log_.write(levels::verbose) <<
                "{in:" << node_->inbound_channel_count() << "}"
                "{ch:" << node_->channel_count() << "}"
                "{rv:" << node_->reserved_count() << "}"
                "{nc:" << node_->nonces_count() << "}"
                "{ad:" << node_->address_count() << "}"
                "{ss:" << node_->stop_subscriber_count() << "}"
                "{cs:" << node_->connect_subscriber_count() << "}."
                << std::endl;

            return false;
        },
        [&](const code&, size_t)
        {
            // By not handling it is possible stop could fire before complete.
            // But the handler is not required for termination, so this is ok.
            // The error code in the handler can be used to differentiate.
        });
}

bool executor::do_run()
{
    if (!metadata_.configured.log.path.empty())
        database::file::create_directory(metadata_.configured.log.path);

    // Hold sinks in scope for the length of the run.
    auto log = create_log_sink();
    auto events = create_event_sink();
    if (!log || !events)
    {
        logger(BN_LOG_INITIALIZE_FAILURE);
        return false;
    }

    subscribe_log(log);
    subscribe_events(events);
    subscribe_capture();
    logger(BN_LOG_HEADER);

    if (check_store_path())
    {
        auto ec = open_store_coded(true);
        if (ec == database::error::flush_lock)
        {
            ec = error::success;
            if (!restore_store(true))
                ec = database::error::integrity;
        }

        if (ec)
        {
            stopper(BN_NODE_STOPPED);
            return false;
        }
    }
    else if (!check_store_path(true) || !create_store(true))
    {
        stopper(BN_NODE_STOPPED);
        return false;
    }

    dump_body_sizes();
    dump_records();
    dump_buckets();
    ////logger(BN_MEASURE_PROGRESS_START);
    ////dump_progress();

    // Stopped by stopper.
    capture_.start();
    dump_version();
    dump_hardware();
    dump_options();
    logger(BN_NODE_INTERRUPT);

    // Create node.
    metadata_.configured.network.initialize();
    node_ = std::make_shared<full_node>(query_, metadata_.configured, log_);

    // Subscribe node.
    subscribe_connect();
    subscribe_close();

    // Start network.
    logger(BN_NETWORK_STARTING);
    node_->start(std::bind(&executor::handle_started, this, _1));

    // Wait on signal to stop node (<ctrl-c>).
    stopping_.get_future().wait();
    toggle_.at(levels::protocol) = false;
    logger(BN_NETWORK_STOPPING);

    // Stop network (if not already stopped by self).
    node_->close();

    // Sizes and records change, buckets don't.
    dump_body_sizes();
    dump_records();
    ////logger(BN_MEASURE_PROGRESS_START);
    ////dump_progress();

    if (!close_store(true))
    {
        stopper(BN_NODE_STOPPED);
        return false;
    }

    stopper(BN_NODE_STOPPED);
    return true; 
}

} // namespace node
} // namespace libbitcoin
