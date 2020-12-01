#ifndef WATCHDOG_CLIENT_HPP
#define WATCHDOG_CLIENT_HPP

#include"/home/pi/Shared/IPCommunicationsLib/0.0.0/mailbox.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/ILogger.hpp"


class WatchdogClient
{
    private:
    void defaultInit();

    ILogger* logger;
    bool loggerOwnership = false;

    Mailbox mailbox;
    MailboxReference watchdogServer;
    int RTO;
    int BaseTTL;

    bool RequestSystemCrash();
    void EmergencyCrash();
    void CreateNulLogger();

    public:
    WatchdogClient(const std::string& mailboxId);
    WatchdogClient(const std::string& mailboxId, ILogger* _logger);
    WatchdogClient(const std::string& mailboxId, ILogger* _logger, int _RTO, int _BaseTTL);
    ~WatchdogClient();

    void ListenAndRespond();
    void Crash();

    mailbox_message Listen();
};

#endif