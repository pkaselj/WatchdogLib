#ifndef WATCHDOG_CLIENT_HPP
#define WATCHDOG_CLIENT_HPP

#include"/home/pi/Shared/LoggerLib/0.0.0/ILogger.hpp"

#include"/home/pi/Shared/SharedMemoryLib/0.0.0/SharedMemory.hpp"

#include<string>

/**
 * @brief Enum of possible states of processes/threads declared as `char`s
 * 
 */
enum Status : char {CLEAR = 0, BUSY = 1, IDLE = 2, TERMINATE = 3};


/**
 * @brief Wrapper struct for process status
 * 
 */
typedef struct ProcessStatus ProcessStatus;
struct ProcessStatus
{
    private:
    /// Status of the current process/thread
    Status status;

    public:
    /**
     * @brief Set the status of the ProcessStatus object
     * 
     * @param _status New status
     */
    void set(Status _status);

    /**
     * @brief Compare the ProcessStatus object to the Status enum
     * 
     * @param _status Status enum
     * @return true ProcessStatus is equal to the Status enum
     * @return false ProcessStatus is NOT equal to the Status enum
     */
    bool operator==(Status _status) const;

    /**
     * @brief Update the ProcessStatus to new status
     * 
     * @param _status Status enum
     * @return const ProcessStatus& Returns the reference to this ProcessStatus object. Used for stacking.
     */
    ProcessStatus& operator=(Status _status);

    /**
     * @brief Conversion to `int` type
     * 
     * Mostly used for printing `int` value of ProcessStatus
     * 
     * @return int `int` value of current ProcessStatus
     */
    operator int() const;
};

/**
 * @brief Watchdog client side class. Every process should implement one.
 * 
 */
class WatchdogClient
{
    private:
    /// Shared memory object with ProcessStatus objects as basic shared memory units.
    SharedMemory<ProcessStatus> watchdogRegister;

    /// Offset from shared memory base address pointing to the designated WatchdogClient status slot
    int offset = -1;

    /// Pointer to a logger
    ILogger* p_logger = nullptr;

    /// Creates NulLogger object. Is called when no logger is specified.
    void CreateNulLogger();

    /// Flag used to signal to destructor to deallocate NulLogger (if it was created in constructor)
    bool loggerOwnership = false;

    /// Checks if the main process signaled the TERMINATE signal
    bool MarkedForTermination();

    public:
    /**
     * @brief Construct a new Watchdog Client object
     * 
     * @param shmName String identifier (name) of shared memory
     * @param _offset Offset from shared memory base address pointing to the designated WatchdogClient status slot
     * @param _p_logger Pointer to a parent ILogger which is used as a logger. - NULL SAFE
     */
    WatchdogClient(const std::string& shmName, int _offset, ILogger* _p_logger = nullptr);

    /**
     * @brief Destroy the Watchdog Client object
     * 
     */
    ~WatchdogClient();

    /**
     * @brief Sets the status to IDLE. RETURNS FALSE IF PROCESS IS MARKED FOR TERMINATION!
     * 
     * @return true Success
     * @return false Process is marked for TERMINATION
     */
    bool Pet();

    /**
     * @brief Sets the status to BUSY. RETURNS FALSE IF PROCESS IS MARKED FOR TERMINATION!
     * 
     * @return true Success
     * @return false Process is marked for TERMINATION
     */
    bool Busy();

    /// Signals to the main process that the attached process is or is in process of being TERMINATED
    void Terminate();


};

#endif