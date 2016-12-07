#define RET_OK 0
#define RET_NOK 1

//���ܽ���
#define ENCRTPT 0
#define DECRTPT 1
#define RESULT  2

//socket �ļ�
#define SOCKET_FILE  "/var/socket_file"

#define CRYPTO_FREE(prt) \
do { \
    if (NULL != (prt)) { \
        free(prt); \
        (prt) = NULL; \
    }\
}while(0)

//��Ϣ�ṹ��
typedef struct tagCryptMSGHead
{
    unsigned int crypt;//���ܻ��ǽ���
    unsigned int len;//���Ļ������ĳ���
}CryptMSGHead;


