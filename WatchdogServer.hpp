#ifndef WATCHDOG_SERVER_HPP
#define WATCHDOG_SERVER_HPP

#include<vector>

#include"/home/pi/Shared/IPCommunicationsLib/0.0.0/mailbox.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/ILogger.hpp"
#include"/home/pi/Shared/SharedMemoryLib/0.0.0/SharedMemory.hpp"

#include"/home/pi/Shared/ProcessLib/0.0.0/ProcessStatus.hpp"

/**
 * @brief Watchdog server side class.
 * 
 */
class WatchdogServer
{
    private:

    unsigned int SleepTime;

    ILogger* p_logger = nullptr;

    Mailbox watchdogServerMailbox;

    SharedMemory<ProcessStatus> currentStatus;
    std::vector<ProcessStatus> previousStatus;
    std::vector<std::string> processes;

    int BaseTTL;
    std::vector<int> TTL;

    int Reserve();
    void CheckAll();
    void Check(unsigned int offset);
    void ResetTTL(unsigned int offset);
    void DecrementTTL(unsigned int offset);
    void Sleep();
    void Setup();
    void CheckMessages();
    bool ListenForMessage();

    bool terminated = false;

    public:

    WatchdogServer() = delete;
    WatchdogServer(const std::string& shmIdentifier,
                   int shmLength,
                   int _BaseTTL = 3,
                   ILogger* _p_logger = nullptr,
                   const std::string& mailboxIdentifier = "watchdogServer",
                   unsigned int _SleepTime = 1);
    ~WatchdogServer();

    void Patrol();
    bool hasTerminated() const;
    void Terminate();
};

#endif