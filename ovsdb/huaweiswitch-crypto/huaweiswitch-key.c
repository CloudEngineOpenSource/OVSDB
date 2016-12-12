#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

#include "huaweiswitch-key.h"
#include "huaweiswitch-crypto-pub.h"

/*****************************************************************************/

// PBKDF2�㷨�ĵ�������
#define ITERATION_NUM       100000

//��־����
#define LOGMSG_MAX_LEN  (2*1024)

//���ܽ���
#define KEYNEW 0
#define KEYOLD 1

//��Կ�����С
#define COMPONENT_SIZE  256

//AES�ӽ��ܿ��С
#ifndef AES_BLOCK_SIZE
#define AES_BLOCK_SIZE 16
#endif

//���Ļ���������󳤶�
//#define TXT_MAX_LEN   1024

//socket��������
#define SOCKET_COUNT 5

//��־
#define LOG_FILE                 "/var/corootkeyproc.log"
#define LOGFILE_BAK              "/var/corootkeyproc.bak.log"

//��Կ����ļ�
#define FILENAME_COMPONE "/usr/share/210p81608dbf7/47329524-d6a6-411c-b1a3-21031608dbf7"
#define FILENAME_COMPTWO "/lib/a6633c855f52/a62bb5bd-fae7-4c75-8989-a66d3c865f52"

//��ֵ�ļ�
#define FILENAME_COMPSALT  "/opt/792b49452ndq1/65jc630o-5t63-2dye-35fs-792b4982ndq1"

//������Կ����ļ�
#define FILENAME_COMPONE_OLD "/var/413ea3oo3ebf3/988fc561-244b-4890-9857-413ea3b3ebf3"
#define FILENAME_COMPTWO_OLD "/mnt/be6961d94f2d/8cd5c096-7455-439c-9236-be69e3d94f2d"

//������ֵ�ļ�
#define FILENAME_COMPSALT_OLD  "/media/a35ui74fg1c/83629a37-9126-l0o2-7838-n20dl398ff12"

/*****************************************************************************/

void print_mem(char * info, unsigned char * msg, unsigned int len)
{
    unsigned int n;

    printf("%s:\r\n", info);

    for (n = 0; n < len; n++) {
        printf("%#02x ", msg[n]);
    }

    printf("\r\nEnd\r\n", info);

    return;
}


/*****************************************************************************
Func Name       : corootkeyprc_log
Date Created    : 2016-11-10
Author          : lishiyang 00193436
Description     : ��־����
Input           : pcFmt
Output          :
Return          : NULL
Caution         :
******************************************************************************/
void corootkeyprc_log(const char* pcFmt, ...)
{
    struct timeval tv;
    struct tm*     ptm;
    va_list        pcArgs;
    const char     szTimeFmt[] = "%Y-%m-%d %H:%M:%S";
    char           szLogBuf[LOGMSG_MAX_LEN] = {0};
    char           szCurTime[64];
    FILE*          pstFile;

    (void)memset(&pcArgs, 0, sizeof(va_list));

    pstFile = fopen(LOG_FILE, "a");
    if (NULL == pstFile)
    {
        return;
    }

    if (ftell(pstFile) > LOGMSG_MAX_LEN)
    {
        (void)fclose(pstFile);
        (void)unlink(LOGFILE_BAK);
        (void)rename(LOG_FILE,LOGFILE_BAK);
        pstFile = fopen(LOG_FILE, "a");
        if (NULL == pstFile)
        {
            return;
        }
    }

    (void)gettimeofday(&tv, NULL);
    ptm = gmtime(&tv.tv_sec);
    if (NULL == ptm)
    {
        (void)fclose(pstFile);
        return;
    }
    (void)strftime(szCurTime, sizeof(szCurTime), szTimeFmt, ptm);

    va_start(pcArgs, pcFmt);
    (void)vsnprintf(szLogBuf, LOGMSG_MAX_LEN, pcFmt, pcArgs);

    va_end(pcArgs);
    szLogBuf[LOGMSG_MAX_LEN - 1] = 0;
    (void)fprintf(pstFile, "[%s.%ld] %s\r\n", szCurTime, (long)tv.tv_usec, szLogBuf);

    (void)fclose(pstFile);
}

/*****************************************************************************
Func Name       : genrootkey
Date Created    : 2016-11-15
Author          : lishiyang 00193436
Description     : ����Կ���ɺ���
Input           : keyver����Կ�汾
Output          :rootKey����Կ
Return          : NULL
Caution         :
******************************************************************************/
int genrootkey(int keyver,unsigned char rootKey[COMPONENT_SIZE])
{
    unsigned char component1[COMPONENT_SIZE] = {0};
    unsigned char component2[COMPONENT_SIZE] = {0};
    unsigned char salt[COMPONENT_SIZE] = {0};
    unsigned char tmpComponent[COMPONENT_SIZE] = {0};
    int amount;
    int i;
    int rv;
    FILE * fp;

    //�������¸���Կ
    if(keyver == KEYNEW)
    {
        //��ȡ��Կ���һ
        fp=fopen(FILENAME_COMPONE,"r");
        if(fp == NULL)
        {
            corootkeyprc_log("Error:open file failed\n");
            return RET_NOK;
        }
        amount = fread(component1,sizeof(unsigned char), COMPONENT_SIZE,fp);
        if (amount < 0)
        {
            corootkeyprc_log("Error:get component failed\n");
            return RET_NOK;
        }
        fclose(fp);
        
        //��ȡ��Կ�����
        fp=fopen(FILENAME_COMPTWO,"r");
        if(fp == NULL)
        {
            corootkeyprc_log("Error:open file failed!\n");
            return RET_NOK;
        }
        amount = fread(component2,sizeof(unsigned char), COMPONENT_SIZE,fp);
        if (amount < 0)
        {
            corootkeyprc_log("Error:get component failed!\n");
            return RET_NOK;
        }
        fclose(fp);
        
        //��ȡ��ֵ
        fp=fopen(FILENAME_COMPSALT,"r");
        if(fp == NULL)
        {
            corootkeyprc_log("Error:open file failed!!\n");
            return RET_NOK;
        }
        amount = fread(salt,sizeof(unsigned char), COMPONENT_SIZE,fp);
        if (amount < 0)
        {
            corootkeyprc_log("Error:get salt failed!!\n");
            return RET_NOK;
        }
        fclose(fp);
    }
    
    //�����ϴα��ݸ���Կ
    else if(keyver == KEYOLD)
    {
        //��ȡ����Կ���һ
        fp=fopen(FILENAME_COMPONE_OLD,"r");
        if(fp == NULL)
        {
            corootkeyprc_log("Error:open old file failed\n");
            return RET_NOK;
        }
        amount = fread(component1,sizeof(unsigned char), COMPONENT_SIZE,fp);
        if (amount < 0)
        {
            corootkeyprc_log("Error:get old component failed\n");
            return RET_NOK;
        }
        fclose(fp);
        
        //��ȡ����Կ�����
        fp=fopen(FILENAME_COMPTWO_OLD,"r");
        if(fp == NULL)
        {
            corootkeyprc_log("Error:open old file failed!\n");
            return RET_NOK;
        }
        amount = fread(component2,sizeof(unsigned char), COMPONENT_SIZE,fp);
        if (amount < 0)
        {
            corootkeyprc_log("Error:get old component failed!\n");
            return RET_NOK;
        }
        fclose(fp);

        //��ȡ����ֵ
        fp=fopen(FILENAME_COMPSALT_OLD,"r");
        if(fp == NULL)
        {
            corootkeyprc_log("Error:open old file failed!!\n");
            return RET_NOK;
        }
        amount = fread(salt,sizeof(unsigned char), COMPONENT_SIZE,fp);
        if (amount < 0)
        {
            corootkeyprc_log("Error:get old salt failed!!\n");
            return RET_NOK;
        }
        fclose(fp);
    }

    // ������Կ���
    for(i = 0; i < COMPONENT_SIZE; i++)
    {
        tmpComponent[i] = component1[i] ^ component2[i];
    }

    // ������Կ�������ɸ���Կ
    rv = PKCS5_PBKDF2_HMAC( (const char *)tmpComponent,
                                COMPONENT_SIZE,
                                (const unsigned char *)salt,
                                COMPONENT_SIZE,
                                ITERATION_NUM,
                                EVP_sha256(),
                                32,
                                rootKey);

    if(rv == 0)
    {
        corootkeyprc_log("Error:rootkey generate fail\n");
        return RET_NOK;
    }

    return RET_OK;
}

/*****************************************************************************
Func Name       : parse_msg
Date Created    : 2016-11-15
Author          : lishiyang 00193436
Description     : ��Ϣ������
Input           : crypt������/���� len��txt���� txt������/����
Output          :tmpanswer:����/���ܽ��
Return          : NULL
Caution         :
******************************************************************************/
int parse_msg(CryptMSGHead * pstMSGHead, unsigned char *in, unsigned char ** result, unsigned int * resultlen)
{
    
    switch(pstMSGHead->crypt)
    {
        case ENCRTPT:
        {
            int ret;
            unsigned char rootKey[COMPONENT_SIZE];
            unsigned int length = 0;
            unsigned char * out = NULL;
            AES_KEY aes_enc_ctx;

            //������¸���Կ
            ret = genrootkey(KEYNEW, rootKey);
            if (ret != RET_OK) {
                corootkeyprc_log("Error: generate root key fail!\n");
                return RET_NOK;
            }

            AES_set_encrypt_key(rootKey, COMPONENT_SIZE, &aes_enc_ctx);

            //���ô������ĳ���Ϊ16�������������������16����������Ĭ�ϼ�һ��16��ΪPKCS5����ʹ�á�
            length = pstMSGHead->len + AES_BLOCK_SIZE - (pstMSGHead->len % AES_BLOCK_SIZE);
            out = (unsigned char *)malloc(length);
            if (NULL == out) {
                corootkeyprc_log("Error: malloc out fail!\n");
                return RET_NOK;
            }
            (void)memset(out, 0, length);

            *result = out;
            *resultlen = length;

            unsigned int len = length;
            unsigned int n;
            unsigned char * iv = "e588fb8e2fd4704c";
            memcpy(out, in, pstMSGHead->len);

            //pading
            unsigned int pad = AES_BLOCK_SIZE - pstMSGHead->len % AES_BLOCK_SIZE;
            for (n = pstMSGHead->len; n < length; n++) {
                out[n] = (0 == pad) ? AES_BLOCK_SIZE : pad;
            }

            //16λΪһ��ȥ����ԭʼ����
            while (len >= AES_BLOCK_SIZE)
            {
                for(n=0; n < AES_BLOCK_SIZE; ++n)
                    out[n] = out[n] ^ iv[n];

                AES_encrypt(out, out, &aes_enc_ctx);
                iv = out;
                len -= AES_BLOCK_SIZE;
                out += AES_BLOCK_SIZE;
            }

            break;
        }

        case DECRTPT:
        {
            int ret;
            unsigned char rootKey[COMPONENT_SIZE];
            AES_KEY aes_dec_ctx;
            int j = 0;
            unsigned int length = 0;
            unsigned int n = 0;
            unsigned char * out = NULL;
            unsigned char * iv = "e588fb8e2fd4704c";

            if (0 != pstMSGHead->len % AES_BLOCK_SIZE) {
                corootkeyprc_log("Error: length is wrong, len = %d.\n", pstMSGHead->len);
                return RET_NOK;
            }

            //������¸���Կ
            ret = genrootkey(KEYNEW, rootKey);
            if (ret != RET_OK) {
                corootkeyprc_log("Error: generate root key fail!\n");
                return RET_NOK;
            }
            
            AES_set_decrypt_key(rootKey, COMPONENT_SIZE, &aes_dec_ctx);

            out = (unsigned char *)malloc(pstMSGHead->len);
            if (NULL == out) {
                corootkeyprc_log("Error: malloc out fail!\n");
                return RET_NOK;
            }
            (void)memcpy(out, in, pstMSGHead->len);

            *result = out;

            while(length < pstMSGHead->len) {
                AES_decrypt(out, out, &aes_dec_ctx);

                for (n = 0; n < AES_BLOCK_SIZE; n++)
                    out[n] = out[n] ^ iv[n];

                iv = in + length;
                out += AES_BLOCK_SIZE;
                length += AES_BLOCK_SIZE;
            }

            out = *result;
            length = pstMSGHead->len;
            for (n = out[pstMSGHead->len - 1]; n > 0; n--) {
                out[pstMSGHead->len - n] = '\0';
                length--;
            }

            *resultlen = length;

            break;
        }

        default:
        {
            corootkeyprc_log("Error:parse msg fail\n");
            return RET_NOK;
        }
    }

    return RET_OK;
}

/*****************************************************************************
Func Name       : main
Date Created    : 2016-11-09
Author          : lishiyang 00193436
Description     : ����ں���
Input           :
Output          :
Return          :
Caution         :
******************************************************************************/
int main(void)
{
    struct sockaddr_un address;
    int sock, conn;
    size_t addrLength;
    CryptMSGHead cryptHead;
    int size;
    int ret;
    unsigned char * resultanswer = NULL;
    unsigned char * msg = NULL;
    unsigned int resultlen = 0;
    unsigned char * returnMSG = NULL;
    CryptMSGHead * returnHead = NULL;

    daemon(0,0);

    corootkeyprc_log("INFO:COROOTKEYPROC\n");
    
    //����unix domain socket
    if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        corootkeyprc_log("Error:create socket fail\n");
        return RET_NOK;
    }

    //ɾ�����ڵ�socket�ļ�
    unlink(SOCKET_FILE);

    //����socket��ַ
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCKET_FILE);

    //��socket
    addrLength = sizeof(address.sun_family) + strlen(address.sun_path);
    if (bind(sock, (struct sockaddr *) &address, addrLength))
    {
        corootkeyprc_log("Error:bind socket fail\n");
        return RET_NOK;
    }

    /* �޸��ļ�Ȩ�� */
    chmod(SOCKET_FILE, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

    //����socket
    if (listen(sock, SOCKET_COUNT))
    {
        corootkeyprc_log("Error:listen socket fail\n");
        return RET_NOK;
    }

    //���ܿͻ������Ӳ�����
    while(1) {
        //�пͻ�������
        if ((conn = accept(sock, (struct sockaddr *) &address,&addrLength)) < 0) {
            sleep(1);
            continue;
        }

        /*��ȡǰ8���ֽڣ�crypt+len
        ���ڲ�֪�����յ������ݳ��ȣ������Ҫ�Ȼ�ȡlen*/
        size = recv(conn, &cryptHead, sizeof(cryptHead), 0);
        if(size < 0) {
            corootkeyprc_log("Error: Recieving msg head faild, size = %d.\n", size);
            close(conn);
            continue;
        }

        if (0 == cryptHead.len || cryptHead.crypt > DECRTPT) {
            close(conn);
            continue;
        }

        //��ȡtxt
        msg = (unsigned char*)malloc(cryptHead.len);
        if (NULL == msg) {
            corootkeyprc_log("Error: Malloc msg faild, length = %d.\n", cryptHead.len);
        }
        (void)memset(msg, 0, cryptHead.len);

        size = recv(conn, msg, cryptHead.len, 0);
        if(size < 0) {
            corootkeyprc_log("Error: Recieving msg msg faild, len = %d, size = %d.\n", cryptHead.len, size);
            CRYPTO_FREE(msg);
            close(conn);
            continue;
        }

        //������Ϣ���õ�������
        ret = parse_msg(&cryptHead, msg, &resultanswer, &resultlen);
        if(ret != RET_OK) {
            corootkeyprc_log("Error:operate failed\n");
            CRYPTO_FREE(msg);
            close(conn);
            continue;
        }

        returnMSG = (unsigned char *)malloc(sizeof(CryptMSGHead) + resultlen);
        if (NULL == returnMSG) {
            CRYPTO_FREE(resultanswer);
            CRYPTO_FREE(msg);
            close(conn);
            continue;
        }

        returnHead = (CryptMSGHead *)(void *)returnMSG;
        returnHead->crypt = RESULT;
        returnHead->len = resultlen;
        (void)memcpy(returnMSG+sizeof(CryptMSGHead), resultanswer, resultlen);
        
        //�ظ�������
        size = send(conn, returnMSG, (sizeof(CryptMSGHead) + resultlen), 0);
        if(size < 0) {
            corootkeyprc_log("Error: error when Sending Data\n");
        }

        CRYPTO_FREE(returnMSG);
        returnHead = NULL;
        CRYPTO_FREE(resultanswer);
        resultlen = 0;
        CRYPTO_FREE(msg);
        memset(&cryptHead, 0, sizeof(cryptHead));
        close(conn);
        continue;
    }
}
