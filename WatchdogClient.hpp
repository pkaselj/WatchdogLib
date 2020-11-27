#ifndef WATCHDOG_CLIENT_HPP
#define WATCHDOG_CLIENT_HPP

#include"/home/pi/Shared/IPCommunicationsLib/0.0.0/mailbox.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/ILogger.hpp"


class WatchdogClient
{
    private:
    int RTO;
    int BaseTTL;

    ILogger* logger;
    bool loggerOwnership = false;

    Mailbox mailbox;
    MailboxReference watchdogServer;

    public:
    WatchdogClient(const std::string& mailboxId);
    WatchdogClient(const std::string& mailboxId, ILogger* _logger);
    WatchdogClient(const std::string& mailboxId, ILogger* _logger, int _RTO, int _BaseTTL);
    ~WatchdogClient();
    void CreateNulLogger();
    void ListenAndRespond();
    void Crash();
    bool RequestSystemCrash();
    void EmergencyCrash();
    mailbox_message Listen();
};


WatchdogClient::WatchdogClient(const std::string& mailboxId, ILogger* _logger)
    :   mailbox(mailboxId, logger), watchdogServer("0")
{
    RTO = 3;
    BaseTTL = 3;

    CreateNulLogger();
}

WatchdogClient::WatchdogClient(const std::string& mailboxId)
    :   WatchdogClient(mailboxId, nullptr) {}


WatchdogClient::WatchdogClient(const std::string& mailboxId, ILogger* _logger, int _RTO, int _BaseTTL)
    :   mailbox(mailboxId, logger), watchdogServer("0"), RTO(_RTO), BaseTTL(_BaseTTL) {}

void WatchdogClient::CreateNulLogger()
{
    if(logger == nullptr)
    {
        logger = new NulLogger();
        loggerOwnership = true;
    }
}

WatchdogClient::~WatchdogClient()
{
    if(loggerOwnership == true)
        delete logger;
}

mailbox_message WatchdogClient::Listen()
{
    std::string rawMessage = mailbox.receive();
    mailbox_message message = mailbox.decodeRawMessage(rawMessage);

    return message;
}

// ================================ TODO Integrate
void WatchdogClient::ListenAndRespond()
{
    mailbox_message message = Listen();

    *logger << "Received message: \"" + message.content + "\" from \"" + message.sender + "\"";

    if(message.sender == "0")
    {
        *logger << "Responding to Watchdog Server with ACK";
        mailbox.send(watchdogServer, "ACK");
    }

}

void WatchdogClient::Crash()
{
    *logger << "Requesting System crash!";
    mailbox.send(watchdogServer, "CRASH");

    if(RequestSystemCrash() == false)
        EmergencyCrash();

}

bool WatchdogClient::RequestSystemCrash()
{
    int TTL = BaseTTL;

    do
    {
        mailbox_message message = Listen();
        if(message.sender == "0" && message.content == "CACK")
        {
            *logger << "System crash authorized by watchdog server";
            return true;
        }

        *logger << "No response from watchdog server";

        TTL--;
        *logger << "TTL: " + TTL;

    } while (TTL > 0);
    
    *logger << "TTL reached 0!";
    return false;
}

// ===============================================
void WatchdogClient::EmergencyCrash()
{
    *logger << "Emergecy crash initiated!";
    exit(-1);
}
#endif