#include"WatchdogClient.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/NulLogger.hpp"

#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>

void ProcessStatus::set(Status _status)
{
    status = _status;
}

bool ProcessStatus::operator==(Status _status) const
{
    return (status == _status) ? true : false;
}
ProcessStatus& ProcessStatus::operator=(Status _status)
{
    status = _status;
    return *this;
}

ProcessStatus::operator int() const
{
    return status;
}

void WatchdogClient::CreateNulLogger()
{
    if(p_logger == nullptr)
    {
        p_logger = new NulLogger();
        loggerOwnership = true;
    }
        
}

WatchdogClient::WatchdogClient(const std::string& shmName, int _offset, ILogger* _p_logger)
    :   watchdogRegister(shmName, 0, _p_logger)
{
    p_logger = _p_logger;
    offset = _offset;
    CreateNulLogger();
}


WatchdogClient::~WatchdogClient()
{
    if(loggerOwnership == true)
        delete p_logger;
}

bool WatchdogClient::Pet()
{
    if(MarkedForTermination() == true) // signal to end process
        return false;

    ProcessStatus status;
    status.set(Status::IDLE);

    watchdogRegister.write(offset, status);
    *p_logger << "Status set to IDLE";

    return true;
}

bool WatchdogClient::Busy()
{
    if(MarkedForTermination() == true) // signal to end process
        return false;

    ProcessStatus status;
    status.set(Status::BUSY);

    watchdogRegister.write(offset, status);
    *p_logger << "Status set to BUSY";

    return true;
}

void WatchdogClient::Terminate()
{
    ProcessStatus status;
    status.set(Status::TERMINATE);
    watchdogRegister.write(offset, status);
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