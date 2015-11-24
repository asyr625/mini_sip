#ifndef LOGGER_H
#define LOGGER_H

#include<string>
#include<queue>

#include "logging_manager.h"
#include "thread.h"
#include "ssingleton.h"
#include "tcp_socket.h"
#include "sip_dialog_config.h"

#define DEFAULT_LEVEL 0
#define INFO 0
#define DEBUG 1
#define ERROR 2

using namespace std;

class Logger_Utils
{
public:
    Logger_Utils();
    std::string create_log(std::string value,std::string message);
    std::string get_process_id();
    void set_current_sip_identity(SRef<SipIdentity*> currentSipIdentity);
    void set_call_id(std::string callId);

private:
    std::string process_id;
    std::string call_id;
    SRef<Sip_Identity*> current_sip_identity;
    std::string get_time_stamp();
};

class Log_Sender: public Runnable
{
public:
    Log_Sender(TCP_Socket* senderSocket);
    ~Log_Sender();
    bool start();							//Starts the thread
    bool stop();							//Stops the thread
    bool join();
    virtual void run();						//Starts the Log sender
    void buffer_logs(std::string log);		//Buffers the logs in the sender

private:
    TCP_Socket* sender_socket;
    queue<string> send_buffer;
    SRef<Thread *> thread;
};

class Logger : public SSingleton<Logger>, public SObject
{
public:
    Logger();
    Logger();
    ~Logger();
    void info(std::string id, std::string message);  	//Logs the messages of information level
    void debug(std::string id, std::string message); 	//Logs the messages of debug level
    void error(std::string id, std::string message);

    void setLevel(std::string logLevel); 			 	//Sets the log level
    void setLoggingModuleVersion(void);					//Sets the Logging Version
    void setLoggingManager(LoggingManager* loggingManager);		//Sets the Logging Manager
    void setLogDirectoryPath(std::string logDirectoryPath);		//Sets the Log Directory path
    void setLoggingFlag(bool flag);						//Sets the logging flag
    void setLocalLoggingFlag(bool flag);						//Sets the local logging flag
    void setCurrentSipIdentity(SRef<SipIdentity*>);			//Sets the SipIdentity
    void startLogger();									//Starts the logger
    void stopLogger();

    std::string log_version;            //The version of the logging module
    Logger_Utils loggerUtils;
private:
    int level; 							//Log level
    int logCount;						//Number of Logs in the buffer
    bool loggingFlag;
    bool localLoggingFlag;
    std::string logDirectoryPath;		//Log Directory path
    SRef<Sip_Identity*> currentSipIdentity;		//User ID

    queue<string> temporaryBuffer;		//Temporary buffer which keeps the logs in logger
    TCP_Socket* senderSocket;			//TCP socket for sending logs

    Logging_Manager* loggingManager; 	//Logging Manager
    Log_Sender* logSender;				//Log Sender

    void sendLogs(std::string log);
};

#endif // LOGGER_H
