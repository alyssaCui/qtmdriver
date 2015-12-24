/**
 * 行情和通道的连接监测库
 * Revison:  20140610 V1.0.0
 *      1)实现行情程序延迟/中断监测;
 * Revison:  20141020 V1.1.0
 *      1)增加通道延迟的检测,增加行情前置连接状态检测，和Oss V2.0.6配合使用;
 * Revison:  20151008 V1.2.0
 *      1) 解决同一进程内行情/通道程序无法区分监测的问题;
 *
 */
#if !defined(_QTM_H_03D7806C_17E9_49CE_9821_12AA667C444A)
#define _QTM_H_03D7806C_17E9_49CE_9821_12AA667C444A

#include <stdio.h>

#define INTERFACE extern "C" __attribute__((visibility("default"))) 

/**
 * @brief 初始化，创建系统消息队列
 * @param [in] type:    TYPE_QUOTE 表明仅监测行情
 *                      TYPE_TCA 表明仅监测通道 
 *                      TYPE_QUOTE|TYPE_TCA 表明同时检测行情和通道
 */
enum tag_type_enum
{
        TYPE_QUOTE=     0x01<<0,
        TYPE_TCA=       0x01<<1
};
INTERFACE void 
qtm_init(int type);

/**
 * @breif 设置被监控的行情源/通道名称，字符串形式，用于RSS上标示行情；
 * @param [in] index: 行情/通道的索引
 * @param [in] name: 名称字符串
 * 
 * NOTE: 行情和通道的索引进程内必须唯一(行情的索引也不允许和通道索引重复);
 */
INTERFACE void 
set_identification(int index, const char *name);

typedef struct 
{
	int interrupt;	
	int data_loss;
	int tca_time_out; 
}criteria_t;
/**
 * @brief 设置延迟判断逻辑模块的告警阈值
 * @param[in] data: 延迟判据数据
 * NOTE: 若未通过该接口设置参数，则内部判断参数保持各参数默认值；
 */
INTERFACE void
set_criteria(criteria_t *data);

/**
 * @brief 监测行情数据延迟/中断
 *      通过行情中的时间戳和本地服务器时间戳比对进行检查。
 * @param [in] index 行情索引
 * @param [in] time_str提取行情中的时间戳，下发给行情延迟判断逻辑模块
 *              时间格式：09:15:00.000 即HH:MM:SS.mmm
 *              H：Hour, M: Minute, S: Second, m: millisecond
 * NOTE: 行情和通道的索引进程内必须唯一(行情的索引也不允许和通道索引重复);
 */
INTERFACE void 
acquire_quote_time_field(int index, const char* time_str);


/* @brief 监测通道报单延迟
 *      通过分析报单回报/成交回报等时间延迟进行判断
 * @param[in] index : 通道索引
 * @param[in] action: 报单动作定义，@see tag_act_enum 
 * @param[in] time_stamp: 报单动作时间戳，1970-1-1 0:0:0开始为微秒数;
 * @param[in] seq_no: 报单流水号
 *
 * NOTE: 行情和通道的索引进程内必须唯一(行情的索引也不允许和通道索引重复);
 */
enum tag_act_enum
{
	act_request=0,
	act_cancel_request,
	act_cancel_rtn,
	act_response,
	act_tradertn,
};
INTERFACE void 
acquire_tca_order(int index, int action, long time_stamp, long seq_no);

/**
 * @brief 更新市场（通道/行情）前置连接状态
 * @param[in] index:行情/通道索引
 * @param[in] type: 表明为行情或通道 @see tag_type_enum
 * @param[in] state_code 状态码,>=0表示正常,<0表示异常
 * @param[in] state_description  状态或步骤描述
 *
 */
INTERFACE void 
update_connection_state(int index, 
                int type,
                int state_code,
                const char * state_description);

/**
 * 清除动作，退出时删除系统消息队列
 * NOTE:消息队列为kernel-persist，退出时，可以选择不删除消息队列；
 */
INTERFACE void 
qtm_finish();


/*********************************************************************** 
 * Obsoleted intrefaces.
 * Will be revmove in V1.3.0
 ***********************************************************************/
INTERFACE void 
init_qdm();
INTERFACE void 
clear_qdm();
INTERFACE void 
set_quote_identity(const char *name, size_t name_size);
INTERFACE void 
set_judge_threshold(void *data, size_t data_size);

/*********************************************************************** 
 * Obsoleted intrefaces.
 * Will be revmove in V1.4.0
 ***********************************************************************/
INTERFACE void 
init_qtm(int type);

INTERFACE void 
set_identity(const char *name, size_t name_size);

INTERFACE void 
acquire_quote_time(const char* time_str);

INTERFACE void 
acquire_tca_monitor_item(int action,
		long time_stamp,
		long seq_no);

INTERFACE void 
update_state(const char * state_name,
		int state_code,
		const char * state_des);

INTERFACE void 
clear_qtm();

#endif 

