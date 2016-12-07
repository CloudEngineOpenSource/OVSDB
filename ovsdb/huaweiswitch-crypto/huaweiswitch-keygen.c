#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "huaweiswitch-key.h"
#include "huaweiswitch-crypto-pub.h"

/*****************************************************************************/
//�����θ���
#define ARGC_MAX 2

#define RET_OK 0
#define RET_NOK 1

//��Կ�����С
#define COMPONENT_SIZE  256

//�ļ����Ƴ���
#define FILE_NAME_SIZE  32

//�ļ�·������
#define PATH_NAME_SIZE  100


//����Ȩ��
#define ACCESS_LIMIT    0600

//��־����
#define LOGMSG_MAX_LEN  (2*1024)

//��־
#define LOG_FILE         "/var/210dbf7/corootkey.log"
#define LOGFILE_BAK      "/var/210dbf7/corootkey.bak.log"

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

static int comp_create();

/*****************************************************************************
Func Name       : corootkey_log
Date Created    : 2016-11-09
Author          : lishiyang 00193436
Description     : ��־����
Input           : pcFmt
Output          :
Return          : NULL
Caution         :
******************************************************************************/
void corootkey_log(const char* pcFmt, ...)
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
    (void)fprintf(pstFile, "[%s.%ld] %s.\r\n", szCurTime, (long)tv.tv_usec, szLogBuf);

    (void)fclose(pstFile);
}

/*****************************************************************************
Func Name       : corootkey_backup
Date Created    : 2016-11-09
Author          : lishiyang 00193436
Description     : f1�����ļ���f2
Input           : 
Output          :
Return          : 
Caution         :
******************************************************************************/
static int corootkey_backup(char *f1,char *f2)
{
    char buffer[COMPONENT_SIZE];
    FILE *in,*out;
    int len;

    corootkey_log("backup corootkey begin.");

    if((in=fopen(f1,"r"))==NULL)
    {
        corootkey_log("the file can not open.");
        return RET_NOK;
    }
    if((out=fopen(f2,"w"))==NULL)
    {
        corootkey_log("the old file can not open.");
        fclose(in);
        return RET_NOK;
    }
    while((len=fread(buffer,sizeof(char),COMPONENT_SIZE,in))>0)
    {
        fwrite(buffer,sizeof(char),len,out);
        memset(buffer,0,COMPONENT_SIZE);
    }

    fclose(out);
    fclose(in);

    corootkey_log("backup corootkey end.");

    return RET_OK;
}

/*****************************************************************************
Func Name       : corootkey_update
Date Created    : 2016-11-09
Author          : lishiyang 00193436
Description     : ������Կ�������ֵ�ļ�
Input           : 
Output          :
Return          : 
Caution         :
******************************************************************************/
static int corootkey_update()
{
    int ret;
    
    corootkey_log("update corootkey begin.");
    
    //����������Կ�������ֵ�ļ�Ŀ¼��Ȩ��Ϊroot����
    (void)mkdir("/var/413ea3oo3ebf3", ACCESS_LIMIT);
    (void)mkdir("/mnt/be6961d94f2d", ACCESS_LIMIT);
    (void)mkdir("/media/a35ui74fg1c", ACCESS_LIMIT);
    
    
    //������Կ���һ
    ret = corootkey_backup(FILENAME_COMPONE,FILENAME_COMPONE_OLD);
    if(ret != RET_OK)
    {
        corootkey_log("Error:backup key failed.");
        return RET_NOK;
    }
    (void)chmod(FILENAME_COMPONE_OLD, ACCESS_LIMIT);

    //������Կ�����
    ret = corootkey_backup(FILENAME_COMPTWO,FILENAME_COMPTWO_OLD);
    if(ret != RET_OK)
    {
        corootkey_log("Error:backup key failed.");
        return RET_NOK;
    }
    (void)chmod(FILENAME_COMPTWO_OLD, ACCESS_LIMIT);
    
    //������ֵ
    ret = corootkey_backup(FILENAME_COMPSALT,FILENAME_COMPSALT_OLD);
    if(ret != RET_OK)
    {
        corootkey_log("Error:backup key failed.");
        return RET_NOK;
    }
    (void)chmod(FILENAME_COMPSALT_OLD, ACCESS_LIMIT);
    
    //����������Կ
    ret = comp_create();
    if(ret != RET_OK)
    {
        corootkey_log("Error:update key failed.");
        return RET_NOK;
    }
    
    printf("Info:update key done.\r\n");
    corootkey_log("update corootkey end.");
    
    return RET_OK;
}

/*****************************************************************************
Func Name       : corootkey_destory
Date Created    : 2016-11-09
Author          : lishiyang 00193436
Description     : ������Կ�������ֵ�ļ�
Input           : 
Output          :
Return          : 
Caution         :
******************************************************************************/
static void corootkey_destory()
{
    corootkey_log("destory corootkey begin.");

    //������Կ�������ֵ�ļ�����־
    (void)system("shred -f -u  -z /usr/share/210p81608dbf7/47329524-d6a6-411c-b1a3-21031608dbf7 1>/dev/null 2>/dev/null");
    (void)system("shred -f -u  -z /lib/a6633c855f52/a62bb5bd-fae7-4c75-8989-a66d3c865f52 1>/dev/null 2>/dev/null");
    (void)system("shred -f -u  -z /var/413ea3oo3ebf3/988fc561-244b-4890-9857-413ea3b3ebf3 1>/dev/null 2>/dev/null");
    (void)system("shred -f -u  -z /mnt/be6961d94f2d/8cd5c096-7455-439c-9236-be69e3d94f2d 1>/dev/null 2>/dev/null");
    (void)system("shred -f -u  -z /opt/792b49452ndq1/65jc630o-5t63-2dye-35fs-792b4982ndq1 1>/dev/null 2>/dev/null");
    
    printf("Info:destory key done.\r\n");
    corootkey_log("destory corootkey end.");
}

/*****************************************************************************
Func Name       : comp_store
Date Created    : 2016-11-09
Author          : lishiyang 00193436
Description     : �洢��Կ�������ֵ�ļ�
Input           : 
Output          :
Return          : 
Caution         :
******************************************************************************/
static int comp_store(unsigned char component1[COMPONENT_SIZE],unsigned char component2[COMPONENT_SIZE],unsigned char salt[COMPONENT_SIZE] )
{
    FILE *fp;
    int i;
    
    corootkey_log("store corootkey begin.");
    
    //������Կ�������ֵ�ļ�Ŀ¼��Ȩ��Ϊroot����
    (void)mkdir("/usr/share/210p81608dbf7", ACCESS_LIMIT);
    (void)mkdir("/lib/a6633c855f52/", ACCESS_LIMIT);
    (void)mkdir("/opt/792b49452ndq1", ACCESS_LIMIT);
    
    //��Կ���һд���ļ�
    fp=fopen(FILENAME_COMPONE,"wt+"); 
    if(fp == NULL)
    {
        corootkey_log("Error:open file failed.");
        return RET_NOK;
    }
    for (i=0;i<COMPONENT_SIZE;i++)
    {
        (void)fprintf(fp,"%c",component1[i]);
    }
    (void)fclose(fp);
    
    (void)chmod(FILENAME_COMPONE, ACCESS_LIMIT);
    
    //��Կ�����д���ļ�
    fp=fopen(FILENAME_COMPTWO,"wt+"); 
    if(fp == NULL)
    {
        corootkey_log("Error:open file failed.");
        return RET_NOK;
    }
    for (i=0;i<COMPONENT_SIZE;i++)
    {
        (void)fprintf(fp,"%c",component2[i]);
    }
    (void)fclose(fp);
    
    (void)chmod(FILENAME_COMPTWO, ACCESS_LIMIT);
    
    //��ֵд���ļ�
    fp=fopen(FILENAME_COMPSALT,"wt+"); 
    if(fp == NULL)
    {
        corootkey_log("Error:open file failed.");
        return RET_NOK;
    }
    for (i=0;i<COMPONENT_SIZE;i++)
    {
        (void)fprintf(fp,"%c",salt[i]);
    }
    (void)fclose(fp);
    
    (void)chmod(FILENAME_COMPSALT, ACCESS_LIMIT); 

    corootkey_log("store corootkey end.");
    
    return RET_OK;
}

/*****************************************************************************
Func Name       : comp_create
Date Created    : 2016-11-09
Author          : lishiyang 00193436
Description     : ������Կ�������ֵ
Input           : 
Output          :
Return          : 
Caution         :
******************************************************************************/
static int comp_create()
{
    unsigned char component1[COMPONENT_SIZE] = {0};
    unsigned char component2[COMPONENT_SIZE] = {0};
    unsigned char salt[COMPONENT_SIZE] = {0};
    unsigned char input[COMPONENT_SIZE] = {0};
    int ret;
    int i;
    unsigned int length = COMPONENT_SIZE;
    
    corootkey_log("create corootkey begin.");
    
    //�û�������Ϊ��Կ���һ   
    printf("Please input a string as a component, and max length of string is 256:\r\n");
    printf("your input is:");
    Getpassword(input, &length);

    if((input == NULL) || (strlen(input) > COMPONENT_SIZE))
    {
        printf("Error:your input is null or exceeds the limit.\r\n");
        return RET_NOK;
    }
    
    strcpy(component1,input);
    RAND_bytes(component1+strlen(input), COMPONENT_SIZE - strlen(input));
    
    // ������Կ�����
    RAND_bytes(component2, COMPONENT_SIZE);

    // ����PBKDF2�㷨�������ֵ
    RAND_bytes(salt, COMPONENT_SIZE);

    //�洢����Կ�������ֵ
    ret = comp_store(component1,component2,salt);
    if(ret == RET_NOK)
    {
        corootkey_log("Error:store rootkey failed.");
        return RET_NOK;
    }
    
    printf("Info:create key done.\r\n");
    corootkey_log("create corootkey end.");
    
    return RET_OK;
}

/*****************************************************************************
Func Name       : parse_options
Date Created    : 2016-11-09
Author          : lishiyang 00193436
Description     : ������Σ�ʵ�ʴ���
Input           : 
Output          :
Return          : 
Caution         :
******************************************************************************/
static int parse_options(int argc, char *argv[])
{
    int ret;
    
    corootkey_log("parse options begin.");
    
    //��θ����Ƿ�
    if(argc != ARGC_MAX)
    {
        printf("Error:para count error.\r\n");
        printf("Usage:cerootkey {create | update | destory}.\r\n");
        return RET_NOK;
    }
    
    //���create
    if(strcmp(argv[1],"create") == 0)
    {
        printf("Info:create key.\r\n");
        ret = comp_create();
        if(ret != RET_OK)
        {
            corootkey_log("Error:create key failed.");
            return RET_NOK;
        }
    }
    //���update
    else if(strcmp(argv[1],"update") == 0)
    {
        printf("Info:update key.\r\n");
        ret = corootkey_update();
        if(ret != RET_OK)
        {
            corootkey_log("Error:update key failed.");
            return RET_NOK;
        }
    }
    //���destory
    else if(strcmp(argv[1],"destory") == 0)
    {
        printf("Info:destory key.\r\n");
        corootkey_destory();
    }
    //��ηǷ�
    else
    {
        printf("Error:unknown option.\r\n");
        printf("Usage:cerootkey {create | update | destory}.\r\n");
        return RET_NOK;
    }
    
    corootkey_log("parse options end.");
    
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
int main(int argc, char *argv[])
{
    int ret;
    
    printf("Info:make sure excute this command in root.\r\n");
    corootkey_log("COROOTKEY BEGIN\n");
    //������־Ŀ¼��Ȩ��Ϊroot����
    (void)mkdir("/var/210dbf7", ACCESS_LIMIT);
    
    //������Σ�ʵ�ʴ���
    ret = parse_options(argc, argv);
    if(ret != RET_OK)
    {
        corootkey_log("Error:operate failed.");
        printf("Error:operate failed.\r\n");
        return RET_NOK;
    }

    return RET_OK;
}

