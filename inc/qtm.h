/**
 * �����ͨ�������Ӽ���
 * Revison:  20140610 V1.0.0
 *      1)ʵ����������ӳ�/�жϼ��;
 * Revison:  20141020 V1.1.0
 *      1)����ͨ���ӳٵļ��,��������ǰ������״̬��⣬��Oss V2.0.6���ʹ��;
 * Revison:  20151008 V1.2.0
 *      1) ���ͬһ����������/ͨ�������޷����ּ�������;
 * Revison:  20151224 V1.3.0
 *      1) �򻯽ӿڣ�ȥ��ʹ��index��ʶ����/ͨ���ӿڵķ�ʽ,
 *      �޸�Ϊֱ��ʹ��name(char*)��ʶ�ķ�ʽ;
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
#if !defined(TYPE_QUOTE)
#define TYPE_QUOTE     (0x01<<0)
#endif

#if !defined(TYPE_TCA)
#define TYPE_TCA       (0x01<<1)
#endif

INTERFACE void
qtm_init(int type);

/* quote/tunnel judge criteria. */
typedef struct {
        int quote_delay; /* Quote delay, default to 3 seconds. */
        int quote_loss;  /* Quote data loss, default to 2 seconds. */
        int quote_interrupt;  /* Quote interrupted, default to 5 seconds, 
                                maximium :10 seconds. 
                               */
        int order_timeout;  /* Default to 5 seconds. */
} criteria_t;

/**
 * @brief �����ӳ��ж��߼�ģ��ĸ澯��ֵ
 * @param[in] data: �ӳ��о�����
 *
 * NOTE: ��δͨ���ýӿ����ò��������ڲ��жϲ������ָ�����Ĭ��ֵ��
 * NOTE: �ýӿڲ����̰߳�ȫ���������ܿ��ǣ���
 *    ������ڳ�ʼ��qtm��󣬿�ʼ��������/����ǰ �趨�澯��ֵ��
 */
INTERFACE void
set_criteria(const criteria_t *data);

/**
 * @brief ������������ӳ�/�ж�
 *      ͨ�������е�ʱ����ͱ��ط�����ʱ����ȶԽ��м�顣
 * @param [in] name �������ƣ���󳤶�Ϊ32 Bytes���������ֽ����ض�;
 *             NOTE: ������ͬһ�����������ϱ���Ψһ; (�����ͨ��������Ҳ�����໥��ͬ��
 * @param [in] time_str��ȡ�����е�ʱ������·��������ӳ��ж��߼�ģ��
 *              ʱ���ʽ��09:15:00.000 ��HH:MM:SS.mmm
 *              H��Hour, M: Minute, S: Second, m: millisecond
 */
INTERFACE void
acquire_quote_time_field(const char *name, const char *time_str);

/* @brief ���ͨ�������ӳ�
 *      ͨ�����������ر�/�ɽ��ر���ʱ���ӳٽ����ж�
 * @param [in] name �������ƣ���󳤶�Ϊ32 Bytes���������ֽ����ض�;
 *             NOTE: ������ͬһ�����������ϱ���Ψһ; (�����ͨ��������Ҳ�����໥��ͬ��
 * @param[in] action: �����������壬@see tag_act_enum
 * @param[in] time_stamp: ��������ʱ�����Ϊ��1970-1-1 0:0:0��ʼ�����ڵ�΢����;
 * @param[in] seq_no: ������ˮ��
 *
 * NOTE: �����ͨ�������������ڱ���Ψһ(���������Ҳ�������ͨ�������ظ�);
 */
enum tag_act_enum {
        act_request=0,
        act_cancel_request,
        act_cancel_rtn,
        act_response,
        act_tradertn,
};
INTERFACE void
acquire_tca_order(const char *name, int action, long time_stamp, long seq_no);

/**
 * @brief �����г���ͨ��/���飩ǰ������״̬
 * @param [in] name �������ƣ���󳤶�Ϊ32 Bytes���������ֽ����ض�;
 *             NOTE: ������ͬһ�����������ϱ���Ψһ; (�����ͨ��������Ҳ�����໥��ͬ��
 * @param[in] type: ����Ϊ�����ͨ�� @see tag_type_enum
 * @param[in] status ״̬��,>=0��ʾ����,<0��ʾ�쳣
 * @param[in] description  ״̬��������, ��󳤶�Ϊ128 Bytes���������ֽ����ض�;
 *
 */
INTERFACE void
update_connection_state(const char *name,
                        int type,
                        int status,
                        const char *description);

/**
 * ����������˳�ʱɾ��ϵͳ��Ϣ����
 * NOTE:��Ϣ����Ϊkernel-persistent���˳�ʱ������ѡ��ɾ����Ϣ���У�
 */
INTERFACE void
qtm_finish();

#endif

