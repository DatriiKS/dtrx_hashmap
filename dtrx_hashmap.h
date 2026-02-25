#include <stdlib.h>
#include <assert.h>
#include <stdio.h> 
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#ifndef DTRX_HASHMAP_H
#define DTRX_HASHMAP_H

#ifdef DTRX_DEBUG
	#define DTRX_DEBUG_MODE(code) code
#else 
	#define DTRX_DEBUG_MODE(code)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define dtrx_hm_rinsert(hm,key,ptr,...) dtrx_hm_rinsert_step_two(hm,key,ptr,##__VA_ARGS__,"\0")
#define dtrx_hm_rinsert_step_two(hm,key,ptr,fmt,...) { \
	do{ \
		void *_macro_tmp_ptr = ptr; \
		dtrx_free_info _macro_fi= NULL__INFO; \
		dtrx__va_args_info _macro_info = dtrx__parse_format((char *)fmt,##__VA_ARGS__); \
		if(_macro_info.f_ptrc > 0) {_macro_fi.free_list = _macro_info.free_list; _macro_fi.count = _macro_info.f_ptrc;} \
		if(_macro_info.fi.count > 0) _macro_fi = _macro_info.fi; \
		dtrx_callback _macro_callback = dtrx__add_value(hm,key,_macro_tmp_ptr,_macro_fi); \
		if(_macro_info.cb_ptr != NULL) *_macro_info.cb_ptr = _macro_callback;\
		if(_macro_callback & DTRX_FAILURE){ \
			dtrx__safe_free(_macro_fi.free_list); \
			break; \
		} \
		ptr = NULL; \
	}while(0); \
}
	
#define dtrx_hm_vinsert(hm,key,val,type,...) dtrx_hm_vinsert_step_two(hm,key,val,type,##__VA_ARGS__,"\0")
#define dtrx_hm_vinsert_step_two(hm,key,val,type,fmt,...) { \
	type *_macro_ptr = (type *)malloc(sizeof(type)); \
	if(_macro_ptr == NULL) DTRX__PANIC("MALLOC RETURNED NULL"); \
	*_macro_ptr = val; \
	dtrx_free_info _macro_fi = NULL__INFO; \
	dtrx__va_args_info _macro_info = dtrx__parse_format((char *)fmt,##__VA_ARGS__); \
	if(_macro_info.f_ptrc > 0) {_macro_fi.free_list = _macro_info.free_list; _macro_fi.count = _macro_info.f_ptrc;} \
	if(_macro_info.fi.count > 0) _macro_fi = _macro_info.fi; \
	dtrx_callback _macro_callback = dtrx__add_value(hm,key,(void *)_macro_ptr,_macro_fi); \
	if(_macro_info.cb_ptr != NULL) *_macro_info.cb_ptr = _macro_callback;\
	if(_macro_callback & DTRX_FAILURE){ \
		dtrx__safe_free(_macro_fi.free_list); \
		dtrx__safe_free(_macro_ptr); \
	} \
}

typedef struct dtrx__hm_entry dtrx__hm_entry;

typedef struct dtrx_hashmap {
	uint64_t size;
	uint64_t count;
	uint64_t remap_multiplier;
	uint64_t remap_limit;
	dtrx__hm_entry *data_block;
} dtrx_hashmap;

typedef enum dtrx_callback {
	DTRX_NONE = 0,
	DTRX_SUCCESS = 1 << 0,
	DTRX_FAILURE = 1 << 1
} dtrx_callback;

typedef struct dtrx_free_info {
	void **free_list;
	uint64_t count;
} dtrx_free_info;

dtrx_hashmap *dtrx_new_hashmap(uint64_t,float,uint64_t);
dtrx_callback dtrx_remove_value(dtrx_hashmap *, const char *);
void * dtrx_get_value(dtrx_hashmap *,const char *);
void dtrx_delete_hashmap(dtrx_hashmap *);


#ifdef DTRX_HM_IMPLEMENTATION
#ifndef DTRX_HM_IMPLEMENTATION_DONE
#define DTRX_HM_IMPLEMENTATION_DONE

#define DTRX__YELLOW_COLOR "\033[33m"
#define DTRX__RED_COLOR "\033[31m"
#define DTRX__CYAN_COLOR "\033[36m"
#define DTRX__RESET_COLOR "\033[0m"

#define DTRX__PANIC(msg) do { \
	fprintf(stderr, DTRX__RED_COLOR "ERROR " DTRX__RESET_COLOR "IN FUNCTION \"%s\" IN %s:%d! " msg "\n",__func__,__FILE__,__LINE__); \
	exit(EXIT_FAILURE); \
} while(0) 

#define dtrx__safe_free(ptr) do { \
	if(ptr != NULL) {free(ptr);ptr = NULL;} \
} while(0)

#define dtrx__free_entry(entry_ptr) do { \
	uint64_t i; \
	if(entry_ptr->fi.count > 0){ \
		for(i = 0; i < entry_ptr->fi.count; i++){ \
			if(entry_ptr->fi.free_list[i] == NULL) DTRX__PANIC("ATTEMPTING TO FREE A NULL POINTER, PROBABLY ALREADY BEEN FREED SOMEWHERE IN THE CODE\n"); \
			dtrx__safe_free(entry_ptr->fi.free_list[i]); \
		} \
	} \
	dtrx__safe_free(entry_ptr->key); \
	dtrx__safe_free(entry_ptr->value); \
	dtrx__safe_free(entry_ptr->fi.free_list); \
} while(0)

#ifdef DTRX_DEBUG
	#define dtrx__print_hm_info(hm) do { \
		uint64_t i; \
		uint64_t counter = 0; \
		for(i = 0; i < hm->size; i++){ \
			if((hm->data_block[i].key) == NULL) continue; \
				printf("Entry struct at hash [%ld] with the key [%s]\n",i,hm->data_block[i].key); \
				counter++; \
		} \
		printf("Map size is [%ld], map counter is [%ld], entries with key [%ld], overall iterations [%ld]\n",hm->size,hm->count,counter,i); \
	} while(0)
#endif

typedef enum dtrx__key_allocation {
	DTRX__ALLOW = 1 << 0,
	DTRX__FORBID = 1 << 1
} dtrx__key_allocation;

typedef struct dtrx__va_args_info {
	uint64_t f_ptrc;
	void **free_list;
	dtrx_callback *cb_ptr;
	dtrx_free_info fi;
} dtrx__va_args_info;

struct dtrx__hm_entry {
	void *value;
	char *key;
	dtrx_free_info fi;
	struct dtrx__hm_entry *ne;
	struct dtrx__hm_entry *pe;
};


static const struct dtrx_free_info NULL__INFO = {0};
static const struct dtrx__hm_entry NULL__ENTRY = {0};

static dtrx_callback dtrx__add_value(dtrx_hashmap *, const char *, void *, dtrx_free_info);
static dtrx_callback dtrx__insert_value(dtrx__hm_entry *, uint64_t, const char *, dtrx__key_allocation, void *, dtrx_free_info);
static void dtrx__remap(dtrx_hashmap *);
static uint64_t dtrx__hash(const char *,uint64_t);
static dtrx__va_args_info dtrx__parse_format(char *,...);
		
static dtrx__va_args_info dtrx__parse_format(char *p,...){
	va_list argp;
	va_start(argp,p);
	dtrx__va_args_info info = {0};
	char *endp = NULL; 
	while(*p != 0){ 
		if(isspace(*p)) {p++; continue;}
		if(*p == '%'){ 
			p++; 
			if(*p == 'c' && *(p + 1) == 'b'){ 
				p += 2; 
				info.cb_ptr = va_arg(argp,dtrx_callback *);
			} 
			else if(*p == 'f' && *(p + 1) == 'i'){ 
				if(info.f_ptrc != 0) DTRX__PANIC("TRYING TO PASS dtrx_free_info STRUCT AND THE LIST OF POINTERS TO FREE AT THE SAME TIME"); 
				p += 2; 
				info.fi = va_arg(argp,dtrx_free_info);
			} 
			else if(*p == 'f' && *(p + 1) == 'p'){ 
				if(info.fi.count != 0) DTRX__PANIC("TRYING TO PASS A LIST OF POINTERS TO FREE AND A dtrx_free_info STRUCT AT THE SAME TIME");
				uint64_t val = strtoul(p + 2, &endp, 10); 
				p = endp; 
				void **vpp = NULL; 
				if(info.f_ptrc > 0){
					vpp = (void **)realloc(info.free_list,sizeof(void *) * (info.f_ptrc + val));
				}
				else{
					vpp = (void **)malloc(sizeof(void *) * val);
				}
				if(vpp == NULL) DTRX__PANIC("MEMORY ALLOCATION RETURNED NULL"); 
				info.f_ptrc += val;
				uint64_t i;
				for(i = info.f_ptrc - val; i < info.f_ptrc; i++){
					vpp[i] = va_arg(argp,void *);
				}
				info.free_list = vpp;
			} 
			else{ 
				DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "FORMATTING ERROR\n"););
			} 
		} 
		else p++; 
	} 
	va_end(argp);
	return info;
}
dtrx_hashmap *dtrx_new_hashmap(uint64_t size, float factor, uint64_t remap_multiplier){
	dtrx_hashmap *hm = (dtrx_hashmap *)malloc(sizeof(dtrx_hashmap));
	if(hm == NULL) DTRX__PANIC("MALLOC RETURNED NULL"); 
	hm->size = size;
	hm->count = 0;
	hm->remap_limit = size * factor;
	hm->remap_multiplier = remap_multiplier;
	hm->data_block = (dtrx__hm_entry *)calloc((size_t)size,sizeof(dtrx__hm_entry)); 
	if(hm->data_block == NULL) DTRX__PANIC("CALLOC RETURNED NULL"); 
	return hm;
}

static dtrx_callback dtrx__insert_value(dtrx__hm_entry *in_block, uint64_t map_size, const char *key, dtrx__key_allocation ka, void *dptr, dtrx_free_info fi){
	uint64_t hash = dtrx__hash(key,map_size);
		
	// Copy key
	char *key_ptr = NULL;
	if(ka & DTRX__ALLOW){
		size_t key_size = strlen(key) + 1;
		key_ptr = (char *)malloc(key_size);
		if(key_ptr == NULL) DTRX__PANIC("MALLOC RETURNED NULL"); 
		memcpy(key_ptr,(const void *)key,key_size);
	} 
	else{key_ptr = (char *)key;}

	// Copy all data
	dtrx__hm_entry *entry = &(in_block[hash]);
	if(entry->key != NULL){
		uint64_t i;
		for(i = hash; in_block[i].key != NULL; i = i % map_size) i++;
		uint64_t entry_hash = dtrx__hash(entry->key,map_size);
		if(entry_hash != hash){			
			entry->pe->ne = &(in_block[i]);
			if(entry->ne != NULL){
				entry->ne->pe = &(in_block[i]);
			}
			in_block[i] = *entry;
			entry->pe = NULL;
			DTRX_DEBUG_MODE(
				uint64_t test_hash = dtrx__hash(entry->key,map_size);
				printf("Entry added as replacement for chain element. Hash/key: [%ld]/[%s]. Replacing element with hash/key: [%ld]/[%s]\n",hash,key,test_hash,entry->key);
			);
		} 
		else{
			dtrx__hm_entry *l_entry = entry;
			while(1){
				if(strcmp(l_entry->key,key) == 0){
					DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "TRYING TO ADD AN ELEMENT WITH ALREADY EXISTING KEY: [%s]; ABORTING...\n",key););
					dtrx__safe_free(key_ptr);
					return DTRX_FAILURE;
				}
				if(l_entry->ne == NULL) break;
				l_entry = l_entry->ne;
			}
			entry = &(in_block[i]);
			l_entry->ne = entry;
			entry->pe = l_entry;
			DTRX_DEBUG_MODE(
				uint64_t p_hash = dtrx__hash(l_entry->key,map_size);
				printf("Entry added as an element of chain. P_hash/P_key => Hash/key/index: [%ld]/[%s] => [%ld]/[%s]/[%ld]\n",p_hash,l_entry->key,hash,key,i);
			);
		}
	} 
	else{
		DTRX_DEBUG_MODE(printf("Entry added regulary. Hash/key: [%ld]/[%s]\n",hash,key););
	}
	entry->fi = fi;
	entry->value = dptr;
	entry->key = key_ptr;
	entry->ne = NULL;
	return DTRX_SUCCESS;
}

static dtrx_callback dtrx__add_value(dtrx_hashmap *hm, const char *key, void *dptr, dtrx_free_info fi){
	if(hm->count >= (hm->remap_limit)){dtrx__remap(hm);}
	dtrx_callback icb = dtrx__insert_value(hm->data_block,hm->size,key,DTRX__ALLOW,dptr,fi);
	if(icb & DTRX_SUCCESS) hm->count++;
	return icb;
}


void *dtrx_get_value(dtrx_hashmap *hm, const char *key){
	uint64_t hash = dtrx__hash(key,hm->size);
	if(hm->data_block[hash].key == NULL){DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "TRYING TO ACCESS A NULL ELEMENT WITH HASH/KEY: [%ld]/[%s], INVALID KEY OR DELETED ENTRY\n",hash,key);); return NULL;}

	dtrx__hm_entry *entry = NULL;
	for (entry = &(hm->data_block[hash]); entry != NULL; entry = entry->ne){
		if(strcmp(key,entry->key) == 0){
			DTRX_DEBUG_MODE(printf("Returning an element with hash/key: [%ld]/[%s]\n",hash,entry->key););
			return entry->value;
		}
	}
	DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "TRYING TO ACCESS A NULL ELEMENT WITH HASH/KEY: [%ld]/[%s], INVALID KEY OR DELETED ENTRY\n",hash,key););
	return NULL;
}

dtrx_callback dtrx_remove_value(dtrx_hashmap *hm, const char *key){
	uint64_t hash = dtrx__hash(key,hm->size);
	if(hm->data_block[hash].key == NULL){
		DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "TRYING TO DELETE AN ELEMENT WITHOUT A KEY, PROBABLY ACCESSING PREVIOUSLY DELETED ENTRY OR THE KEY IS INVALID. KEY: [%s]\n",key););
		return DTRX_FAILURE;
	}
	
	uint64_t entry_hash = dtrx__hash(hm->data_block[hash].key,hm->size);
	if(entry_hash != hash){
		DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "HASH MISMATCH, AN ELEMENT WITH KEY [%s] WAS PROBABLY NEVER ADDED\n",key););
		return DTRX_FAILURE;
	}

	dtrx__hm_entry *entry;
	for(entry = &(hm->data_block[hash]); entry != NULL; entry = entry->ne){
		if(strcmp(entry->key,key) == 0) break;
	}
	if(entry == NULL){
		DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "COULDN'T FIND ENTRY IN THE CHAIN, THE ELEMENT WITH KEY [%s] IS NOT IN HM AND JUST HAPPENS TO HAVE A VALID HASH\n",key););
		return DTRX_FAILURE;
	}
	DTRX_DEBUG_MODE(
		size_t key_size = strlen(entry->key) + 1;
		char *deb_key = (char *)malloc(key_size);
		if(deb_key == NULL) DTRX__PANIC("MALLOC RETURNED NULL"); 
		memcpy(deb_key,(const void *)entry->key,key_size);
	);
	dtrx__free_entry(entry);
	// Its first entry in chain
	if(entry->pe == NULL){
		// It has following entries
		if(entry->ne != NULL){
			dtrx__hm_entry *tmp_ptr = entry->ne;
			DTRX_DEBUG_MODE(
				uint64_t deb_hash = dtrx__hash(tmp_ptr->key,hm->size);
				printf("Removing first entry in a chain. Swapping entry with hash/key [%ld]/[%s] with entry with hash/key [%ld]/[%s]\n",hash,deb_key,deb_hash,tmp_ptr->key);
			);
			*entry = *tmp_ptr;
			entry->pe = NULL;
			if(tmp_ptr->ne != NULL){
				tmp_ptr->ne->pe = entry;
			}
			entry = tmp_ptr;
		}
		// If there is no following entries it means its alone and should be left as is
		DTRX_DEBUG_MODE(printf("Removing first entry in a chain with entry with hash/key [%ld]/[%s]\n",hash,deb_key););
	}
	// Its 2nd or later entry
	else{
		if(entry->ne != NULL){
			DTRX_DEBUG_MODE(
				uint64_t deb_ne_hash = dtrx__hash(entry->ne->key,hm->size);
				uint64_t deb_pe_hash = dtrx__hash(entry->pe->key,hm->size);
				printf("Removing entry in a chain: [%ld]/[%s] <= [%ld]/[%s] => [%ld]/[%s]\n",deb_pe_hash,entry->pe->key,hash,deb_key,deb_ne_hash,entry->ne->key);
			);
			entry->ne->pe = entry->pe;
			entry->pe->ne = entry->ne;
		}
		else{
			DTRX_DEBUG_MODE(
				uint64_t deb_pe_hash = dtrx__hash(entry->pe->key,hm->size);
				printf("Removing last entry in a chain: [%ld]/[%s] <= [%ld]/[%s]\n",deb_pe_hash,entry->pe->key,hash,deb_key);
			);
			entry->pe->ne = NULL;
		}
	}
	DTRX_DEBUG_MODE(dtrx__safe_free(deb_key););
	*entry = NULL__ENTRY;
	hm->count--;
	return DTRX_SUCCESS;
}

void dtrx_delete_hashmap(dtrx_hashmap *hm){
	uint64_t i;
	dtrx__hm_entry *entry;
	for(i = 0; i < hm->size; i++){
		entry = &hm->data_block[i];
		if(entry->key != NULL){
			dtrx__free_entry(entry);
		}
	}
	dtrx__safe_free(hm->data_block);
	dtrx__safe_free(hm);
}

static void dtrx__remap(dtrx_hashmap *hm){
	DTRX_DEBUG_MODE(printf(DTRX__CYAN_COLOR "==========" DTRX__RESET_COLOR "\n THE MAP IS GETTING TOO SMALL (%ld ELEMENTS AT %ld CAPACITY), REMAPPING...\n" DTRX__CYAN_COLOR "==========" DTRX__RESET_COLOR"\n",hm->count,hm->size););
	uint64_t new_size = hm->size * hm->remap_multiplier;
	dtrx__hm_entry *data_block = (dtrx__hm_entry *)calloc((size_t)new_size, sizeof(dtrx__hm_entry));
	if(data_block == NULL) DTRX__PANIC("CALLOC RETURNED NULL");
	dtrx__hm_entry *tmp_ptr = hm->data_block;
	dtrx__hm_entry *entry = NULL;
	uint64_t i;
	for(i = 0; i < hm->size; i++){ 
		entry = &(tmp_ptr[i]);
		entry->pe = NULL;
		entry->ne = NULL;

		if(entry->key != NULL){
			dtrx__insert_value(data_block,new_size,entry->key,DTRX__FORBID,entry->value,entry->fi);
		}
	}
	dtrx__safe_free(hm->data_block);

	hm->data_block = data_block;
	hm->size = new_size;
	hm->remap_limit *= hm->remap_multiplier;
	DTRX_DEBUG_MODE(dtrx__print_hm_info(hm););
}

static uint64_t dtrx__hash(const  char *string, uint64_t size){
	/*   djb2 algorithm   */
	const unsigned char *ustring = (const unsigned char *)string;
	uint64_t hash = 5381;
	uint64_t c;
	while((c = *ustring++)){
		hash = ((hash << 5) + hash) + c;
	}
	return hash % size;
}
	
#endif // DTRX_HM_IMPLEMENTATION_DONE
#endif // DTRX_HM_IMPLEMENTATION
#ifdef __cplusplus
}
#endif
#endif // DTRX_HASHMAP_H
