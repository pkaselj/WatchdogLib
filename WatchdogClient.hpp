#ifndef WATCHDOG_CLIENT_HPP
#define WATCHDOG_CLIENT_HPP

#include"/home/pi/Shared/IPCommunicationsLib/0.0.0/mailbox.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/ILogger.hpp"

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

    /**
     * @brief Requests system crash (soft crash) from WatchdogServer
     * 
     * @return true if system crash was authorized
     * @return false if system crash was not authorized or WatchdogServer did not respond
     */
    bool RequestSystemCrash();

    /**
     * @brief Crashes the process (hard crash)
     * 
     * Used when WatchdogServer does not respond to `RequestSystemCrash()`
     */
    void EmergencyCrash();

    /// Logger flag. Used in destructor to deallocate NulLogger if one was created in constructor (if logger was not specified explicitly)
    bool loggerOwnership = false;

    /// Used to create a new NulLogger object when logger isn't specified in constructor.
    void CreateNulLogger();

    //////////////////////////////////////////////////
    public:
    //////////////////////////////////////////////////

    /**
     * @brief Construct a new Watchdog Client object without logger and parameters set to default value
     * 
     * See class implementation for default paramater values
     * 
     * @param mailboxId String identifier (mailbox's name) - GLOBALLY UNIQUE
     */
    WatchdogClient(const std::string& mailboxId);

    /**
     * @brief Construct a new Watchdog Client object with logger and parameters set to default value
     * 
     * See class implementation for default paramater values
     * 
     * @param mailboxId String identifier (mailbox's name) - GLOBALLY UNIQUE
     * @param _logger Pointer to a logger
     */
    WatchdogClient(const std::string& mailboxId, ILogger* _logger);

    /**
     * @brief Construct a new Watchdog Client object with logger and parameters set to specified value
     * 
     * @param mailboxId String identifier (mailbox's name) - GLOBALLY UNIQUE
     * @param _logger Pointer to a logger
     * @param _RTO RTO = Request TimeOut
     * @param _BaseTTL TTL = Time To Live
     */
    WatchdogClient(const std::string& mailboxId, ILogger* _logger, int _RTO, int _BaseTTL);

    /**
     * @brief Destroy the Watchdog Client object
     * 
     */
    ~WatchdogClient();

    /**
     * @brief Block and listen for WatchdogServer "PING" messages and respond with "ACK"
     * 
     */
    void ListenAndRespond();

    /**
     * @brief Block and listen for messages and decode them to mailbox_message format
     * 
     * @return mailbox_message Received message decoded to mailbox_message format
     */
    mailbox_message Listen();

    
    /**
     * @brief Try to request the system crash from WatchdogServer, else exit process
     * 
     */
    void Crash();
};

#endif