#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#include "data_type.h"
#include "errno.h"
#include "alloc.h"
#include "string.h"
#include "basefunc.h"
#include "struct_deal.h"
#include "crypto_func.h"
#include "memdb.h"
#include "message.h"
#include "ex_module.h"
#include "tesi.h"

#include "vtpm_input.h"
#include "../../include/app_struct.h"


static struct timeval time_val={0,50*1000};
int print_error(char * str, int result)
{
	printf("%s %s",str,tss_err_string(result));
}

int vtpm_input_init(void * sub_proc,void * para)
{
	return 0;
}

int vtpm_input_start(void * sub_proc,void * para)
{
	int ret;
	int retval;
	void * recv_msg;
	void * send_msg;
	void * context;
	BYTE uuid[DIGEST_SIZE];
	int i;
	int type;
	int subtype;

	printf("begin vtpm_input start!\n");
	
	sleep(1);
	
	ret=proc_vtpm_input_start(sub_proc,para);
	if(ret<0)
		return ret;

	for(i=0;i<300*1000;i++)
	{
		usleep(time_val.tv_usec);
		ret=ex_module_recvmsg(sub_proc,&recv_msg);
		if(ret<0)
			continue;
		if(recv_msg==NULL)
			continue;

 		type=message_get_type(recv_msg);
		subtype=message_get_subtype(recv_msg);
		
		if((type==DTYPE_VTPM_OUT)&&(subtype==SUBTYPE_EXTEND_OUT))
		{
			proc_extend_out(sub_proc,recv_msg);
			
		}
	}

	return 0;
};


int proc_vtpm_input_start(void * sub_proc,void * para)
{
	int ret;
	int i;
	printf("begin proc vtpm input \n");

	struct start_para * start_para=para;
	
	struct vtpm_in_pcrread * pcrread;

	if((start_para==NULL) || (start_para->argc <2))
		return -EINVAL;

	if(Strcmp(start_para->argv[1],"extend")==0)
		ret=proc_extend_in(sub_proc,para);
	else if(Strcmp(start_para->argv[1],"pcrread")==0)		
		ret=proc_pcrread_in(sub_proc,para);
	else
		return -EINVAL;
	return ret;
}

int proc_extend_in ( void * sub_proc, void * para)
{
	int ret;
	struct start_para * start_para=para;
	struct vtpm_in_extend * extend;
	int  pcrIndex;

	if(start_para->argc <6)
	{
		printf(" error usage! should be %s %s -ix pcrIndex -ic message!",start_para->argv[0],start_para->argv[1] );
		return -EINVAL;	
	}
	void * send_msg=message_create(DTYPE_VTPM_IN,SUBTYPE_EXTEND_IN,NULL);
	if(send_msg==NULL)
		return -EINVAL;
	extend=Calloc(sizeof(*extend));
	if(extend==NULL)
		return -ENOMEM;

	extend->tag=0xC1;
	extend->paramSize=sizeof(*extend);
	extend->ordinal=0x14;

	extend->pcrIndex=Atoi(start_para->argv[3],10);
	calculate_context_sha1(start_para->argv[5],Strlen(start_para->argv[5]),extend->outDigest);
//	SHA_CTX sha;
//	SHA1_Init(&sha);
//	SHA1_Update(&sha,start_para->argv[5],Strlen(start_para->argv[5]));
//	SHA1_Final(extend->outDigest,&sha);
	
	message_add_record(send_msg,extend);

	ret=ex_module_sendmsg(sub_proc,send_msg);
	BYTE digest[DIGEST_SIZE];
	char uuid[DIGEST_SIZE*2];
	Memcpy(digest,extend->outDigest,20);
	digest_to_uuid(digest,uuid);
	uuid[20*2]=0;
	printf("\n Input Digest is %s\n",uuid); 
	return 0;		
}

int proc_pcrread_in ( void * sub_proc, void * para)
{
	int ret;
	struct start_para * start_para=para;
	struct vtpm_in_pcrread * pcrread;
	int  pcrIndex;

	if(start_para->argc <4)
	{
		printf(" error usage! should be %s %s -ix pcrIndex -ic message!",start_para->argv[0],start_para->argv[1] );
		return -EINVAL;	
	}
	void * send_msg=message_create(DTYPE_VTPM_IN,SUBTYPE_PCRREAD_IN,NULL);
	if(send_msg==NULL)
		return -EINVAL;
	pcrread=Calloc(sizeof(*pcrread));
	if(pcrread==NULL)
		return -ENOMEM;

	pcrread->tag=0xC1;
	pcrread->paramSize=sizeof(*pcrread);
	pcrread->ordinal=0x15;

	pcrread->pcrIndex=Atoi(start_para->argv[3],10);
	
	message_add_record(send_msg,pcrread);

	ret=ex_module_sendmsg(sub_proc,send_msg);

	return 0;		
}

int proc_extend_out(void * sub_proc,void * recv_msg)
{
	int ret;
	BYTE digest[DIGEST_SIZE];
	char uuid[DIGEST_SIZE*2];

	struct vtpm_out_extend * extend_out;
	ret=message_get_record(recv_msg,&extend_out,0);
	if(ret<0)
		return ret;
	if(extend_out==NULL)
		return -EINVAL;
	Memcpy(digest,extend_out->outDigest,20);
	digest_to_uuid(digest,uuid);
	uuid[20*2]=0;
	printf("\n New Value of PCR is %s\n",uuid); 
	
	return 0;
}
