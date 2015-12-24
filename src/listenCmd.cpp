#include "listenCmd.hpp"
#include "log.h"

extern CLog g_log;

void* ListerCmd::Listen(void* p)
{
	ListerCmd *pInstance = (ListerCmd *)p;
	pInstance->InitServer();

	pInstance->Run();

		
	return NULL;
}

void ListerCmd::Run()
{
	fd_set readfds;
	struct sockaddr_in cli_addr;		
	m_client_fd = -1;	
	int maxFd = -1;
	struct timeval time_out;
	int ret = 0;
	
	socklen_t length = (socklen_t )sizeof(struct sockaddr_in);
	while(1)
	{		
		time_out.tv_sec = 1;
		time_out.tv_usec = 0;
			
		FD_ZERO(&readfds);
		FD_SET(m_fd,&readfds); 
		if(m_client_fd > 0)
		{
			FD_SET(m_client_fd,&readfds); 
		}
		
		maxFd = m_client_fd > m_fd ? m_client_fd: m_fd;

		ret = select(maxFd + 1, &readfds, NULL, NULL, &time_out);
		if(ret < 0)
		{
			g_log.error("select error !\n");
			close(m_fd);
			break;
		}
		else if (0 == ret)
		{
			//g_log.debug("select timeout !\n");
		}
		else
		{
			if(FD_ISSET(m_fd, &readfds) && (-1 == m_client_fd))
			{
				if ((m_client_fd = accept(m_fd, (struct sockaddr *)&cli_addr, &length)) == -1)
				{
					g_log.error("accept error !\n");
					continue;
				}
			}			
			else if(FD_ISSET(m_client_fd, &readfds))
			{
				RcvCmd();
			}
		}
	}

}
int ListerCmd::InitServer()
{
	char ip[16] = "127.0.0.1";	
	tcpConnector.Init(ip,m_port);
	m_fd = tcpConnector.CreateTcpServer();
	if(m_fd < 0)
	{
		g_log.error("Failed to create a TCP server !\n");
		return ERR;
	}
	g_log.info("[InitServer]socket %d(IP = %s, Port = %d)\n",m_fd,ip,m_port);
	return SUCC;
}

void ListerCmd::RcvCmd()
{
	if(m_client_fd < 0)
		return;

	char szTcpBuf[1024] = {0};
	char *pBufBegin = &szTcpBuf[0];
	char *pBufEnd = pBufBegin;
	
	int iRecvedBytes = 0;
	bool bTerminated = false;
	char * pCmdBegin = pBufBegin;
	
	while(!bTerminated)
	{
		iRecvedBytes = recv(m_client_fd, pBufEnd,1, 0);
		if(iRecvedBytes == 1)
		{
			CMD cmd = {0};
			unsigned int iCmdSize = 0;
			
			if(*pBufEnd == '\n')
			{
				iCmdSize = pBufEnd  - pCmdBegin;				
				bTerminated = true;//ÖÕÖ¹·ûºÅ
			}
			else if(*(pBufEnd-1) == '\\' && *pBufEnd=='n')
			{
				iCmdSize = pBufEnd -1 - pCmdBegin;							
			}
			
			if(iCmdSize > 0)
			{
				if(iCmdSize < sizeof(CMD))
				{
					memcpy(cmd,pCmdBegin,iCmdSize);
					while(SUCC == m_pqueue->IsFull())
					{
						sleep(1);
					}
					m_pqueue->PushOneData(&cmd);
					pCmdBegin = pBufEnd+1;
					printf("got a cmd:%s\n",cmd);
				}
				else
				{
					g_log.error("input cmd is too long(more than %d)\n",sizeof(CMD));
				}
			}
			
			if((pBufEnd++ - pBufBegin) >= (int)sizeof(szTcpBuf))
			{
				g_log.error("ListerCmd::RcvCmd tcpbuf is full\n");
				break;
			}
			//printf("=============TcpConnector::RecvData 1 iRecvedBytes=%d\n",iRecvedBytes);
		}
		else if(errno == EINTR ||errno == EAGAIN)
		{
			continue;
		}
		else 
		{
			close(m_fd);
			g_log.error("ListerCmd::RcvCmd recv fd:%d ret:%d errno:%s!\n",m_client_fd,iRecvedBytes,strerror(errno));
			return;
		}
	}
}

int ListerCmd::Work(int port,LockFreeQueue<CMD>* pQueue)
{
	SetQueue(pQueue);
	pthread_t tid;
	//char cmd[24] = {0};

	m_port = port;
	if (0 != pthread_create(&tid,NULL,Listen,this))
	{	
		g_log.error("Fail to create thread.\n");
		return ERR;
	}

	
	
	return SUCC;
}

