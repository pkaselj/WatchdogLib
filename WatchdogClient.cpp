#include"WatchdogClient.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/NulLogger.hpp"

#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>

bool WatchdogClient::SetStatus(ProcessStatus status)
{
    if(MarkedForTermination() == true) // signal to end process
        return false;

    watchdogRegister.write(offset, status);
    return true;
}

void WatchdogClient::Reserve()
{
    Mailbox temporaryMailbox(name, p_logger);


    mailbox_message receivedMessage;
    while(receivedMessage.isEmpty())
    {
        temporaryMailbox.send(watchdogServer, "RESERVE");
        receivedMessage = temporaryMailbox.timedReceive();
    }


    if(receivedMessage.isEmpty())
    {
        *p_logger << "No response from Watchdog Server";
        exit(-1);
    }

    if(receivedMessage.sender != watchdogServer.getName())
    {
        *p_logger << "No response from Watchdog Server";
        exit(-1);
    }

    offset = std::stoi(receivedMessage.content);

    if(watchdogRegister.read(offset) != Status::CLAIMED)
    {
        *p_logger << "Desginated watchdog status slot is not CLAIMED!";
        exit(-1);
    }

    SetStatus(Status::IDLE);
}

void WatchdogClient::Release()
{
    SetStatus(Status::FREE);
    *p_logger << "Free'd the Watchdog status slot at offset: " + std::to_string(offset);
}

WatchdogClient::WatchdogClient(const std::string& _name,
                               const std::string& shmName,
                               const std::string& watchdogServerMailboxIdentifier,
                               ILogger* _p_logger)

    :   watchdogRegister(shmName, 0, _p_logger),
        watchdogServer(watchdogServerMailboxIdentifier),
        name(_name)
{
    p_logger = _p_logger;
    if(p_logger == nullptr)
        p_logger = NulLogger::getInstance();

    if(name == "")
    {
        *p_logger << "Name cannot be empty!";
        exit(-1);
    }
    
    Reserve();
}


WatchdogClient::~WatchdogClient()
{
    Release();
}

bool WatchdogClient::Pet()
{
    if(SetStatus(Status::IDLE) == false)
        return false;
    
    *p_logger << "Status set to IDLE";
    return true;
}

bool WatchdogClient::Busy()
{
    if(SetStatus(Status::BUSY) == false)
        return false;
    *p_logger << "Status set to BUSY";

    return true;
}

void WatchdogClient::Terminate()
{
    SetStatus(Status::TERMINATE);
    *p_logger << "Status set to TERMINATE";
}

bool WatchdogClient::MarkedForTermination()
{
    ProcessStatus status = watchdogRegister.read(offset);
    if(status == Status::TERMINATE)
    {
        *p_logger << "Marked for termination!";
        return true;
    }
    
    *p_logger << "NOT marked for termination!";
    return false;
}