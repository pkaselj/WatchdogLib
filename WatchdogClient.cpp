#include"WatchdogClient.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/NulLogger.hpp"

void WatchdogClient::defaultInit()
{
    mailbox.RTO = RTO;
    mailbox.BaseTTL = BaseTTL;

    CreateNulLogger();
}

WatchdogClient::WatchdogClient(const std::string& mailboxId, int* _p_status, ILogger* _logger)
    :   p_status(_p_status),
        logger(_logger),
        mailbox(mailboxId, logger),
        watchdogServer("0")
{
    RTO = 3;
    BaseTTL = 3;

    defaultInit();
}

WatchdogClient::WatchdogClient(const std::string& mailboxId, int* _p_status)
    :   p_status(_p_status),
        logger(nullptr),
        mailbox(mailboxId, logger),
        watchdogServer("0")
{
    RTO = 3;
    BaseTTL = 3;

    defaultInit();
}


WatchdogClient::WatchdogClient(const std::string& mailboxId, int* _p_status, ILogger* _logger, int _RTO, int _BaseTTL)
    :   p_status(_p_status),
        logger(_logger),
        mailbox(mailboxId, logger),
        watchdogServer("0"),
        RTO(_RTO),
        BaseTTL(_BaseTTL)
{
    defaultInit();
}

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


void WatchdogClient::RequestSystemCrash()
{
    if(TryToSignal("CRASH", "CACK"))
        *logger << "Requesting System crash!";
    exit(-1);
}


bool WatchdogClient::ListenFor(const std::string& source, const std::string& response)
{
    mailbox_message message = mailbox.receive();
    if(message.sender == source && message.content == response)
    {
        *logger << "Response \"" + response + "\" received from: " + source;
        return true;
    }

    *logger << "No response from: " + source;
    return false;
}

void WatchdogClient::Synchronize()
{
    /*if( TryToSignal("SYN", "SYN") == true)
        *logger << "Process ready and synchronized!";
    else
        RequestSystemCrash();*/
    bool status = false;
    do
    {
        status = ListenFor(watchdogServer.getName(), "SYN");
    }
    while( status == false );

    *logger << "SYNchronization signal received!";

}

bool WatchdogClient::TryToSignal(const std::string& signal, const std::string& response)
{
    int TTL = BaseTTL;
    do
    {
        mailbox.send(watchdogServer, signal);

        if( ListenFor( watchdogServer.getName(), response) == true)
        {
            *logger << "Action authorized by watchdog server";
            return true;
        }
            
        TTL--;
        *logger << "TTL: " + TTL;

    } while (TTL > 0);
    
    *logger << "TTL reached 0!";
    return false;
}

void WatchdogClient::SetStatusTo(ProcessStatus status)
{
    if(p_status == NULL)
        return;
    
    *p_status = status;
}

void WatchdogClient::StartMonitoring()
{
    do
    {
        mailbox_message message = mailbox.receive();

        if(message.content == "CRASH" && message.sender == watchdogServer.getName())
            SetStatusTo(ProcessStatus::TERMINATE);
        
        if(message.content == "PING")
            *logger << message.sender + " PINGed!";

    } while(*p_status != ProcessStatus::TERMINATE); // p_status == nullptr??


}

