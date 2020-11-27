#ifndef WATCHDOG_SERVER_HPP
#define WATCHDOG_SERVER_HPP

#include<vector>
//#include<string>

#include"/home/pi/Shared/IPCommunicationsLib/0.0.0/mailbox.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/ILogger.hpp"


class WatchdogServer
{
    private:
    int RTO; // Waiting time for answer
    int SleepTime; // Time between two rounds of pings
    int BaseTTL; // Base number of ping tries before crashing

    bool loggerOwnership = false;

    ILogger* logger;
    Mailbox mailbox;
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
};

WatchdogServer::WatchdogServer(ILogger* _logger)
    : mailbox("0", logger), logger(_logger)
{
    RTO = 3;
    SleepTime = 10;
    BaseTTL = 3;

    CreateNulLogger();

    mailbox.RTO = RTO;
}

WatchdogServer::WatchdogServer()
    : WatchdogServer(nullptr) {}

WatchdogServer::WatchdogServer(ILogger* _logger, int _RTO, int _PauseTime, int _BaseTTL)
    : mailbox("0", logger), logger(_logger), RTO(_RTO), SleepTime(_PauseTime), BaseTTL(_BaseTTL)
{
    CreateNulLogger();
    mailbox.RTO = RTO;
}


void WatchdogServer::CreateNulLogger()
{
    if(logger == nullptr)
    {
        logger = new NulLogger();
        loggerOwnership = true;
    }
        
}

WatchdogServer::~WatchdogServer()
{
    if(loggerOwnership == true)
        delete logger;
}


void WatchdogServer::AttachByPID(int* pids, int size)
{
    if(pids == nullptr)
    {
        *logger << "PID array is null. Exiting ...";
        CrashSystem();
        return;
    }

    for(int i = 0; i < size; i++)
    {
        MailboxReference temporaryMReference( std::to_string(pids[i]) );
        attachedClients.push_back(temporaryMReference);
        *logger << " Attached PID: " + pids[i]; 
    }
}

// =============================================================== TODO Hard Exit
void WatchdogServer::CrashSystem(const std::string& initiator)
{
    *logger << initiator + " Requested System Crash!";
    exit(-1);
}

void WatchdogServer::CrashSystem()
{
    CrashSystem("WatchdogServer");
}
// =============================================================================

void WatchdogServer::Ping(MailboxReference& destination)
{
    *logger << "Sending PING to " + destination.getName();
    mailbox.send(destination, "PING");
}

mailbox_message WatchdogServer::Listen()
{
    mailbox_message message;

    *logger << "Awaiting message ....";
    std::string rawMessage = mailbox.timedReceive();

    message = mailbox.decodeRawMessage(rawMessage);
    return message;
}

bool WatchdogServer::Decode(const mailbox_message& message)
{
    if(message.content == "ACK")
        return true;
    
    if(message.content == "CRASH")
    {
        MailboxReference sender(message.sender);
        mailbox.send(sender, "CACK"); // Crash Request ACK

        CrashSystem(message.sender);
        return false;
    }

    return false;
}

bool WatchdogServer::TryToContact(MailboxReference& destination)
{
    mailbox_message message;
    int TTL = BaseTTL;
        do
        {
            Ping(destination);
            message = Listen();
            if(Decode(message) == true) return true;

            TTL--;
            *logger << "TTL: " + TTL;

        }while(TTL > 0);

    *logger << "TTL reached 0!";
    return false;
}

void WatchdogServer::Sleep()
{
    int previousMailboxRTO = mailbox.RTO;
    
    mailbox.RTO = SleepTime;
    Decode(Listen());

    mailbox.RTO = previousMailboxRTO;
}

void WatchdogServer::Start()
{

    for(auto & process : attachedClients)
    {
        if(TryToContact(process) == false)
            CrashSystem(process.getName());
    }

    Sleep();
}



#endif