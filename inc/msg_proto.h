#if !defined(_PROTO_H_)
#define _PROTO_H_

enum {
        CMD_ERROR_INFO = 2,
};
#define SRC_NAME_LEN    (20)
#define DESC_LEN        (128)
#define SEQ_NO_LEN       (32)

#define XSTR(x) STR(x)
#define STR(x) #x

#if !defined(VERSION)
#define VERSION 1.2
#endif

#define MQ_QDM_NAME  ("/qtm_mq_" XSTR(VERSION))


struct base_proto_t {
        /* Protocol command type: quote/tca
         * @see tag_cmd_type_enum
         */
        int cmd_type;
        /* Market connection source name e.g.(quote or tunnel name) */
        char  src_name[SRC_NAME_LEN];
};

enum tag_cnt_state_enum {
        CNT_QTM_DISCNT_ERROR = -127,
        CNT_ERROR = -1,
        CNT_NORMAL =1,
};

enum tag_quote_interrupt_enum {
        QUOTE_INT_NORMAL = 0,
        QUOTE_INT_DELAY = -1,
        QUOTE_INT_ABNORMAL = -2, 
};

enum tag_quote_dataloss_enum {
        QUOTE_DATA_NORMAL = 0,
        QUOTE_DATA_LOSS = -1,
        QUOTE_DATA_ABNORMAL= -2,
};


enum tag_seq_state_enum {
        SEQ_NO_RSP = 1,
        SEQ_NO_TRADE_RETURN= 2,
};
typedef struct {
        /* 0: normal, -1: delay, -2:abnormal, -3:time out.
         * @see tag_quote_interrupt_enum
         */
        int is_interrupt;
        /* 0: normal, -1: data loss, -2:abnormal.
         * @see tag_quote_dataloss_enum
         */
        int is_data_loss;
        int delay_millisec;
        /* Quote connection state 
         * >=0: normal connection.
         * <0: abnormal connection.
         * @see tag_cnt_state_enum
         */
        int cnt_state;
        /* connection stage descritption
         * e.g. 
         *      a) Quote forwarder connected to broker.
         *      b) Broker authoriazed.
         *      c) Broker connected to exchange.
         */
        char cnt_desc[DESC_LEN];
        /* Quote judge result */
        char judge_desc[DESC_LEN];
} quote_t;



typedef struct {
        char seq_no[SEQ_NO_LEN];
        /* 0: tca_monitor disconnection.
         * 1: No response.
         * 2: No trading return. 
         * @see tag_seq_state_enum
         */
        int seq_stage;
        /* Tunnel connection state 
         * >=0: normal connection.
         * <0: abnormal connection.
         */
        int cnt_state;
        /* connection stage descritption
         * e.g. 
         *      a) Quote forwarder connected to broker.
         *      b) Broker authoriazed.
         *      c) Broker connected to exchange.
         */
        char cnt_desc[DESC_LEN];
        /* Tunnel judge result */
        char judge_desc[DESC_LEN];
} tca_t;

/* Posix Message queue
 * message body definition.
 */
typedef struct {
        public:
                enum tag_cmd_type_enum {
                        QUOTE=0, TCA=1
                };
                base_proto_t base;
                union {
                        quote_t quote;
                        tca_t tca;
                };
}qtm_mon_proto_t;

#endif

