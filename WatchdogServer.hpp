#ifndef WATCHDOG_SERVER_HPP
#define WATCHDOG_SERVER_HPP

#include<vector>

#include"/home/pi/Shared/IPCommunicationsLib/0.0.0/mailbox.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/ILogger.hpp"
#include"/home/pi/Shared/ProcessLib/0.0.0/IProcess.hpp"

/**
 * @brief Watchdog server side class.
 * 
 */
class WatchdogServer
{
    ///////////////////////////////////
    private:
    ///////////////////////////////////

    void defaultInit();

    /// Pointer to a logger
    ILogger* logger;

    /// WatchdogServer mailbox used to communicate with WatchdogClients
    Mailbox mailbox;

    /**
     * @brief RTO = Request TimeOut
     * 
     * Used as a timer to `timedReceive()` from Mailbox class. \n
     * When time runs out (without answer from destination mailbox that sent message was received) \n
     * WatchdogClient then requests system crash (major error). \n
     * 
     */
    int RTO; // Waiting time for answer

    /**
     * @brief Sleep time between bursts of "PING" messages to attached WatchdogClients
     * 
     */
    int SleepTime; // Time between two rounds of pings

    /**
     * @brief TTL = Time To Live
     * 
     * BaseTTL specifies how many times the WatchdogClient \n
     * will resend messages to destination via a mailbox \n
     * without acknowledgement, before requesting system crash (major error). \n
     * 
     */
    int BaseTTL; // Base number of ping tries before crashing
    
    std::vector<IProcess*> processes;
    std::vector<int> PIDs;
    std::vector<bool> syncStatus;

    /// Logger flag. Used in destructor to deallocate NulLogger if one was created in constructor (if logger was not specified explicitly)
    bool loggerOwnership = false;

    /// Used to create a new NulLogger object when logger isn't specified in constructor.
    void CreateNulLogger();

    /**
     * @brief Pings WatchdogClient via a mailbox and listens for response.
     * 
     * @param destination MailboxReference which references the WatchdogClient's mailbox
     * @return true WatchdogClient responded with acknowledgement
     * @return false WatchdogClient did not respond
     */
    bool TryToContact(MailboxReference& destination);

    void CreateProcess(IProcess* p_process);

    ///////////////////////////////////
    public:
    ///////////////////////////////////

    /**
     * @brief Construct a new Watchdog Server object without logger and default parameter values
     * 
     * Check class implementation for default parameter values
     * 
     */
    WatchdogServer();

    /**
     * @brief Construct a new Watchdog Server object with logger and default parameter values
     * 
     * Check class implementation for default parameter values
     * 
     * @param _logger Pointer to a logger
     */
    WatchdogServer(ILogger* _logger);

    /**
     * @brief Construct a new Watchdog Server object with logger and specified parameter values
     * 
     * @param _logger Pointer to a logger
     * @param _RTO RTO = Request TimeOut
     * @param _PauseTime Pause time between two bursts of "PING" messages
     * @param _BaseTTL TTL = Time To Live
     */
    WatchdogServer(ILogger* _logger, int _RTO, int _PauseTime, int _BaseTTL);
    
    /**
     * @brief Destroy the Watchdog Server object
     * 
     */
    ~WatchdogServer();

    /**
     * @brief Send "CRASH" directive to all attached processes to terminate themselves.
     * 
     * Watchdog server is registered as a crash initiator.
     * 
     */
    void CrashSystem();

    /**
     * @brief Send "CRASH" directive to all attached processes to terminate themselves.
     * 
     * `inititator` is registered as a crash initiator. 
     * 
     * @param initiator Name of the process which initiated the crash
     */
    void CrashSystem(const std::string& initiator);

    /**
     * @brief Send "PING" control message to the `destination` (WIP)
     * 
     * @param destination MailboxReference used as a destination for the control message
     */
    void Ping(MailboxReference& destination);


    void Decode(const mailbox_message& message);

    /**
     * @brief Wait for `SleepTime` while listening for requests.
     * 
     * After request gets processed, `Sleep()` gets interrupted.
     * 
     */
    void Sleep();

    /**
     * @brief Listen for requests
     * 
     * @return mailbox_message Request in a mailbox_message format
     */
    mailbox_message Listen();

    /**
     * @brief Start WatchdogServer
     * 
     * Starts "PING"ing attached processes every `SleepTime` seconds, and listening to requests.
     * 
     */
    void Start();

    /**
     * @brief Kills all attached processes (WIP)
     * 
     */
    void KillAll();

    void AttachProcess(IProcess* p_process);

    void Sweep();

    void StartMonitoring();
    
    void StartAndSynchronize();
};

#endif