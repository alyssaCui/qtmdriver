#include <unistd.h>
#include "qtmdriver.hpp"
#include "log.h"
#include <vector>

CLog g_log;


int QtmDriver::init()
{
	//int flag = 0; 
	m_quote_num = 0;
	m_tunnel_num = 0;
	m_order_rtn_interval = TIME_ORDER_DELAY_DEFAULT;
	
	m_sec_interrupt = 5;
	m_sec_data_loss = 2;
	m_sec_data_delay = 3;
	m_tca_time_out = 5;

	m_pid = getpid();  

	m_lister.Work(m_listenPort,&m_queue);	
	
	for(int i=0;i<MAX_QUOTE;i++)
	{
		m_quote_interval[i] = 500; 
		m_quote_tid[i] = 0;
		m_quote_idx[i] = i;
		m_quote_connected[i] = false;
		
		m_quote_interrupt[i] = false;
		m_quote_loss[i] = false;
		m_quote_delay[i] = false;

		m_tunnel_idx[i] = i + 1000;
		m_tunnel_connected[i] = false;
		m_tunnel_seq_no[i] = 1;
	}

	qtm_init(TYPE_QUOTE|TYPE_TCA);	
	g_log.info("qtm_init(TYPE_QUOTE|TYPE_TCA)\n");
	
	return SUCC;
}

void QtmDriver::run(int port)
{
	CMD cmd;

	m_listenPort = port;

	init();
	

	while(1)
	{
		if(SUCC == m_queue.IsEmpty())
		{
			sleep(1);
			continue;
		}

		memset(&cmd, 0 ,sizeof(cmd));
		m_queue.PopOneData(&cmd);		
		ParseCmd(cmd);
		
	}

	return;
}


void  QtmDriver::ParseCmd(char *pCmd)
{
	std::vector<string> vecSplitted;
	
	string trimedStr(ltrim(pCmd));
	strsplit(trimedStr, vecSplitted);

	if(0 == m_quote_num && 0 == m_tunnel_num && !strstr(pCmd,"qnum"))
	{
		g_log.info("Please set quote_num and tunnel_num\n");
		return;
	}
		
	if(NULL != strstr(pCmd,"qnum"))
	{
		HandleQnum(vecSplitted);
	}
	else if(NULL != strstr(pCmd,"qinterval"))
	{
		HandleQinterval(vecSplitted);
	}
	else if(NULL != strstr(pCmd,"qconn"))
	{
		HandleQconn(vecSplitted);
	}
	else if(NULL != strstr(pCmd,"qlogin"))
	{
		HandleQlogin(vecSplitted);
	}
	else if(NULL != strstr(pCmd,"qsubs"))
	{
		HandleQsubs(vecSplitted);
	}
	else if(NULL != strstr(pCmd,"qlogout"))
	{
		HandleQlogout(vecSplitted);
	}
	else if(NULL != strstr(pCmd,"tnum"))
	{
		HandleTnum(vecSplitted);			
	}
	else if(NULL != strstr(pCmd,"tconn"))
	{
		HandleTconn(vecSplitted);
	}
	else if(NULL != strstr(pCmd,"tlogin"))
	{
		HandleTlogin(vecSplitted);
	}
	else if(NULL != strstr(pCmd,"tlogout"))
	{
		HandleTlogout(vecSplitted);
	}
	else if(NULL != strstr(pCmd,"order"))
	{
		HandleOrder(vecSplitted);
	}
	else if(NULL != strstr(pCmd,"criteria"))
	{
		HandleCriteria(vecSplitted);
	}
	else if(NULL != strstr(pCmd,"qinterrupt"))
	{
		HandleQinterrupt(vecSplitted);
	}
	else if(NULL != strstr(pCmd,"qloss"))
	{
		HandleQloss(vecSplitted);
	}
	else if(NULL != strstr(pCmd,"qdelay"))
	{
		HandleQdelay(vecSplitted);
	}
	else
	{
		g_log.error("[%s]No defined order:%s",__FUNCTION__,pCmd);
	}
}

void QtmDriver::HandleQnum(std::vector<string> & vecSplitted)
{
		/*int idx=0;
		//printf("vecSplitted.size=%d\n",vecSplitted.size());
		for(vector<string>::iterator it  = vecSplitted.begin(); it != vecSplitted.end(); )  
        {  
        	string tmp = (string)*it;
            printf("%d:%s\n",idx++,tmp.c_str());
			it++;
		}*/
	if(m_quote_num > 0)
	{
		return;
	}
	
	if(2 == vecSplitted.size())
	{
		m_quote_num = atoi(vecSplitted[1].c_str());
			
		for(int i=0;i<m_quote_num;i++)
		{
			m_qPara[i].pDriverInstance = (void*)this;
			m_qPara[i].quote_idx = m_quote_idx[i];
			if (0 != pthread_create(&m_quote_tid[i],NULL,handleQuote,(void*)&m_qPara[i]))
			{	
				perror("Fail to create thread");
				return;
			}
		}
	}

	
}

void QtmDriver::HandleTnum(std::vector<string> & vecSplitted)
{
	if(m_tunnel_num > 0)
	{
		return;
	}

	if(2 == vecSplitted.size())
	{
		m_tunnel_num = atoi(vecSplitted[1].c_str());
			
        //printf("tunnel_num = %d\n",tunnel_num);

		for(int i=0;i<m_tunnel_num;i++)
		{
			char name[LEN_NAME] = {0};	
			sprintf(name,"t%d_%d",i,(int)m_pid);
			m_tunnel_idx[i] += m_pid;
			set_identification(m_tunnel_idx[i],name);
			g_log.info("set_identification(%d,%s)\n",m_tunnel_idx[i],name);
		}
	}
}

void QtmDriver::HandleQinterval(std::vector<string> & vecSplitted)
{
	int qIdx = -1;
	
	if(3 == vecSplitted.size())
	{
		qIdx = atoi(vecSplitted[1].c_str());
		m_quote_interval[qIdx] = atoi(vecSplitted[2].c_str());
			
        printf("quote_interval[%d] = %d\n",qIdx,m_quote_interval[qIdx]);
	}
}

void QtmDriver::HandleQconn(std::vector<string> & vecSplitted)
{
	int qIdx = -1;
	int status = -1;
	
	if(3 == vecSplitted.size())
	{
		qIdx = atoi(vecSplitted[1].c_str());
		status= atoi(vecSplitted[2].c_str());

		////status:0--success , 1--fail
		switch(status)
		{
		case 1:
			{
				update_connection_state(m_quote_idx[qIdx], TYPE_QUOTE, 1, "succeeded to connect");
				
				g_log.info("update_connection_state(%d,TYPE_QUOTE,1,\"succeeded to connect\")\n",m_quote_idx[qIdx]);
			}
			break;
		case -1:
			{
				update_connection_state(m_quote_idx[qIdx], TYPE_QUOTE, -1, "Failed to connect");				
				m_quote_connected[qIdx] = false;
				g_log.info("update_connection_state(%d,TYPE_QUOTE,-1,\"Failed to connect\")\n",m_quote_idx[qIdx]);
			}
			break;
		default:
			g_log.error("[%s]No defined stauts:%d",__FUNCTION__,status);
			break;			
		}
	}
}

void QtmDriver::HandleQlogin(std::vector<string> & vecSplitted)
{
	int qIdx = -1;
	int status = -1;
	
	if(3 == vecSplitted.size())
	{
		qIdx = atoi(vecSplitted[1].c_str());
		status= atoi(vecSplitted[2].c_str());
		
		////status:0--success , 1--fail
		switch(status)
		{
		case 1:
			{
				update_connection_state(m_quote_idx[qIdx], TYPE_QUOTE, 1, "succeeded to login");
				
				g_log.info("update_connection_state(%d,TYPE_QUOTE,1,\"succeeded to login\")\n",m_quote_idx[qIdx]);
			}
			break;
		case -1:
			{
				update_connection_state(m_quote_idx[qIdx], TYPE_QUOTE, -1, "Failed to login");
				g_log.info("update_connection_state(%d,TYPE_QUOTE,-1,\"Failed to login\")\n",m_quote_idx[qIdx]);
			}
			break;
		default:
			g_log.error("[%s]No defined stauts:%d",__FUNCTION__,status);
			break;			
		}
	}
}

void QtmDriver::HandleQsubs(std::vector<string> & vecSplitted)
{
	int qIdx = -1;
	int status = -1;
	
	if(3 == vecSplitted.size())
	{
		qIdx = atoi(vecSplitted[1].c_str());
		status= atoi(vecSplitted[2].c_str());
		
		////status:0--success , 1--fail
		switch(status)
		{
		case 1:
			{
				update_connection_state(m_quote_idx[qIdx], TYPE_QUOTE, 1, "succeeded to subscribe");				
				m_quote_connected[qIdx] = true;
				g_log.info("update_connection_state(%d,TYPE_QUOTE,1,\"succeeded to subscribe\")\n",m_quote_idx[qIdx]);
			}
			break;
		case -1:
			{
				update_connection_state(qIdx, TYPE_QUOTE, -1, "Failed to subscribe");
				g_log.info("update_connection_state(%d,TYPE_QUOTE,-1,\"Failed to subscribe\")\n",m_quote_idx[qIdx]);
			}
			break;
		default:
			g_log.error("[%s]No defined stauts:%d",__FUNCTION__,status);
			break;			
		}
	}
}

void QtmDriver::HandleQlogout(std::vector<string> & vecSplitted)
{
	int qIdx = -1;
	int status = -1;
	
	if(3 == vecSplitted.size())
	{
		qIdx = atoi(vecSplitted[1].c_str());
		status= atoi(vecSplitted[2].c_str());
		
		////status:0--success , 1--fail
		switch(status)
		{
		case 1:
			{
				update_connection_state(m_quote_idx[qIdx], TYPE_QUOTE, 1, "succeeded to logout");				
				m_quote_connected[qIdx] = false;
				g_log.info("update_connection_state(%d,TYPE_QUOTE,1,\"succeeded to logout\")\n",m_quote_idx[qIdx]);
			}
			break;
		case -1:
			{
				update_connection_state(m_quote_idx[qIdx], TYPE_QUOTE, -1, "Failed to logout");
				g_log.info("update_connection_state(%d,TYPE_QUOTE,-1,\"Failed to logout\")\n",m_quote_idx[qIdx]);
			}
			break;
		default:
			g_log.error("[%s]No defined stauts:%d",__FUNCTION__,status);
			break;			
		}
	}
}

void QtmDriver::HandleTconn(std::vector<string> & vecSplitted)
{
	int tIdx = -1;
	int status = -1;
	
	if(3 == vecSplitted.size())
	{
		tIdx = atoi(vecSplitted[1].c_str());
		status= atoi(vecSplitted[2].c_str());

		////status:0--success , 1--fail
		switch(status)
		{
		case 1:
			{
				update_connection_state(m_tunnel_idx[tIdx], TYPE_TCA, 1, "succeeded to connect");
				
				g_log.info("update_connection_state(%d,TYPE_TCA,1,\"succeeded to connect\")\n",m_tunnel_idx[tIdx]);
			}
			break;
		case -1:
			{
				update_connection_state(m_tunnel_idx[tIdx], TYPE_TCA, -1, "Failed to connect");
				g_log.info("update_connection_state(%d,TYPE_TCA,-1,\"Failed to connect\")\n",m_tunnel_idx[tIdx]);
			}
			break;
		default:
			g_log.error("[%s]No defined stauts:%d",__FUNCTION__,status);
			break;			
		}
	}
}

void QtmDriver::HandleTlogin(std::vector<string> & vecSplitted)
{
	int tIdx = -1;
	int status = -1;
	
	if(3 == vecSplitted.size())
	{
		tIdx = atoi(vecSplitted[1].c_str());
		status= atoi(vecSplitted[2].c_str());
		
		////status:0--success , 1--fail
		switch(status)
		{
		case 1:
			{
				update_connection_state(m_tunnel_idx[tIdx], TYPE_TCA, 1, "succeeded to login");
				m_tunnel_connected[tIdx] = true;
				g_log.info("update_connection_state(%d,TYPE_TCA,1,\"succeeded to login\")\n",m_tunnel_idx[tIdx]);
			}
			break;
		case -1:
			{
				update_connection_state(m_tunnel_idx[tIdx], TYPE_TCA, -1, "Failed to login");
				g_log.info("update_connection_state(%d,TYPE_TCA,-1,\"Failed to login\")\n",m_tunnel_idx[tIdx]);
			}
			break;
		default:
			g_log.error("[%s]No defined stauts:%d",__FUNCTION__,status);
			break;			
		}
	}
}

void QtmDriver::HandleTlogout(std::vector<string> & vecSplitted)
{
	int tIdx = -1;
	int status = -1;
	
	if(3 == vecSplitted.size())
	{
		tIdx = atoi(vecSplitted[1].c_str());
		status= atoi(vecSplitted[2].c_str());
		
		////status:0--success , 1--fail
		switch(status)
		{
		case 1:
			{
				update_connection_state(m_tunnel_idx[tIdx], TYPE_TCA, 1, "succeeded to logout");
				m_tunnel_connected[tIdx] = true;
				g_log.info("update_connection_state(%d,TYPE_TCA,1,\"succeeded to logout\")\n",m_tunnel_idx[tIdx]);
			}
			break;
		case -1:
			{
				update_connection_state(m_tunnel_idx[tIdx], TYPE_TCA, -1, "Failed to logout");
				g_log.info("update_connection_state(%d,TYPE_TCA,-1,\"Failed to logout\")\n",m_tunnel_idx[tIdx]);
			}
			break;
		default:
			g_log.error("[%s]No defined stauts:%d",__FUNCTION__,status);
			break;			
		}
	}
}


void QtmDriver::HandleOrder(std::vector<string> & vecSplitted)
{
	int tIdx = -1;
	int i = -1;
	int type = -1;	
	time_t now;
	
	if(3 == vecSplitted.size())
	{
		i = atoi(vecSplitted[1].c_str());
		tIdx = m_tunnel_idx[i];
		if(false == m_tunnel_connected[i])
		{
			g_log.error("Tunnnel %d is not login ! please execute command\"qlogin %d 1\"\n",tIdx,i);
		}
		
		type= atoi(vecSplitted[2].c_str());

		
		////status:0--success , 1--fail
		switch(type)
		{
		case 1:  //一次全部成交
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 2:   //报单分两次成交
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		//case3:	报单部分成交(接口acquire_tca_order不对此做区分)
		
		case 4:  //报单后撤单，撤单成功
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_request, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_rtn, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_rtn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 5:  //报单后撤单，单成交撤单失败
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_request, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_rtn, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_rtn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 6:  //报单回报后撤单，撤单成功
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_request, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_rtn, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_rtn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 7://报单回报后撤单，单成交撤销失败
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_request, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_rtn, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_rtn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 8:  //报单回报延时，单全部成交
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval + TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 9:  //全部成交回报延时，单全部成交
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval + TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 10:  //单分两次成交回报，两个部分成交回报都延时，单全部成交
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval + TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval + TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		//case11:	部分成交，部分成交回报延时，单部分成交
		//(接口acquire_tca_order不对此做区分)
		
		case 12:  //报单回报延时后撤单，撤单成功
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval + TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_request, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_rtn, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_rtn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 13://报单回报延时后撤单，单全部成交撤单失败
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval + TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_cancel_request, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_rtn, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_rtn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 14://报单回报延时后撤单，单分两次成交撤单失败
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval + TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_cancel_request, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_rtn, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_rtn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		//case15:	报单回报延时后撤单，单部分成交部分撤单
		//接口acquire_tca_order不能区分全部成交和部分成交
		
		case 16:  //成交回报延时后撤单，撤单成功
			{
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval + TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_request, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_rtn, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_rtn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 17:  //成交回报延时后撤单，单全部成交撤单失败
			{
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval + TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_request, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				//usleep(order_rtn_interval); 			
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_rtn, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_rtn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 18:  //成交回报延时后撤单，单分两次成交撤单失败
			{
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval + TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_request, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_rtn, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_rtn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		//19	成交回报延时后撤单，单部分成交部分撤单
		//(接口acquire_tca_order不能区分全部成交和部分成交)

		case 20:  //报单回报延时后撤单，撤单成功，但撤单回报延时
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval + TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_request, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval + TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_rtn, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_rtn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 21://报单回报延时后撤单，单全部成交撤单失败，但撤单回报延时
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval + TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_cancel_request, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval + TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_cancel_rtn, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_rtn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 22://不存在的通道报单
			{
				time(&now);
				acquire_tca_order(30, act_request, now*(long)1e6,999);
				g_log.info("acquire_tca_order(30,act_request,%ld,999)\n",now*(long)1e6);
				
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(30, act_response, now*(long)1e6, 999);
				g_log.info("acquire_tca_order(30,act_response,%ld,999)\n",now*(long)1e6);

				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(30, act_tradertn, now*(long)1e6, 999);				
				g_log.info("acquire_tca_order(30,act_tradertn,%ld,999)\n",now*(long)1e6);
			}
			break;
		case 23://未定义的动作
			{
				time(&now);
				acquire_tca_order(tIdx, 9, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,9,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, 9, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,9,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, 9, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,9,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 24://不存在的单号的报单回报
			{
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, 888);
				g_log.info("acquire_tca_order(%d,act_response,%ld,888)\n",tIdx,now*(long)1e6);
			}
			break;
		case 25://不存在的单号的成交回报
			{
				usleep(m_order_rtn_interval - TIME_DELTA); 			
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, 777);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,777)\n",tIdx,now*(long)1e6);
			}
			break;
		case 26://不存在的单号撤单
			{
				time(&now);
				acquire_tca_order(tIdx, act_cancel_request, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 27://不存在的撤单回报
			{
				time(&now);
				acquire_tca_order(tIdx, act_cancel_rtn, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_cancel_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		case 28://重复的报单单号
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_response, now*(long)1e6, m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_response,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);

				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
			
		case 29://时间戳不合规
			{
				acquire_tca_order(tIdx, act_request,0,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,0,%ld)\n",tIdx,m_tunnel_seq_no[i]);
			}
			break;
		case 30://报单后没有收到报单回报，直接收到成交回报
			{
				time(&now);
				acquire_tca_order(tIdx, act_request, now*(long)1e6,m_tunnel_seq_no[i]);
				g_log.info("acquire_tca_order(%d,act_request,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
				
				usleep(m_order_rtn_interval - TIME_DELTA);				
				time(&now);
				acquire_tca_order(tIdx, act_tradertn, now*(long)1e6, m_tunnel_seq_no[i]);				
				g_log.info("acquire_tca_order(%d,act_tradertn,%ld,%ld)\n",tIdx,now*(long)1e6,m_tunnel_seq_no[i]);
			}
			break;
		default:
			g_log.error("[%s]No defined type:%d",__FUNCTION__,type);
			break;			
		}
	}
	
	m_tunnel_seq_no[i]++;
}

void QtmDriver::HandleCriteria(std::vector<string> & vecSplitted)
{
	criteria_t data;	
	
	if(3 == vecSplitted.size())
	{		
		if (strcasecmp(vecSplitted[1].c_str(), "interrupt") == 0)
		{
			data.interrupt = atoi(vecSplitted[2].c_str());
			data.data_loss = m_sec_data_loss;
			
			data.tca_time_out = m_tca_time_out;
			
			set_criteria(&data);
			g_log.info("set_criteria(interrupt=%d,data_loss=%d,tca_time_out=%d)\n",data.interrupt,data.data_loss,data.tca_time_out);
		}
		else if (strcasecmp(vecSplitted[1].c_str(), "dataloss") == 0)
		{
			data.interrupt = m_sec_interrupt;
			data.data_loss = atoi(vecSplitted[2].c_str());
			data.tca_time_out = m_tca_time_out;
			
			set_criteria(&data);
			g_log.info("set_criteria(interrupt=%d,data_loss=%d,tca_time_out=%d)\n",data.interrupt,data.data_loss,data.tca_time_out);
		}
		else if (strcasecmp(vecSplitted[1].c_str(), "datadelay") == 0)
		{


			
			set_criteria(&data);
			g_log.info("set_criteria(interrupt=%d,data_loss=%d,tca_time_out=%d)\n",data.interrupt,data.data_loss,data.tca_time_out);
		}
		else if (strcasecmp(vecSplitted[1].c_str(), "ordertimeout") == 0)
		{
			data.interrupt = m_sec_interrupt;
			data.data_loss = m_sec_data_loss;
			data.tca_time_out = atoi(vecSplitted[2].c_str());
			
			set_criteria(&data);
			g_log.info("set_criteria(interrupt=%d,data_loss=%d,tca_time_out=%d)\n",data.interrupt,data.data_loss,data.tca_time_out);

			m_order_rtn_interval = data.tca_time_out*1000000;
		}
		else
		{
			g_log.error("[%s]No defined Criteria:%s",__FUNCTION__,vecSplitted[1].c_str());
		}
	}
}

void QtmDriver::HandleQinterrupt(std::vector<string> & vecSplitted)
{
	int i = 0;
	
	if(2 == vecSplitted.size())
	{
		i = atoi(vecSplitted[1].c_str());
		m_quote_interrupt[i] = true;
	}
}

void QtmDriver::HandleQloss(std::vector<string> & vecSplitted)
{
	int i = 0;
	
	if(2 == vecSplitted.size())
	{
		i = atoi(vecSplitted[1].c_str());
		m_quote_loss[i] = true;
	}
}

void QtmDriver::HandleQdelay(std::vector<string> & vecSplitted)
{
	int i = 0;
	
	if(2 == vecSplitted.size())
	{
		i = atoi(vecSplitted[1].c_str());
		m_quote_delay[i] = true;
	}
}

void *QtmDriver::handleQuote(void *para)
{
	char time_str[LENGTH_CMD];
	char name[LEN_NAME] = {0};
	int iIdx = -1;
	
	QuoteIdx *pInput = (QuoteIdx *)para;	
	QtmDriver *pInst = (QtmDriver*)pInput->pDriverInstance;
	iIdx = pInput->quote_idx;
	
	sprintf(name,"q%d_%d",iIdx,pInst->m_pid);
	pInst->m_quote_idx[iIdx] += pInst->m_pid;
	set_identification(pInst->m_quote_idx[iIdx],name);
	g_log.info("set_identification(%d,%s)\n",pInst->m_quote_idx[iIdx],name);

	while(1)
	{
		if(!pInst->m_quote_connected[iIdx])
		{
			sleep(1);
			continue;
		}

		if(pInst->m_quote_interrupt[iIdx])
		{
			pInst->QuoteInterrupt(iIdx);
			
			pInst->m_quote_interrupt[iIdx] = false;
			continue;
		}
		
		if(pInst->m_quote_loss[iIdx])
		{
			pInst->QuoteLoss(iIdx);
			
			pInst->m_quote_loss[iIdx] = false;
			continue;
		}
		
		if(pInst->m_quote_delay[iIdx])
		{
			pInst->QuoteDelay(iIdx);
			
			pInst->m_quote_delay[iIdx] = false;			
			continue;
		}
		
		GetNowTimeStr_HHMMSSmmm(time_str,sizeof(time_str));
		acquire_quote_time_field(pInst->m_quote_idx[iIdx],time_str);
		g_log.info("acquire_quote_time_field(%d,%s)\n",pInst->m_quote_idx[iIdx],time_str);
		
		usleep(1000*pInst->m_quote_interval[iIdx]);
	}
	
	return NULL;
}



void QtmDriver::QuoteInterrupt(int iIdx)
{
	char time_str[LENGTH_CMD];

	sleep(m_sec_interrupt);
	GetNowTimeStr_HHMMSSmmm(time_str,sizeof(time_str));
	acquire_quote_time_field(m_quote_idx[iIdx],time_str);
	g_log.info("acquire_quote_time_field(%d,%s)\n",m_quote_idx[iIdx],time_str);
	
	usleep(1000*m_quote_interval[iIdx]);
	GetNowTimeStr_HHMMSSmmm(time_str,sizeof(time_str));
	acquire_quote_time_field(m_quote_idx[iIdx],time_str);
	g_log.info("acquire_quote_time_field(%d,%s)\n",m_quote_idx[iIdx],time_str);
	
	sleep(m_sec_interrupt - 1);
	GetNowTimeStr_HHMMSSmmm(time_str,sizeof(time_str));
	acquire_quote_time_field(m_quote_idx[iIdx],time_str);
	g_log.info("acquire_quote_time_field(%d,%s)\n",m_quote_idx[iIdx],time_str);

}

void QtmDriver::QuoteLoss(int iIdx)
{
	char time_str[LENGTH_CMD];

	sleep(m_sec_data_loss);
	GetNowTimeStr_HHMMSSmmm(time_str,sizeof(time_str));
	acquire_quote_time_field(m_quote_idx[iIdx],time_str);
	g_log.info("acquire_quote_time_field(%d,%s)\n",m_quote_idx[iIdx],time_str);

	usleep(1000*m_quote_interval[iIdx]);
	GetNowTimeStr_HHMMSSmmm(time_str,sizeof(time_str));
	acquire_quote_time_field(m_quote_idx[iIdx],time_str);
	g_log.info("acquire_quote_time_field(%d,%s)\n",m_quote_idx[iIdx],time_str);

	sleep(m_sec_data_loss - 1);
	GetNowTimeStr_HHMMSSmmm(time_str,sizeof(time_str));
	acquire_quote_time_field(m_quote_idx[iIdx],time_str);
	g_log.info("acquire_quote_time_field(%d,%s)\n",m_quote_idx[iIdx],time_str);
}

void QtmDriver::QuoteDelay(int iIdx)
{
	char time_str[LENGTH_CMD];
	
	GetNowTimeStr_HHMMSSmmm(time_str,sizeof(time_str));
	sleep(m_sec_data_delay);
	acquire_quote_time_field(m_quote_idx[iIdx],time_str);
	g_log.info("acquire_quote_time_field(%d,%s)\n",m_quote_idx[iIdx],time_str);

	usleep(1000*m_quote_interval[iIdx]);
	GetNowTimeStr_HHMMSSmmm(time_str,sizeof(time_str));
	acquire_quote_time_field(m_quote_idx[iIdx],time_str);
	g_log.info("acquire_quote_time_field(%d,%s)\n",m_quote_idx[iIdx],time_str);

	GetNowTimeStr_HHMMSSmmm(time_str,sizeof(time_str));
	sleep(m_sec_data_delay - 1);
	acquire_quote_time_field(m_quote_idx[iIdx],time_str);
	g_log.info("acquire_quote_time_field(%d,%s)\n",m_quote_idx[iIdx],time_str);
}

int main(int argc,char **agrv)
{
	QtmDriver qtmdriver;
	
	if(1 == argc)
	{
		printf("Usage: qtmdriver port");
		perror("No listen port, progrem will exit !!");
		return ERR;
	}

	LogConf logConf;
	logConf.fileLogLevel = LOG_LEVEL_DEBUG;
	logConf.stdoutLogLevel = LOG_LEVEL_DEBUG;

	GetExeDir(logConf.logFile,sizeof(logConf.logFile));
	strcat(logConf.logFile,"/qtmdriver.log");
	g_log.init(logConf);

	qtmdriver.run(atoi(agrv[1]));
	
	return SUCC;
}


