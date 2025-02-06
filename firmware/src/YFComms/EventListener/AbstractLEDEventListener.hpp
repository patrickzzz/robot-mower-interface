#pragma once

namespace YFComms {
    class AbstractLEDEventListener {
    public:
        virtual ~AbstractLEDEventListener() = default;
        virtual void onLEDStateChangeEvent() = 0;
    };
} // namespace YFComms