// Copyright (c) 2020 Mapless AI, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "rcutils/cmdline_parser.h"
#include "rclcpp_components/register_node_macro.hpp"

#include "sw_watchdog/msg/heartbeat.hpp"
#include "sw_watchdog/visibility_control.h"

using namespace std::chrono_literals;

namespace
{

void print_usage()
{
    std::cout <<
        "Usage: simple_heartbeat period [-h]\n\n"
        "required arguments:\n"
        "\tperiod: Period in positive integer milliseconds of the heartbeat signal.\n"
        "optional arguments:\n"
        "\t-h : Print this help message." <<
        std::endl;
}

} // anonymous ns

namespace sw_watchdog
{

/**
 * A class that publishes heartbeats at a fixed frequency with the header set to current time.
 */
class SimpleHeartbeat : public rclcpp::Node
{
public:
    SW_WATCHDOG_PUBLIC
    explicit SimpleHeartbeat(const rclcpp::NodeOptions& options)
        : Node("simple_heartbeat", options)
    {
        // Parse node arguments
        const std::vector<std::string>& args = this->get_node_options().arguments();
        std::vector<char *> cargs;
        cargs.reserve(args.size());
        for(size_t i = 0; i < args.size(); ++i)
            cargs.push_back(const_cast<char*>(args[i].c_str()));

        if(args.size() < 1 || rcutils_cli_option_exist(&cargs[0], &cargs[0] + cargs.size(), "-h")) {
            print_usage();
            // TODO: Update the rclcpp_components template to be able to handle
            // exceptions. Raise one here, so stack unwinding happens gracefully.
            std::exit(0);
        }

        std::chrono::milliseconds heartbeat_period(std::stoul(args[1]));

        publisher_ = this->create_publisher<sw_watchdog::msg::Heartbeat>("heartbeat", 1);
        timer_ = this->create_wall_timer(heartbeat_period,
                                         std::bind(&SimpleHeartbeat::timer_callback, this));
    }

private:
    void timer_callback()
    {
        auto message = sw_watchdog::msg::Heartbeat();
        rclcpp::Time now = this->get_clock()->now();
        message.stamp = now;
        RCLCPP_INFO(this->get_logger(), "Publishing heartbeat, sent at [%f]", now.seconds());
        publisher_->publish(message);
    }
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<sw_watchdog::msg::Heartbeat>::SharedPtr publisher_;
};

}  // namespace sw_watchdog

RCLCPP_COMPONENTS_REGISTER_NODE(sw_watchdog::SimpleHeartbeat)