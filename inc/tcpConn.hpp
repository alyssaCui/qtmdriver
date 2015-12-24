#include <sys/socket.h>

#define BACKLOG     10


class TcpConnector
{
public:
	TcpConnector();
	~TcpConnector();
	void Init(int fd);	
	int Init(char * ip,int port);
	int TcpTimedConnect(int sock, const  struct sockaddr *addr,socklen_t addr_len, int timeout);		
	int TcpClientConnect();
	int CreateTcpServer();
	void RecvData();
	int GetData(char * &rpcDataBeg, int len);
	void FlushData(int len);
	int GetFD(){return m_connfd;}

private:
	const static int m_iUsrRecvBufSize = 30*1024;
	int m_connfd;
	char m_acBuf[m_iUsrRecvBufSize];
	int m_Begin;
	int m_End;
	char m_ip[16];
	int m_port;
};


