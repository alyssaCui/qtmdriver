#include <string.h>//bzero()
#include <netinet/in.h>//socket(),bind(),recvfrom(), struct sockaddr
#include <fcntl.h>
#include <sys/select.h> 

#include "tcpConn.hpp"
#include "log.h"

extern CLog g_log;

TcpConnector::TcpConnector():m_connfd(-1),m_Begin(0),m_End(0)
{
	memset(m_acBuf,0,sizeof(m_acBuf));
}

TcpConnector::~TcpConnector()
{

}

void TcpConnector::Init(int fd)
{
	m_connfd = fd;
	m_Begin = 0;
	m_End = 0;
}

int TcpConnector::Init(char * ip,int port)
{
	if(m_ip == NULL || m_port < 0)
	{
		g_log.error("[%s]Input param error,ip is NULL or port is less than zero!\n",__FUNCTION__);
		return -1;
	}
	
	memcpy(m_ip,ip,sizeof(m_ip));
	m_port = port;

	return 0;
}

int TcpConnector::TcpClientConnect()
{
	struct sockaddr_in svr_addr;	

	m_connfd = socket(AF_INET,SOCK_STREAM,0);//tcp
	if(m_connfd < 0)
	{
		g_log.error("%s create socket error!\n",__FUNCTION__);
		return -1;	
	}
	
	bzero(&svr_addr,sizeof(struct sockaddr_in));
	svr_addr.sin_family = AF_INET;//internet-domain 
	svr_addr.sin_addr.s_addr = inet_addr(m_ip);
	svr_addr.sin_port = htons(m_port);

    if(TcpTimedConnect(m_connfd, (struct sockaddr*)&svr_addr, sizeof(struct sockaddr),3000) == 0) 
    {
        //printf("[%s]Connect to server (%s:%d) OK\n", __FUNCTION__,ip, port);
        return m_connfd;
    }
	
	return -1;
}

int TcpConnector::TcpTimedConnect(int sock, const  struct sockaddr *addr,socklen_t addr_len, int timeout) /* timeout in millsec */
{
#ifdef LINUX 

    /* this may be GNU/Linux specific */
    int err;
    socklen_t errlen = sizeof(err);
    fd_set fds;
    struct timeval time_out ;
	
    int old_flags = fcntl(sock, F_GETFL);
    if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
    {
        g_log.error("[%s]%d(%s)\n",__FUNCTION__, errno,strerror(errno));
        return -1;
    }
    
    if (connect(sock, addr, addr_len) == -1 && errno != EINPROGRESS)
    {
        g_log.error("[%s]Error in connect(), %d(%s)\n",__FUNCTION__, errno,strerror(errno));
        fcntl(sock, F_SETFL, old_flags);
        return -1;
    } 
    
    time_out.tv_sec = (timeout / 1000);
    time_out.tv_usec = (timeout % 1000) * 1000;
    
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    if (-1 == select(sock + 1, 0, &fds, 0, &time_out))
    {
        g_log.error("[%s]connection timed out, %d(%s)\n",__FUNCTION__, errno,strerror(errno));
        fcntl(sock, F_SETFL, old_flags);
        return -1;
    } 
         
    err = ETIMEDOUT;  
    if (FD_ISSET(sock, &fds))
    {
        if (getsockopt(sock, SOL_SOCKET, SO_ERROR,  &err, &errlen) == -1)
        {
            g_log.error("%d(%s)\n", errno,strerror(errno));
            return -1;
        }
    } 
	
    if (err == 0) /* succeeded */ 
        ;
    else
    {
        errno = err;
        g_log.error("[%s]Connect failed, %d(%s)\n",__FUNCTION__,errno, strerror(errno));
        fcntl(sock, F_SETFL, old_flags);
        return -1;
    }       
    
    fcntl(sock, F_SETFL, old_flags);
    return 0;
#else
#warning not implemented on this platform!
    return connect(sock, addr, addr_len);
#endif    
}

void TcpConnector::RecvData()
{
	if(m_connfd < 0)
	{
		g_log.error("Error!TcpConnector::RecvData m_connfd<0\n");
		return;
	}
	
	if( m_Begin == m_End )
	{
		m_Begin = 0;
		m_End = 0;
	}
	
	//printf("TcpConnector::RecvData m_Begin=%d\n",m_Begin);
	
	int iRecvedBytes = 0;
	do
	{
		if( m_End == m_iUsrRecvBufSize )
		{
			if (m_Begin > 0) 
			{
				memmove(&m_acBuf[0], &m_acBuf[m_Begin], m_End - m_Begin);
				m_End -= m_Begin;
				m_Begin = 0;				
			}
			else//m_Begin equals 0 ,and m_End equals m_iUsrRecvBufSize, meaning buf is full!!
			{
				g_log.info("TcpConnector::RecvData full!!! m_Begin=%d, m_End=%d\n",m_Begin,m_End);
				return ;
			}			
		}

		iRecvedBytes = recv(m_connfd, &m_acBuf[m_End],m_iUsrRecvBufSize-m_End, 0);
		if(iRecvedBytes > 0)
		{
			m_End += iRecvedBytes;
			//printf("=============TcpConnector::RecvData 1 iRecvedBytes=%d\n",iRecvedBytes);
		}
		/*else if( iRecvedBytes == 0 )
		{
			printf("=============TcpConnector::RecvData 2 iRecvedBytes=%d\n",iRecvedBytes);
			close(m_connfd);
			return ;
		}*/
		else if(errno == EINTR ||errno == EAGAIN)
		{
			continue;
		}
		else 
		{
			close(m_connfd);
			g_log.error("error!TcpConnector::RecvData recv errno:%s!\n",strerror(errno));
			return ;
		}

	}while(iRecvedBytes>0);

	//printf("=============TcpConnector::RecvData 3 iRecvedBytes=%d\n",iRecvedBytes);

}

int TcpConnector::GetData(char * &rpcDataBeg, int len)
{
	
	rpcDataBeg = &m_acBuf[m_Begin];
	if(len <= m_End - m_Begin)
	{
		//printf("m_Begin=%5d,m_End=%5d, len=%d\n",m_Begin,m_End,len);
		return len;
	}

	return 0;
}

void TcpConnector::FlushData(int iFlushLength)
{
	if(m_acBuf == NULL )
	{
		return  ;
	}

	if(iFlushLength > m_End - m_Begin || iFlushLength == -1)
	{
		iFlushLength = m_End - m_Begin;
	}

	m_Begin += iFlushLength;

	if( m_Begin == m_End )
	{
		m_Begin = m_End = 0;
	}

	//printf("TcpConnector::FlushData   m_Begin=%d \n",m_Begin );
}

int TcpConnector::CreateTcpServer()
{
	struct sockaddr_in ser_addr;

	int iReusePortFlag = 1;

	m_connfd = socket(AF_INET,SOCK_STREAM,0);    //tcp socket
	if(m_connfd < 0)
	{	
		printf("%s create socket error, %d(%s)\n",__FUNCTION__,errno,strerror(errno));
		return -1;
	}	

	bzero(&ser_addr,sizeof(struct sockaddr_in));
	ser_addr.sin_family = AF_INET;             //internet domain
	//ser_addr.sin_addr.s_addr=htonl(INADDR_ANY);  //server side
	ser_addr.sin_addr.s_addr = inet_addr(m_ip);
	ser_addr.sin_port = htons(m_port) ;

	setsockopt(m_connfd, SOL_SOCKET, SO_REUSEADDR, &iReusePortFlag, sizeof(iReusePortFlag));
	
	if(bind(m_connfd,(struct sockaddr*)&ser_addr,sizeof(struct sockaddr_in))<0)
	{	
		printf("[%s] failed to bind socket %d for port %d, %d(%s)\n",__FUNCTION__,m_connfd,m_port,errno,strerror(errno));
		return -1;
	}
	
	if (listen(m_connfd, BACKLOG) == -1) 	
	{
		printf("[%s]failed to listen socket %d, %d(%s)\n",__FUNCTION__,m_connfd,errno,strerror(errno)); 
		close(m_connfd); 	
		return -1;	
	}	
	
	return m_connfd;
}


