#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#define DTRX_HM_IMPLEMENTATION
#include "dtrx_hashmap.h"
#include "test_h.h"

#define INT_STR_BUF 32
char *si_concat(const  char *string,int integer){
	char *buf = (char*)malloc(strlen(string) + INT_STR_BUF);
       	sprintf(buf,"%s%i",string,integer);
	return buf;
}

void say_inner(){printf("Default inner function call\n");}
void say_inner_changed(){printf("Changed inner function call\n");}

typedef struct Inner_data {
	void (* say_ptr)();
} Inner_data;

typedef struct Cmplx_data {
	int ival;
	char *strval;
	float *fval;
	Inner_data *ip ;
} Cmplx_data;

#define COUNTER 10


int main(int argc, char *argv[]){
	dtrx_test_foo();

	dtrx_hashmap *hm = dtrx_new_hashmap(COUNTER,0.7f,2);
	srand(time(NULL));
	// Loop remap test
	uint64_t total_v_int = 0;
	uint64_t total_r_int = 0;
	uint64_t rnd = 0; 
	for(int i = 0; i < COUNTER; i++){	
		rnd = rand() % COUNTER;
		char *vkey = si_concat("ikey",rnd);
		dtrx_callback vcb = DTRX_NONE;
		dtrx_hm_vinsert(hm,vkey,i,int,"%cb",&vcb);
		if(vcb & DTRX_SUCCESS)total_v_int++;
		dtrx__safe_free(vkey);

		char *rkey = si_concat("Cmplx",rnd);
		Cmplx_data *cdp = (Cmplx_data *)malloc(sizeof(Cmplx_data));
		cdp->ip = (Inner_data *)malloc(sizeof(Inner_data));
		*cdp->ip = (struct Inner_data){.say_ptr = &say_inner};
		cdp->fval = (float *)malloc(sizeof(float));
		*cdp->fval =  2.817f;
		const char *str = "Cmplx";
		cdp->strval = (char *)malloc(strlen(str) + 1);
		strcpy(cdp->strval,str);
		dtrx_callback rcb = DTRX_NONE;
		dtrx_hm_rinsert(hm,rkey,cdp,"%cb %  %fp2 %fp1 %",&rcb,cdp->strval,cdp->fval,cdp->ip);
		if(rcb & DTRX_SUCCESS){
			total_r_int++;
		}
		else{
			dtrx__safe_free(cdp->strval);
			dtrx__safe_free(cdp->fval);
			dtrx__safe_free(cdp->ip);
			dtrx__safe_free(cdp);
		}
		dtrx__safe_free(rkey);
	}
	
	printf("Total INT_R: %ld, total INT_V: %ld\n",total_r_int,total_v_int);
	
	printf("\n@!@!@!@!@!@!@!@!@!@!\n DELETING LOOP\n@!@!@!@!@!@!@!@!@!@!\n");
	dtrx_callback rcb = DTRX_NONE;
	for(uint64_t i = (COUNTER/3); i < (COUNTER - COUNTER/3); i++){
		rnd= rand() % COUNTER;
		char *vkey = si_concat("ikey",rnd);
		rcb = dtrx_remove_value(hm,vkey); 
		if(rcb & DTRX_SUCCESS) total_v_int--;
		dtrx__safe_free(vkey);

		char *rkey = si_concat("Cmplx",rnd);
		rcb = dtrx_remove_value(hm,rkey); 
		if(rcb & DTRX_SUCCESS) total_r_int--;
		dtrx__safe_free(rkey);
	}
	
	printf("Total INT_R: %ld, total INT_V: %ld\n",total_r_int,total_v_int);
	
	printf("\n~!~!~!~!~!~!~!~!~!~!\n RETRIEVING LOOP\n~!~!~!~!~!~!~!~!~!~!\n");
	for(uint64_t i = 0; i < hm->size; i++){
		char *vkey = si_concat("ikey",i);
		int *iv = (int *)dtrx_get_value(hm,vkey); 
		if(iv != NULL) {total_v_int--;
		} 
		dtrx__safe_free(vkey);

		char *rkey = si_concat("Cmplx",i);
		Cmplx_data *cdpn = (Cmplx_data *)dtrx_get_value(hm,rkey); 
		if(cdpn != NULL){
			total_r_int--;
		}
		dtrx__safe_free(rkey);
	}
	DTRX_DEBUG_MODE(dtrx__print_hm_info(hm););
	printf("Total INT_R: %ld, total INT_V: %ld\n",total_r_int,total_v_int);
	dtrx_delete_hashmap(hm);
	return 0;
}
