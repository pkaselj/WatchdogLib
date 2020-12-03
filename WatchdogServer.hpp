#ifndef WATCHDOG_SERVER_HPP
#define WATCHDOG_SERVER_HPP

#include<vector>

#include"/home/pi/Shared/IPCommunicationsLib/0.0.0/mailbox.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/ILogger.hpp"

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
    
    /// Vector of MailboxReferences of attached WatchdogClients
    std::vector<MailboxReference> attachedClients;

    /// Logger flag. Used in destructor to deallocate NulLogger if one was created in constructor (if logger was not specified explicitly)
    bool loggerOwnership = false;

    /// Used to create a new NulLogger object when logger isn't specified in constructor.
    void CreateNulLogger();

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
     * @brief Attach processes by their PIDs
     * 
     * This method takes PID int array as a parameter \n
     * and creates MailboxReference objects with names \n
     * correspondin to PIDs. \n
     * \n
     * WARNING: This only works if WatchdogClient object's \n
     * MailboxReferences also have the same names (corresponding to their respective PIDs)\n
     * 
     * @param pids Pointer to an array of `int`s containing PIDs of processes to be attached to the WatchdogServer 
     * @param size Number of elements in `pids` array
     */
    void AttachByPID(int* pids, int size);

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

    /**
     * @brief Decodes `message` and takes appropriate action.
     * 
     * If received message is "ACK" ("PING" acknowledgement from WatchdogClient) then return TRUE \n
     * else receives message, if that message is a known request message (e.g. "CRASH" - system crash request from WatchdogClient) \n
     * then takes the appropiate action and returns FALSE. \n 
     * \n
     * Used to check if WatchdogClient acknowledged "PING" message, whilst decoding important requests like "CRASH" \n
     * 
     * @param message Received message to be decoded
     * @return true Received message is equal to "ACK"
     * @return false Received message is NOT equal to "ACK"
     */
    bool Decode(const mailbox_message& message);

    /**
     * @brief Pings WatchdogClient via a mailbox and listens for response.
     * 
     * @param destination MailboxReference which references the WatchdogClient's mailbox
     * @return true WatchdogClient responded with acknowledgement
     * @return false WatchdogClient did not respond
     */
    bool TryToContact(MailboxReference& destination);

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
};

#endif