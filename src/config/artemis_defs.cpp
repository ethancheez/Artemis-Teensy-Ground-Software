#include "config/artemis_defs.h"

const char *current_sen_names[ARTEMIS_CURRENT_SENSOR_COUNT] = {"solar_panel_1", "solar_panel_2", "solar_panel_3", "solar_panel_4", "battery_board"};
const char *temp_sen_names[ARTEMIS_TEMP_SENSOR_COUNT] = {"obc", "pdu", "battery board", "solar pannel 1", "solar panel 2", "solar panel 3", "solar panel 4"};

vector<struct thread_struct> thread_list;

std::map<string, NODES> NodeType = {
    {"ground", NODES::GROUND_NODE_ID},
    {"artemis_teensy", NODES::TEENSY_NODE_ID},
    {"artemis_rpi", NODES::RPI_NODE_ID},
};

std::map<string, PDU_CMD> PDUType = {
    {"3v3_1", PDU_CMD::SW_3V3_1},
    {"3v3_2", PDU_CMD::SW_3V3_2},
    {"5v_1", PDU_CMD::SW_5V_1},
    {"5v_2", PDU_CMD::SW_5V_2},
    {"5v_3", PDU_CMD::SW_5V_3},
    {"5v_4", PDU_CMD::SW_5V_4},
    {"12v", PDU_CMD::SW_12V},
    {"vbatt", PDU_CMD::VBATT},
    {"wdt", PDU_CMD::WDT},
    {"hbridge1", PDU_CMD::HBRIDGE1},
    {"hbridge2", PDU_CMD::HBRIDGE2},
    {"burn", PDU_CMD::BURN},
    {"burn1", PDU_CMD::BURN1},
    {"burn2", PDU_CMD::BURN2},
    {"rpi", PDU_CMD::RPI},
    {"all", PDU_CMD::All},
};

// Mutex for Command Queues
Threads::Mutex main_queue_mtx;
Threads::Mutex rfm23_queue_mtx;

// Command Queues
queue<PacketComm> main_queue;
queue<PacketComm> rfm23_queue;

// Other Mutex
Threads::Mutex spi1_mtx;

// Utility Functions
int kill_thread(uint8_t channel_id)
{
    for (auto it = thread_list.begin(); it != thread_list.end(); it++)
    {
        if (it->channel_id == channel_id)
        {
            int ret = it->thread_id;
            threads.kill(it->thread_id);
            thread_list.erase(it);
            return ret;
        }
    }
    return -1;
}

// Thread-safe way of pushing onto the packet queue
int32_t PushQueue(PacketComm &packet, queue<PacketComm> &queue, Threads::Mutex &mtx)
{
    Threads::Scope lock(mtx);
    if (queue.size() >= MAXQUEUESIZE)
    {
        queue.pop();
    }
    queue.push(packet);
    return 1;
}
// Thread-safe way of pulling from the packet queue
int32_t PullQueue(PacketComm &packet, queue<PacketComm> &queue, Threads::Mutex &mtx)
{
    Threads::Scope lock(mtx);
    if (queue.size() > 0)
    {
        packet = queue.front();
        queue.pop();
        return 1;
    }
    else
    {
        return 0;
    }
}
