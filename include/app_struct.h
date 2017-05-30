#ifndef VTPM_APPSTRUCT_H
#define VTPM_APPSTRUCT_H

enum  vtpm_record_type
{
	DTYPE_VTPM_IN=0x2010,
	DTYPE_VTPM_OUT=0x2020
};
enum vtpm_in_subtype
{
	SUBTYPE_EXTEND_IN=0x14,
	SUBTYPE_PCRREAD_IN=0x15
};

enum vtpm_out_subtype
{
	SUBTYPE_EXTEND_OUT=0x14,
	SUBTYPE_PCRREAD_OUT=0x15
};

struct vtpm_in_pcrread
{
        UINT16 tag;    
        int paramSize; 
	int ordinal;
        int pcrIndex;   
} __attribute__((packed));

struct vtpm_in_extend
{
        UINT16 tag;    
        int paramSize; 
	int ordinal;
        int pcrIndex;   
	BYTE outDigest[20];
} __attribute__((packed));

struct vtpm_out_extend
{
        UINT16 tag;    
        int paramSize; 
	int returnCode;
	BYTE outDigest[20];
} __attribute__((packed));
#endif
