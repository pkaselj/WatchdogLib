#include"WatchdogClient.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/NulLogger.hpp"

void WatchdogClient::defaultInit()
{
    RTO = 3;
    BaseTTL = 3;

    CreateNulLogger();
}

WatchdogClient::WatchdogClient(const std::string& mailboxId, ILogger* _logger)
    :   logger(_logger),
        mailbox(mailboxId, logger),
        watchdogServer("0")
{
    defaultInit();
}

WatchdogClient::WatchdogClient(const std::string& mailboxId)
    :   logger(nullptr),
        mailbox(mailboxId, logger),
        watchdogServer("0")
{
    defaultInit();
}


WatchdogClient::WatchdogClient(const std::string& mailboxId, ILogger* _logger, int _RTO, int _BaseTTL)
    :   logger(_logger),
        mailbox(mailboxId, logger),
        watchdogServer("0"),
        RTO(_RTO),
        BaseTTL(_BaseTTL)
        {}

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