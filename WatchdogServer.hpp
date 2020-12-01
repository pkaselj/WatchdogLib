#ifndef WATCHDOG_SERVER_HPP
#define WATCHDOG_SERVER_HPP

#include<vector>

#include"/home/pi/Shared/IPCommunicationsLib/0.0.0/mailbox.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/ILogger.hpp"


class WatchdogServer
{
    private:
    void defaultInit();

    bool loggerOwnership = false;

    ILogger* logger;
    Mailbox mailbox;
    int RTO; // Waiting time for answer
    int SleepTime; // Time between two rounds of pings
    int BaseTTL; // Base number of ping tries before crashing
    
    std::vector<MailboxReference> attachedClients;

    public:
    WatchdogServer();
    WatchdogServer(ILogger* _logger);
    WatchdogServer(ILogger* _logger, int _RTO, int _PauseTime, int _BaseTTL);
    ~WatchdogServer();
    void CreateNulLogger();
    void AttachByPID(int* pids, int size);
    void CrashSystem();
    void CrashSystem(const std::string& initiator);
    void Ping(MailboxReference& destination);
    bool Decode(const mailbox_message& message);
    bool TryToContact(MailboxReference& destination);
    void Sleep();
    mailbox_message Listen();
    void Start();
    void KillAll();
};

#endif