#ifndef WATCHDOG_CLIENT_HPP
#define WATCHDOG_CLIENT_HPP

#include"/home/pi/Shared/LoggerLib/0.0.0/ILogger.hpp"
#include"/home/pi/Shared/SharedMemoryLib/0.0.0/SharedMemory.hpp"
#include"/home/pi/Shared/IPCommunicationsLib/0.0.0/mailbox.hpp"

#include"/home/pi/Shared/ProcessLib/0.0.0/ProcessStatus.hpp"

#include<string>

/**
 * @brief Watchdog client side class. Every process should implement one.
 * 
 */
class WatchdogClient
{
    private:
    /// Shared memory object with ProcessStatus objects as basic shared memory units.
    SharedMemory<ProcessStatus> watchdogRegister;

    /// Offset from shared memory base address pointing to the designated WatchdogClient status slot
    int offset = -1;

    /// Pointer to a logger
    ILogger* p_logger = nullptr;

    /// Checks if the main process signaled the TERMINATE signal
    bool MarkedForTermination();

    MailboxReference watchdogServer;

    std::string name;

    bool SetStatus(ProcessStatus status);
    void Reserve();
    void Release();

    public:
    /**
     * @brief Construct a new Watchdog Client object
     * 
     * @param _name Name of the current process/thread - must be globally UNIQUE
     * @param shmName String identifier (name) of shared memory
     * @param watchdogServerMailboxIdentifier String identifier (name) of mailbox reference to watchdog server mailbox.
     * @param _p_logger Pointer to a parent ILogger which is used as a logger. - NULL SAFE
     */
    WatchdogClient(const std::string& _name,
                   const std::string& shmName,
                   const std::string& watchdogServerMailboxIdentifier = "watchdogServer",
                   ILogger* _p_logger = NulLogger::getInstance());

    /**
     * @brief Destroy the Watchdog Client object
     * 
     */
    ~WatchdogClient();

    /**
     * @brief Sets the status to IDLE. RETURNS FALSE IF PROCESS IS MARKED FOR TERMINATION!
     * 
     * @return true Success
     * @return false Process is marked for TERMINATION
     */
    bool Pet();

    /**
     * @brief Sets the status to BUSY. RETURNS FALSE IF PROCESS IS MARKED FOR TERMINATION!
     * 
     * @return true Success
     * @return false Process is marked for TERMINATION
     */
    bool Busy();

    /// Signals to the main process that the attached process is or is in process of being TERMINATED
    void Terminate();
};

#endif