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

#include "file_struct.h"
#include "tesi_key.h"
#include "tesi_aik_struct.h"
#include "vtpm_pcr.h"

#include "../../include/app_struct.h"

static struct timeval time_val={0,50*1000};
static struct vtpm_pcr_scene * pcr_scenes;

int vtpm_pcr_init(void * sub_proc,void * para)
{
	int i,j;
	pcr_scenes = Calloc(sizeof(struct vtpm_pcr_scene)*vtpm_scene_num);
	if(pcr_scenes==NULL)
		return -ENOMEM;
	for(i=0;i<vtpm_scene_num;i++)
	{
		pcr_scenes[i].index_num=tpm_pcr_index;
		pcr_scenes[i].pcr_size=tpm_pcr_size;
		pcr_scenes[i].pcr=(BYTE *)Calloc0(pcr_scenes[i].index_num*pcr_scenes[i].pcr_size);
		if(pcr_scenes[i].pcr==NULL)
			return -ENOMEM;
	}
	
	ex_module_setpointer(sub_proc,pcr_scenes);
	// prepare the slot sock
	return 0;
}

int vtpm_pcr_start(void * sub_proc,void * para)
{
	int ret;
	int retval;
	void * recv_msg;
	void * context;
	int i;
	int type;
	int subtype;
	void * sock;	
	BYTE uuid[DIGEST_SIZE];

	printf("vtpm_pcr module start!\n");

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
		
		if((type==DTYPE_VTPM_IN)&&(subtype==SUBTYPE_EXTEND_IN))
		{
			proc_vtpm_extend(sub_proc,recv_msg);
		}
		else if((type==DTYPE_VTPM_IN)&&(subtype==SUBTYPE_PCRREAD_IN))
		{
			proc_vtpm_pcrread(sub_proc,recv_msg);
		}
	}

	return 0;
};

int proc_vtpm_extend(void * sub_proc,void * recv_msg)
{
	int ret=0;
	int i=0;
	BYTE buffer[DIGEST_SIZE*2];
	struct vtpm_pcr_scene * pcr_scene = ex_module_getpointer(sub_proc);
	struct vtpm_in_extend * vtpm_extend;
	struct vtpm_out_extend * vtpm_extend_out;
	int pcr_size;
	BYTE * pcr;
	void * send_msg;

	ret = message_get_record(recv_msg,&vtpm_extend,0);
	if(ret<0)
		return ret;
	if(vtpm_extend==NULL)
		return -EINVAL;

	pcr_size=pcr_scene[0].pcr_size;
	pcr=pcr_scene[0].pcr+vtpm_extend->pcrIndex*pcr_size;
	Memcpy(buffer,pcr,pcr_size);
	Memcpy(buffer+pcr_size,vtpm_extend->outDigest,pcr_size);
//	calculate_context_sha1(buffer,pcr_size*2,pcr);
	SHA_CTX sha;
	SHA1_Init(&sha);
	SHA1_Update(&sha,buffer,pcr_size*2);
	SHA1_Final(pcr,&sha);
	
	vtpm_extend_out=Talloc(sizeof(*vtpm_extend_out));
	if(vtpm_extend_out==NULL)
		return -ENOMEM;
	vtpm_extend_out->tag=0xC4;
	vtpm_extend_out->paramSize=sizeof(*vtpm_extend_out);
	vtpm_extend_out->returnCode=0;
	Memcpy(vtpm_extend_out->outDigest,pcr,pcr_size);
	
	send_msg=message_create(DTYPE_VTPM_OUT,SUBTYPE_EXTEND_OUT,recv_msg);
	if(send_msg==NULL)
		return -EINVAL;
	message_add_record(send_msg,vtpm_extend_out);
	ret=ex_module_sendmsg(sub_proc,send_msg);

	return ret;
}

int proc_vtpm_pcrread(void * sub_proc,void * recv_msg)
{
	int ret=0;
	return 0;
}
