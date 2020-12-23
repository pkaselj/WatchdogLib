#include"WatchdogServer.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/NulLogger.hpp"


int WatchdogServer::Reserve()
{
    *p_logger << "Attempting to reserve a slot for another process...";

    for(int i = 0; i < currentStatus.getLength(); i++)
    {
        if(currentStatus.read(i) == Status::FREE)
        {
            previousStatus.at(i) = Status::FREE;
            currentStatus.write(i, Status::CLAIMED);
            ResetTTL(i);

            *p_logger << "Found free slot at offset: " + std::to_string(i);
            return i;
        }
            
    }

    // No free slots left! TODO dynamically expand the shared memory segment!
    *p_logger << "No free slots left!";
    Terminate();
    return -1;
}

void WatchdogServer::CheckAll()
{
    *p_logger << "Starting pulse check on all attached processes.";
    for(int i = 0; i < currentStatus.getLength(); i++)
    {
        Check(i);
    }

}

void WatchdogServer::Check(unsigned int offset)
{
    ProcessStatus currentProcessStatus = currentStatus.read(offset);
    ProcessStatus& previousProcessStatus = previousStatus.at(offset);

    if(TTL.at(offset) <= 0) Terminate();

    if(currentProcessStatus == Status::FREE) ; // Do nothing

    else if(currentProcessStatus == Status::IDLE)
    {
        *p_logger << "Slot " + std::to_string(offset) + " : " + processes.at(offset) + " is IDLE!";

        currentStatus.write(offset, Status::CLEAR);
        ResetTTL(offset);
    }
        
    else if(currentProcessStatus == Status::BUSY &&
        previousProcessStatus == Status::BUSY)
    {
       *p_logger << "Slot " + std::to_string(offset) + " : " + processes.at(offset) + " is STILL BUSY!";
       DecrementTTL(offset);
    }
        
    else if(currentProcessStatus == Status::CLAIMED)
    {
       *p_logger << "Slot " + std::to_string(offset) + " : " + processes.at(offset) + " is UNCLAIMED!";
       DecrementTTL(offset);
    }

    else if(currentProcessStatus == Status::TERMINATE)
    {
        *p_logger << "Slot " + std::to_string(offset) + " : " + processes.at(offset) + " requested TERMINATION!";
        Terminate();
    }

    else if(currentProcessStatus == Status::CLEAR)
    {
        *p_logger << "Slot " + std::to_string(offset) + " : " + processes.at(offset) + " is UNRESPONSIVE!";
        DecrementTTL(offset);
    }
        
    previousStatus.at(offset) = currentStatus.read(offset);
}

void WatchdogServer::Terminate()
{
    if(terminated == true)
        return; // prevent TERMINATING multiple times

    for(int i = 0; i < currentStatus.getLength(); i++)
    {
        currentStatus.write(i, Status::TERMINATE);
    }

    terminated = true;

    *p_logger << "All slots set to TERMINATE!";
}

void WatchdogServer::ResetTTL(unsigned int offset)
{
    *p_logger << "TTL reset for slot with offset " + std::to_string(offset) + " : " + processes.at(offset);
    TTL.at(offset) = BaseTTL;
}

void WatchdogServer::DecrementTTL(unsigned int offset)
{
    *p_logger << "TTL decremented for slot with offset " + std::to_string(offset) + " : " + processes.at(offset);
    if(--TTL.at(offset) < 0)
    {
        *p_logger << "TTL ran out on slot with offset: " + std::to_string(offset) + " : " + processes.at(offset);
        Terminate();
    }
        
}

void WatchdogServer::Sleep()
{
    CheckMessages();
}

void WatchdogServer::Setup()
{
    *p_logger << "Starting watchdog server setup...";
    ProcessStatus startingStatus = Status::FREE;

    int length = currentStatus.getLength();

    previousStatus.resize(length);
    processes.resize(length);
    TTL.resize(length);

    for(int i = 0; i < length; i++)
    {
        currentStatus.write(i, startingStatus);
        previousStatus.at(i) = (startingStatus);
        processes.at(i) = ("UNKNOWN");
        TTL.at(i) = (BaseTTL);
    }

    *p_logger << "Watchdog server setup finished!";

}

WatchdogServer::WatchdogServer(const std::string& shmIdentifier,
                               int shmLength,
                               int _BaseTTL,
                               ILogger* _p_logger,
                               const std::string& mailboxIdentifier,
                               unsigned int _SleepTime)
    :   watchdogServerMailbox(mailboxIdentifier, p_logger),
        currentStatus(shmIdentifier, shmLength, p_logger),
        BaseTTL(_BaseTTL),
        SleepTime(_SleepTime)
{
    p_logger = _p_logger;
    if(p_logger == nullptr)
        p_logger = NulLogger::getInstance();

    Setup();
}

WatchdogServer::~WatchdogServer()
{
    Terminate();
}

void WatchdogServer::Patrol()
{
    CheckAll();
    CheckMessages();
}

bool WatchdogServer::hasTerminated() const
{
    return terminated;
}

void WatchdogServer::CheckMessages()
{
    // prevents calling of ListenForMessage() more than `DoSGuard` times
    int DoSGuard = 5;
    watchdogServerMailbox.RTO = SleepTime;
    
    
    bool timedOut = false;
    while(DoSGuard > 0)
    {
        *p_logger << "Sleep while listening for messages. Max message count left: " + std::to_string(DoSGuard);
        timedOut = ListenForMessage();
        if(timedOut == true)
        {
            *p_logger << "No messages received! Continuing...";
            break;
        }
        
        --DoSGuard;
    }

}

bool WatchdogServer::ListenForMessage()
{
    mailbox_message request = watchdogServerMailbox.timedReceive();

    if(request.isEmpty())
        return true;

    MailboxReference requestSource = request.sender;

    if(request.content == "RESERVE")
    {
        *p_logger << "RESERVE request received.";

        int freeSlot = Reserve();
        if(watchdogServerMailbox.send(requestSource, std::to_string(freeSlot)) == false)
            Terminate();
        processes.at(freeSlot) = requestSource.getName();
    }

    return false;

}