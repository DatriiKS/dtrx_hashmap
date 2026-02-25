#include "dtrx_hashmap.h"

typedef struct Cmplx_data_foo {
	int ival;
	char *strval;
	float *fval;
} Cmplx_data_foo;

void dtrx_test_foo(){
	dtrx_hashmap *hm = dtrx_new_hashmap(10,0.7f,2);
	
	dtrx_hm_vinsert(hm,"ikey_foo",1,int);

	Cmplx_data_foo *cdp = (Cmplx_data_foo *)malloc(sizeof(Cmplx_data_foo));
	float *flt_ptr = (float *)malloc(sizeof(float));
	*flt_ptr =  2.817f;
	*cdp = (struct Cmplx_data_foo){.ival = 1,.fval = flt_ptr};
	const char *str = "Cmplx";
	cdp->strval = (char *)malloc(strlen(str) + 1);
	strcpy(cdp->strval,str);
	dtrx_callback rcb = DTRX_NONE;
	void **vdp = (void **)malloc(sizeof(void *) * 2);
	vdp[0] = cdp->strval;
	vdp[1] = cdp->fval;
	dtrx_free_info fi = {.free_list = vdp, .count = 2};
	dtrx_hm_rinsert(hm,"rkey_foo",cdp,"%cb %fi",&rcb,fi); 
	if(rcb ^ DTRX_SUCCESS){
		dtrx__safe_free(cdp->strval);
		dtrx__safe_free(cdp->fval);
		dtrx__safe_free(cdp);
	}

	printf(DTRX__CYAN_COLOR "This is a foo hm info\n" DTRX__RESET_COLOR);
	dtrx_delete_hashmap(hm);
}
