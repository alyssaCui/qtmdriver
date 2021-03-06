#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>
#include "qtm.h"
#include "listenCmd.hpp"

#define MAX_QUOTE 8
#define TIME_ORDER_DELAY_US_DEFAULT 5000000
#define TIME_DELTA    100000 
#define TIME_DELTA_NS 1000 
#define TIME_DELTA_US 2000000
#define TIME_US_5000 5000
#define TIME_DELTA_S  2 
#define LEN_NAME 32

#define printf_green(x,...) printf("\e[32m");printf(x,## __VA_ARGS__);printf("\033[0m");

struct QuoteIdx
{
	void *pDriverInstance;
	int quote_idx;
};


class QtmDriver
{
public:	
	int init();	
	void run(int port);
	void ParseCmd(char *pCmd);
	static void *handleQuote(void *para);
	void HandleQnum(std::vector<string> &);
	void HandleTnum(std::vector<string> &);
	void HandleQinterval(std::vector<string> &);
	void HandleQconn(std::vector<string> &);
	void HandleQlogin(std::vector<string> &);
	void HandleQsubs(std::vector<string> &);
	void HandleQlogout(std::vector<string> &);
	void HandleTconn(std::vector<string> &);
	void HandleTlogin(std::vector<string> &);
	void HandleTlogout(std::vector<string> &);
	void HandleOrder(std::vector<string> &);
	void HandleCriteria(std::vector<string> &);
	void HandleQinterrupt(std::vector<string> &);
	void HandleQloss(std::vector<string> &);
	void HandleQdelay(std::vector<string> &);	
	void HandlePrint(std::vector<string> &);
	void QuoteInterrupt(int iIdx);
	void QuoteLoss(int iIdx);
	void QuoteDelay(int iIdx);
	LockFreeQueue<CMD>* GetQueue(){return &m_queue;}
	
private:
	pid_t m_pid;
	int m_listenPort;
	LockFreeQueue<CMD>  m_queue;
	ListerCmd m_lister;
	
	int m_quote_num;
	int m_quote_interval[MAX_QUOTE];	//ms
	pthread_t m_quote_tid[MAX_QUOTE];
	int m_quote_idx[MAX_QUOTE];
	char m_quote_name[MAX_QUOTE][LEN_NAME];
	bool m_quote_connected[MAX_QUOTE];
	bool m_quote_normal[MAX_QUOTE];
	bool m_quote_interrupt[MAX_QUOTE];
	bool m_quote_loss[MAX_QUOTE];
	bool m_quote_delay[MAX_QUOTE];
	QuoteIdx m_qPara[MAX_QUOTE];
	
	int m_tunnel_num;
	char m_tunnel_name[MAX_QUOTE][LEN_NAME];
	int m_order_rtn_interval;
	int m_tunnel_idx[MAX_QUOTE];
	bool m_tunnel_connected[MAX_QUOTE];
	long m_tunnel_seq_no[MAX_QUOTE]; 

	int m_sec_quote_interrupt;
	int m_sec_quote_loss;	
	int m_sec_quote_delay;
	int m_order_timeout;
};


