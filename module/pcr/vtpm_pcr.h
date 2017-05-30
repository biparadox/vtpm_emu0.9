#ifndef KEY_REQUEST_H
#define KEY_REQUEST_H

int vtpm_pcr_init(void * sub_proc,void * para);
int vtpm_pcr_start(void * sub_proc,void * para);

const int tpm_pcr_size=20;
const int tpm_pcr_index=24;
static int vtpm_scene_num=3;


struct vtpm_pcr_scene
{
	int index_num;
	int pcr_size;
	BYTE  * pcr;	
}__attribute__((packed));

#endif
