/**
 * �����ͨ�������Ӽ���
 * Revison:  20140610 V1.0.0
 *      1)ʵ����������ӳ�/�жϼ��;
 * Revison:  20141020 V1.1.0
 *      1)����ͨ���ӳٵļ��,��������ǰ������״̬��⣬��Oss V2.0.6���ʹ��;
 * Revison:  20151008 V1.2.0
 *      1) ���ͬһ����������/ͨ�������޷����ּ�������;
 *
 */
#if !defined(_QTM_H_03D7806C_17E9_49CE_9821_12AA667C444A)
#define _QTM_H_03D7806C_17E9_49CE_9821_12AA667C444A

#include <stdio.h>

#define INTERFACE extern "C" __attribute__((visibility("default"))) 

/**
 * @brief ��ʼ��������ϵͳ��Ϣ����
 * @param [in] type:    TYPE_QUOTE �������������
 *                      TYPE_TCA ���������ͨ�� 
 *                      TYPE_QUOTE|TYPE_TCA ����ͬʱ��������ͨ��
 */
enum tag_type_enum
{
        TYPE_QUOTE=     0x01<<0,
        TYPE_TCA=       0x01<<1
};
INTERFACE void 
qtm_init(int type);

/**
 * @breif ���ñ���ص�����Դ/ͨ�����ƣ��ַ�����ʽ������RSS�ϱ�ʾ���飻
 * @param [in] index: ����/ͨ��������
 * @param [in] name: �����ַ���
 * 
 * NOTE: �����ͨ�������������ڱ���Ψһ(���������Ҳ�������ͨ�������ظ�);
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
 * @brief �����ӳ��ж��߼�ģ��ĸ澯��ֵ
 * @param[in] data: �ӳ��о�����
 * NOTE: ��δͨ���ýӿ����ò��������ڲ��жϲ������ָ�����Ĭ��ֵ��
 */
INTERFACE void
set_criteria(criteria_t *data);

/**
 * @brief ������������ӳ�/�ж�
 *      ͨ�������е�ʱ����ͱ��ط�����ʱ����ȶԽ��м�顣
 * @param [in] index ��������
 * @param [in] time_str��ȡ�����е�ʱ������·��������ӳ��ж��߼�ģ��
 *              ʱ���ʽ��09:15:00.000 ��HH:MM:SS.mmm
 *              H��Hour, M: Minute, S: Second, m: millisecond
 * NOTE: �����ͨ�������������ڱ���Ψһ(���������Ҳ�������ͨ�������ظ�);
 */
INTERFACE void 
acquire_quote_time_field(int index, const char* time_str);


/* @brief ���ͨ�������ӳ�
 *      ͨ�����������ر�/�ɽ��ر���ʱ���ӳٽ����ж�
 * @param[in] index : ͨ������
 * @param[in] action: �����������壬@see tag_act_enum 
 * @param[in] time_stamp: ��������ʱ�����1970-1-1 0:0:0��ʼΪ΢����;
 * @param[in] seq_no: ������ˮ��
 *
 * NOTE: �����ͨ�������������ڱ���Ψһ(���������Ҳ�������ͨ�������ظ�);
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
 * @brief �����г���ͨ��/���飩ǰ������״̬
 * @param[in] index:����/ͨ������
 * @param[in] type: ����Ϊ�����ͨ�� @see tag_type_enum
 * @param[in] state_code ״̬��,>=0��ʾ����,<0��ʾ�쳣
 * @param[in] state_description  ״̬��������
 *
 */
INTERFACE void 
update_connection_state(int index, 
                int type,
                int state_code,
                const char * state_description);

/**
 * ����������˳�ʱɾ��ϵͳ��Ϣ����
 * NOTE:��Ϣ����Ϊkernel-persist���˳�ʱ������ѡ��ɾ����Ϣ���У�
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

