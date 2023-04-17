#ifndef _ARTEMIS_CHANNELS_H
#define _ARTEMIS_CHANNELS_H

#include "config/artemis_defs.h"

namespace Artemis
{
    namespace Teensy
    {
        namespace Channels
        {
            // Channel IDs
            enum Channel_ID : uint8_t
            {
                NONE,
                RFM23_CHANNEL,
                PDU_CHANNEL,
                RPI_CHANNEL,
            };

            void rfm23_channel();
        }
    }
}

#endif // _ARTEMIS_CHANNELS_H
