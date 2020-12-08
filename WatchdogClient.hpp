#ifndef WATCHDOG_CLIENT_HPP
#define WATCHDOG_CLIENT_HPP

#include"/home/pi/Shared/IPCommunicationsLib/0.0.0/mailbox.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/ILogger.hpp"


enum ProcessStatus : int {RUNNING, TERMINATE};

/**
 * @brief Watchdog client side class. Every process should implement one.
 * 
 */
class WatchdogClient
{
    //////////////////////////////////////////////////
    private:
    //////////////////////////////////////////////////

    void defaultInit();

    /// Pointer to a logger
    ILogger* logger;

    /// Mailbox used to send and receive messages from WatchdogServer
    Mailbox mailbox;

    /// MailboxReference to WatchdogServer mailbox
    MailboxReference watchdogServer;

    /**
     * @brief RTO = Request TimeOut
     * 
     * Used as a timer to `timedReceive()` from Mailbox class. \n
     * When time runs out (without answer from destination mailbox that sent message was received) \n
     * WatchdogClient then requests system crash (major error). \n
     * 
     */
    int RTO;

    /**
     * @brief TTL = Time To Live
     * 
     * BaseTTL specifies how many times the WatchdogClient \n
     * will resend messages to destination via a mailbox \n
     * without acknowledgement, before requesting system crash (major error). \n
     * 
     */
    int BaseTTL;

    /// Logger flag. Used in destructor to deallocate NulLogger if one was created in constructor (if logger was not specified explicitly)
    bool loggerOwnership = false;

    /// Used to create a new NulLogger object when logger isn't specified in constructor.
    void CreateNulLogger();

    /**
     * @brief Requests system crash (soft crash) from WatchdogServer
     * 
     * @return true if system crash was authorized
     * @return false if system crash was not authorized or WatchdogServer did not respond
     */
    void SendCrashRequest();

    bool ListenFor(const std::string& source, const std::string& response);

    //////////////////////////////////////////////////
    public:
    //////////////////////////////////////////////////

    int* p_status;

    WatchdogClient(const std::string& mailboxId, int* _p_status);

    WatchdogClient(const std::string& mailboxId, int* _p_status, ILogger* _logger);

    WatchdogClient(const std::string& mailboxId, int* _p_status, ILogger* _logger, int _RTO, int _BaseTTL);

    /**
     * @brief Destroy the Watchdog Client object
     * 
     */
    ~WatchdogClient();

    
    /**
     * @brief Try to request the system crash from WatchdogServer, else exit process
     * 
     */
    void RequestSystemCrash();

    void Synchronize();

    bool TryToSignal(const std::string& signal, const std::string& response);

    void SetStatusTo(ProcessStatus status);

    void StartMonitoring();
};

#endif