/* Created by Apehaenger on 2025-02-07.
 * Copyright (c) 2024 Jörg Ebeling for Robot-Interface of patrickzzz. All rights reserved.
 *
 * This file is part of the robot-interface project at https://github.com/patrickzzz/robot-interface.
 *
 * This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * Feel free to use the design in your private/educational projects, but don't try to sell the design or products based on it without getting our consent first.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <variant>

#include "etl/queue.h"
#include "etl/unordered_map.h"
#include "etl/vector.h"

// namespace broker { FIXME: @patrickzzz No real requirement for a separate namespace for me, if we don't need a separate debug TAG and you're fine with the next constexpr

// To be heap save, we use a couple of ETL containers for which we need some max. boundaries
constexpr size_t MAX_SUBSCRIBERS = 10;  // Max number of subscribers per topic
constexpr size_t MAX_MESSAGES = 20;     // Max number of messages in the queue

/**
 * @brief EventBroker will handle Mainboard, CoverUI or WebApp requests/responses.
 * Each Device (Mainboard, CoverUI, ...) may publish or subscribe to the defined topics.
 * By this we can handle the different direction for LEDs/Button more easy and flexible.
 * In addition we've higher flexibility for possible required HighLevel messages.
 */
class EventBroker {
   public:
    // Yet known/implemented topics, ever class may subscribe to
    enum class Topic {
        VERSION,    // Version info. Direction might depend on device
        LED,        // Generic Mainboard => CoverUI message
        BUTTON,     // YF-CoverUI => Mainboard message? For OM this is probably different
        EMERGENCY,  // Not sure if we need that. OM needs it, but if CoverUI send emergencies as Button, OM-MainboardDriver has to convert
        RAIN,       // Not sure of required
        MAX
    };
    // Ugly, but required because we use an "enum class"
    static constexpr size_t MAX_TOPICS = static_cast<size_t>(Topic::MAX);

    // Subscriber Type is currently mainly for informational purposes like debugging,
    // but also as preparation for the case that we need to act differently on specific types of versions.
    enum class SubscriberType {
        COVERUI,
        MAINBOARD,
        WEBAPP
    };

    // Subscriber Info. Not yet required but see comment of previous enum
    struct SubInfo {
        SubscriberType type;
        uint16_t version;  // Multiplied by 100. Means: Version string "12.34" becomes uint 1234
    };

    // ----- Individual message types for each topic -----

    // We need a generalized LED enum because YF and OM use different LED numbers.
    // I simply took the ones from YF here, because I assume you defined them based on the YF protocol.
    //   OM LED ID's need to be handled different anyway, so OM-MB-Driver need to adapt accordingly.
    // Sorry for the naming clash with the proposed LED.h yesterday where I named them "Names" (which has been a very poor name)
    // Exception: Hour-LEDs. Changed them again back to 14-44 because neither Mainboard nor EventBroker knows which LED is labeled. Only CoverUIDriver may know probably.
    enum class LEDID {
        LIFTED,
        SIGNAL,
        BATTERY_LOW,
        CHARGING,
        S1,
        S2,
        LOCK,
        WIFI,
        GPS,
        BACKGROUND,
        HOURS_14,
        HOURS_24,
        HOURS_34,
        HOURS_44,
        HOURS_TEN,  // <---- UUUPS! @patrickzzz: Do you have a CoverUI with a 5th HR LEDs?
        DAY_MON,
        DAY_TUE,
        DAY_WED,
        DAY_THR,
        DAY_FRI,
        DAY_SAT,
        DAY_SUN,
        MAX
    };

    // Possible led states
    // Same here, simply took the one from your YF code. OM has different ones and OM-MB-Driver needs to convert
    enum class LEDState {
        OFF = 0x00,
        ON = 0x10,
        FLASH_SLOW = 0x20,
        FLASH_FAST = 0x22
    };

    struct LEDMessage {
        LEDID id;
        LEDState state;
    };

    struct ButtonMessage {
        // TBD: Only for demonstration
    };

    struct VersionMessage {
        // TBD: Only for demonstration
    };

    // Message structure that can hold all message types
    // Did not know that std::variant before asking DeepSeek ;-/
    using MessageVariant = std::variant<LEDMessage, ButtonMessage, VersionMessage>;

    // The message which get be transferred between the publisher and subscriber
    struct Message {
        Topic topic;
        MessageVariant payload;
    };

    // This is the callback type the subscriber need to implement and inform
    using Callback = std::function<void(Topic topic, const MessageVariant& message, SubInfo subInfo)>;

    // Singleton implementation because in my opinion we alway have one and only one EventBroker.
    // By this we can simply get in touch with a simple EventBroker::getInstance() from everywhere.
    static EventBroker& getInstance();

    // Std. deletion on remove
    EventBroker(const EventBroker&) = delete;
    EventBroker& operator=(const EventBroker&) = delete;

    // Subscribe to a topic
    // MainboardDriver i.e. requires to register his subscribe callback to get Button Events, or
    // CoverUIDriver need to register his callback for handling LED messages
    void subscribe(Topic topic, Callback callback, SubInfo subInfo);

    // Publish a message to a topic
    // Get called i.e. by MainboardDriverOM via EventBroker::getInstance().publish(...) on a Set-LED packet
    // or by CoverUI when a button got pressed
    void publish(Topic topic, const MessageVariant& message);

    // This should become a RTOS Queue Task and shall relay the incoming publish message to all subscribers
    void run();

   private:
    EventBroker() = default;
    ~EventBroker() = default;

    // Subscriber information (callback + subscriber info)
    struct SubscriberInfo {
        Callback callback;
        SubInfo subInfo;
    };

    // Management of subscribers
    etl::unordered_map<Topic, etl::vector<SubscriberInfo, MAX_SUBSCRIBERS>, MAX_TOPICS> subscribers_;
    etl::queue<Message, MAX_MESSAGES> messageQueue_;
    std::mutex mutex_;
    std::condition_variable condition_;
};
//}