#include "tcpConn.hpp"
#include "common.h"

#define LENGTH_CMD 32
typedef char CMD[LENGTH_CMD];


class ListerCmd
{
public:
	int Work(int port,LockFreeQueue<CMD>* pQueue);
	static void *Listen(void *);	
	void RcvCmd();	
	int Report();
	void SetQueue(LockFreeQueue<CMD>* pQueue){m_pqueue = pQueue;}
	int InitServer();
	void Run();

private:	
	TcpConnector tcpConnector;
	int m_fd;
	int m_port;
	LockFreeQueue<CMD>  * m_pqueue;
	int m_client_fd;
	
};

