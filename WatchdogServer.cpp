#include"WatchdogServer.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/NulLogger.hpp"

#include<thread>

void WatchdogServer::defaultInit()
{
    mailbox.RTO = RTO;
    mailbox.BaseTTL = BaseTTL;
    CreateNulLogger();
}

WatchdogServer::WatchdogServer(ILogger* _logger)
    : logger(_logger),
      mailbox("0", logger)
{
    RTO = 3;
    SleepTime = 10;
    BaseTTL = 3;
    defaultInit();
}

WatchdogServer::WatchdogServer()
    : logger(nullptr),
      mailbox("0", logger)
{
    RTO = 3;
    SleepTime = 10;
    BaseTTL = 3;
    defaultInit();
}

WatchdogServer::WatchdogServer(ILogger* _logger, int _RTO, int _PauseTime, int _BaseTTL)
    : logger(_logger),
      mailbox("0", logger),
      RTO(_RTO),
      SleepTime(_PauseTime),
      BaseTTL(_BaseTTL)
{
    defaultInit();
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
    if(TryToContact(destination) == false)
    {
        *logger << destination.getName() + " didn't respond to PINGs! Crashing system!";
        CrashSystem();
    }
        
}

void WatchdogServer::Decode(const mailbox_message& message)
{
    
    if(message.content == "CRASH")
    {
        MailboxReference sender(message.sender);
        mailbox.sendWithoutAcknowledgement(sender, "CACK"); // Crash Request ACK

        CrashSystem(message.sender);
        return;
    }

    return;
}

bool WatchdogServer::TryToContact(MailboxReference& destination)
{
    mailbox_message message;
    int TTL = BaseTTL;
        do
        {
            bool success = mailbox.send(destination, "PING");
            if(success == true)
                return true;

            TTL--;
            *logger << "TTL: " + std::to_string(TTL);

        }while(TTL > 0);

    *logger << "TTL reached 0!";
    return false;
}

void WatchdogServer::Sleep()
{
    int previousMailboxRTO = mailbox.RTO;
    
    mailbox.RTO = SleepTime;
    mailbox_message message = mailbox.timedReceive();
    Decode(message);

    mailbox.RTO = previousMailboxRTO;
}

void WatchdogServer::CreateProcess(IProcess* p_process)
{
    if(p_process == nullptr)
    {
        *logger << "Process cannot be NULL/nullptr!";
        exit(-1);
    }

    p_process->initialize();
    p_process->synchronize();

    // Bog nek mi oprosti grije +
    void (IProcess::*p_start) (void) = &(*p_process)::start; // pointer to p_process->start()
    std::thread mainThread(p_start);

    void (WatchdogClient::*p_startMonitoring) (void) = &WatchdogClient::StartMonitoring();
    std::thread watchdogThread(p_startMonitoring);
    
    mainThread.join();
    watchdogThread.join();

    if(p_process->status != ProcessStatus::TERMINATE) // Terminated silently
        CrashSystem();

}


void WatchdogServer::Sweep()
{
    for(auto& process : processes)
        Ping(process->getMailboxReference());

}

void WatchdogServer::StartMonitoring()
{
    Sweep();
    Sleep();
}
    
void WatchdogServer::StartAndSynchronize()
{
    int pid = -1;
    for(auto& process : processes)
    {
        pid = fork();
        if(pid != 0)
        {
            PIDs.push_back(pid);
            //syncStatus.push_back(false);
            continue;
        }

        CreateProcess(process);
        exit(0);
    }

    for(auto& process : processes)
    {
        bool status = mailbox.send(process->getMailboxReference(), "SYN");
        if(status == false)
            CrashSystem();
        else
            *logger << process->getMailboxReference().getName() + " SYNchronized!";
    }
    
}

void WatchdogServer::Start()
{

}

void WatchdogServer::KillAll()
{
     
}

void WatchdogServer::AttachProcess(IProcess* p_process)
{
    if(p_process == nullptr)
    {
        *logger << "Process cannot be NULL/nullptr!";
        exit(-1);
    }

    processes.push_back(p_process);
}
