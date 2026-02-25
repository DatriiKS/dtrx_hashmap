![Banner](https://github.com/user-attachments/assets/e1b41715-e759-42c3-9f97-a3bef2b7e5b9)
# About
**DTRX Hashmap** is a C++ compatible, single header-file library written in C99 that implements a generic, high-level-like hashmap. It was built for use in personal projects, that prioritize simplicity and ease of use over raw performance.

It can also serve as a learning experience for people that are new to C programming, as it utilizes a lot of common tricks and features of the language.

| Table of contents|
|-------------------|
|[Quick start](https://github.com/DatriiKS/dtrx_hashmap/tree/main?tab=readme-ov-file#quick-start)|
|[In-detail tutorial](https://github.com/DatriiKS/dtrx_hashmap/tree/main?tab=readme-ov-file#in-detail-tutorial)|
|[0: Repeatedly used definitions](https://github.com/DatriiKS/dtrx_hashmap/tree/main?tab=readme-ov-file#0-repeatedly-used-definitions)|
|[1: Hashmap creation](https://github.com/DatriiKS/dtrx_hashmap/tree/main?tab=readme-ov-file#1-hashmap-creation)|
|[2: Data insertion macros](https://github.com/DatriiKS/dtrx_hashmap/tree/main?tab=readme-ov-file#2-data-insertion-macros)|
|[3: Parsing a formatting string](https://github.com/DatriiKS/dtrx_hashmap/tree/main?tab=readme-ov-file#3-parsing-a-formatting-string)|
|[4: Adding an entry to the hashmap](https://github.com/DatriiKS/dtrx_hashmap/tree/main?tab=readme-ov-file#4-adding-an-entry-to-the-hashmap)|
|[5: How the actual insertion happens](https://github.com/DatriiKS/dtrx_hashmap/tree/main?tab=readme-ov-file#5-how-the-actual-insertion-happens)|
|[6: Remapping the hashmap](https://github.com/DatriiKS/dtrx_hashmap/tree/main?tab=readme-ov-file#6-remapping-the-hashmap)|
|[7: Retrieving a value from the hashmap](https://github.com/DatriiKS/dtrx_hashmap/tree/main?tab=readme-ov-file#7-retrieving-a-value-from-the-hashmap)|
|[8: Removing an entry from the hashmap](https://github.com/DatriiKS/dtrx_hashmap/tree/main?tab=readme-ov-file#8-removing-an-entry-from-the-hashmap)|
|[9: Deleting the hashmap](https://github.com/DatriiKS/dtrx_hashmap/tree/main?tab=readme-ov-file#9-deleting-the-hashmap)|
|[10: File structure](https://github.com/DatriiKS/dtrx_hashmap/tree/main?tab=readme-ov-file#10-file-structure)|

# Quick start:
* Get the **dtrx_hashmap.h** file and include it in your program. You can clone the whole repo, which includes 2 test files if you want to visualize how the hashmap works.
* **To simply use** the library no specific compilation flags are required. **To see the visualization** of each step, compile the program with the `-DDTRX_DEBUG` flag.
* Define a `#DTRX_HM_IMPLEMENTATION` implementation macro in the main file of your program.
* To **create a hashmap** use the `dtrx_new_hashmap(<size>,<remap_factor>)` function.
* To **insert a pointer** use `dtrx_hm_rinsert(<hm *>,const char *<key>,<val *>,...<format_str>,<data according to format_str>)` to perform a (**reference**)insert. 
This call will isnert a pointer and `NULL` it, to avoid any change of the data inside on accident by accessing that poiner. If for some reason this behaviour is desired - make a copy of the pointer beforehand.
* To **insert a value** use `dtrx_hm_vinsert(<hm *>,const char *<key>,<val>,<type>,...<format_str>,data according to format_str>);` to perform a (**value**)insert. 
This call will allocate memory for specified type and perform a shallow copy of the data passed. Mostly used for simple types to avoid manual memory allocations on each insertion.
* **The `<format_str>`** mentioned above is a string of characters akin to `printf()`'s that is used to parse variadic arguments that come after it. It has following valid keys: `"%cb"`, `"%fi"`, `"%fp<number>"`. Where:
    * `cb` is an address of a `dtrx_callback` variable that will contain either `DTRX_SUCCESS` or `DTRX_FAILURE` depending on the outcome of the insertion.
    * `fi` is an instance of `dtrx_free_info` struct, that looks like this:
        ```
        struct dtrx_free_info {
            void **free_list;
            uint64_t count;
        };
        ```
        `free_list` is a pointer to poiters that you want to be freed when the hashmap entry is deleted and `count` is the number of said pointers. 
        
        This key is mostly usefull for handling big structures that contain a lot of pointers to dynamically allocated memory. A custom function for a specific type can be created, wich will return `dtrx_free_info` struct and pass it instead of passing each pointer manually.
    * `fp<number>` is a sign to parser, that there will be a `<number>` of pointers passed as variadic arguments, and parser will use them to construct `dtrx_free_info` mentioned above. Usefull for smaller structs, that don't contain many pointers and where each of them can be passed separately as an argument.
    
        For this case it is better if the allocated memory was assigned directly to the struct's field, like in the `test.c` example file, so that the pointer can be passed by dereferencing struct's member; by doing so no access points to the allocated data will be left in the caller function once the struct pointer will be `NULL`'ed. 
    
        **Note:**
        * Arguments after the `<format_str>` should go in the order they were specified. 
        * `fp<number>` key expects a number right after, without any whitespace.
        * `fp<number>` can be split into multiple keys, for example: `"%fp2 %cb %fp1"`. The arguments should be split accordingly.

* To **get a value** from the hashmap use `dtrx_get_value(<hm *>,const char *<key>)`. It returns a void pointer, so the result will need to be casted to the appropriate type.
* To **remove value** from the hashmap use `dtrx_remove_value(<hm *>, const char *<key>)`. This will remove an entry from the hashmap and free all the pointers that you've passed on insertion. This function returns a `dtrx_callback` which can be either `DTRX_SUCCESS` or `DTRX_FAILURE` depending on how the outcome of the operation.
* To **delete hashmap** use `dtrx_delete_hashmap(<hm *>)`. It deallocates all the hashmap's entries, their associated data and the hashmap itself.


# In-detail tutorial
The following section will explain how and why this library works as it does. It will be mostly usefull for the people who are just starting to learn C and/or want to know how "generics" can be implemented in this language. This part will provide a deep, almost line-by-line explanation. To simply set up the library and use it, refer to the [Quick start](https://markdownlivepreview.com/) section. 

## **0: Repeatedly used definitions**
Going through this tutorial, multiple different definitions will be repeatedly used, to avoid cluttering the text, they will be listed in this section.
    
1. `DTRX_DEBUG_MODE` macro. Defined like this:
```
 1 #ifdef DTRX_DEBUG
 2         #define DTRX_DEBUG_MODE(code) code
 3 #else
 4         #define DTRX_DEBUG_MODE(code)
 5 #endif
```
It executes the code passed to it if `DTRX_ASSERT` was defined, usually during compilation with the use of the `-DDTRX_ASSERT` compile flag. And if it wasn't, the macro will expand into nothing during the preprocessor step, allowing for zero overhead in that case.

2. `DTRX_PANIC` macro. Defined like this:
```
 1 #define DTRX__PANIC(msg) do { \
 2         fprintf(stderr, DTRX__RED_COLOR "ERROR " DTRX__RESET_COLOR "IN FUNCTION \"%s\" IN %s:%d! " msg "\n",__func__,__FILE__,__LINE__); \
 3         exit(EXIT_FAILURE); \
 4 } while(0) 
```
This macro is used to exit the program if a critical logic error occurred; it prints an error message.

3. Both of the macros above print some information when used in code. To color the output, they use a group of `DTRX__*_COLOR` macros:
```
 1 #define DTRX__YELLOW_COLOR "\033[33m"
 2 #define DTRX__RED_COLOR "\033[31m"
 3 #define DTRX__CYAN_COLOR "\033[36m"
 4 #define DTRX__RESET_COLOR "\033[0m"
```
Each define is associated with an [ANSI escape code](https://en.wikipedia.org/wiki/ANSI_escape_code) responsible for coloring the text output.

4. A `dtrx__safe_free` macro:
```
 1 #define dtrx__safe_free(ptr) do { \
 2         if(ptr != NULL) {free(ptr);ptr = NULL;} \
 3 } while(0)
```
A classic implementation of a `safe_free` macro expansion. It allows avoiding invalid memory access and dangling pointers; it frees the memory.

5. The `dtrx_callback` [enum](https://en.wikipedia.org/wiki/Enumerated_type) is composed of [bit flags](https://en.wikipedia.org/wiki/Flag_(programming)) where each field represents a power of 2 value and is defined using a left-shift [bitwise operator](https://en.wikipedia.org/wiki/Bitwise_operation). It is declared like this:
```
 1 typedef enum dtrx_callback {
 2         DTRX_NONE = 0,
 3         DTRX_SUCCESS = 1 << 0,
 4         DTRX_FAILURE = 1 << 1
 5 } dtrx_callback;
```
It is not strictly necessary to use bit flags; rather, it's just a common idiom in C that allows for efficient checks of multiple return states using bitwise operators, so feel free to add your own bit flags to this declaration if you need additional callback information.

* **Note:** Some of the macro expansions explained in this tutorial (including the ones mentioned above) follow this structure:
```
 1 #define <macro_name> () do {
    <line1>;
    <line2>;
    etc...
 } while (0)
```
This is the so-called "`do-while(0)` macro trick" or "`do-while` swallow". It allows for safe multi-line macro expansion in any context, including the expansion within `if-else` statements without breaking the syntax. It makes each call behave in a more predictable, function-like manner. 

For a more detailed explanation, refer to [this Stack Overflow question](https://stackoverflow.com/questions/923822/whats-the-use-of-do-while0-when-we-define-a-macro).

## **1: Hashmap creation**
This implementation uses void pointers to store data. This approach allows a single hashmap to store any data type without any manual intervention each time a new type needs to be stored, unlike more performance-oriented libraries, which often use macro expansions to create separate instances of hashmaps for a specified type.

To create a hashmap a `dtrx_new_hashmap()` function is used:
```
 1 dtrx_hashmap *dtrx_new_hashmap(uint64_t size, float factor, uint64_t remap_multiplier){
 2         dtrx_hashmap *hm = (dtrx_hashmap *)malloc(sizeof(dtrx_hashmap));
 3         if(hm == NULL) DTRX__PANIC("MALLOC RETURNED NULL");
 4         hm->size = size;
 5         hm->count = 0;
 6         hm->remap_limit = size * factor;
 7         hm->remap_multiplier = remap_multiplier;
 8         hm->data_block = (dtrx__hm_entry *)calloc((size_t)size,sizeof(dtrx__hm_entry));
 9         if(hm->data_block == NULL) DTRX__PANIC("CALLOC RETURNED NULL");
10         return hm;
11 }
```
This function is pretty straightforward: it fills the struct with the given data and allocates memory "on the heap" for the hashmap itself, so it can live beyond the function's scope. It also checks each pointer returned by a memory allocation function, and if it's `NULL`, it exits the program by calling the `DTRX__PANIC` macro.

The only part worth noting here is the use of `calloc` when assigning to `hm->data_block` **on line 8**, as it returns memory filled with zeroes, which will be important later.

The function returns a pointer to the `dtrx_hashmap` struct, which is defined like this:
```
 1 typedef struct dtrx_hashmap {
 2         uint64_t size;
 3         uint64_t count;
 4         uint64_t remap_multiplier;
 5         uint64_t remap_limit;
 6         dtrx__hm_entry *data_block;
 7 } dtrx_hashmap;
```

In essence, this code functions as the equivalent of a [constructor](https://en.wikipedia.org/wiki/Constructor_(object-oriented_programming)) in [OOP](https://en.wikipedia.org/wiki/Object-oriented_programming) approaches.

## **2: Data insertion macros** 
To insert a value into the hashmap, two macro expansions are used: 

`dtrx_hm_rinsert` to perform a (**reference**)insert:
```
 1 #define dtrx_hm_rinsert(hm,key,ptr,...) dtrx_hm_rinsert_step_two(hm,key,ptr,##__VA_ARGS__,"\0")
 2 #define dtrx_hm_rinsert_step_two(hm,key,ptr,fmt,...) { \
 3         do{ \
 4                 void *_macro_tmp_ptr = ptr; \
 5                 dtrx_free_info _macro_fi = NULL__INFO; \
 6                 dtrx__va_args_info _macro_info = dtrx__parse_format((char *)fmt,##__VA_ARGS__); \
 7                 if(_macro_info.f_ptrc > 0) {_macro_fi.free_list = _macro_info.free_list; _macro_fi.count = _macro_info.f_ptrc;} \
 8                 if(_macro_info.fi.count > 0) _macro_fi = _macro_info.fi; \
 9                 dtrx_callback _macro_callback = dtrx__add_value(hm,key,_macro_tmp_ptr,_macro_fi); \
10                 if(_macro_info.cb_ptr != NULL) *_macro_info.cb_ptr = _macro_callback;\
11                 if(_macro_callback & DTRX_FAILURE){ \
12                         dtrx__safe_free(_macro_fi.free_list); \
13                         break; \
14                 } \
15                 ptr = NULL; \
16         }while(0); \
17 }
```
This call will insert a pointer and `NULL`ify it, to avoid any accidental change of the data inside by accessing that poiner. If, for some reason, this behaviour is desired, make a copy of the pointer beforehand. 

`dtrx_hm_vinsert` to perform a (**value**)insert:
```
 1 #define dtrx_hm_vinsert(hm,key,val,type,...) dtrx_hm_vinsert_step_two(hm,key,val,type,##__VA_ARGS__,"\0")
 2 #define dtrx_hm_vinsert_step_two(hm,key,val,type,fmt,...) { \
 3         type *_macro_ptr = (type *)malloc(sizeof(type)); \
 4         if(_macro_ptr == NULL) DTRX__PANIC("MALLOC RETURNED NULL"); \
 5         *_macro_ptr = val; \
 6         dtrx_free_info _macro_fi = NULL__INFO; \
 7         dtrx__va_args_info _macro_info = dtrx__parse_format((char *)fmt,##__VA_ARGS__); \
 8         if(_macro_info.f_ptrc > 0) {_macro_fi.free_list = _macro_info.free_list; _macro_fi.count = _macro_info.f_ptrc;} \
 9         if(_macro_info.fi.count > 0) _macro_fi = _macro_info.fi; \
10         dtrx_callback _macro_callback = dtrx__add_value(hm,key,(void *)_macro_ptr,_macro_fi); \
11         if(_macro_info.cb_ptr != NULL) *_macro_info.cb_ptr = _macro_callback;\
12         if(_macro_callback & DTRX_FAILURE){ \
13                 dtrx__safe_free(_macro_fi.free_list); \
14                 dtrx__safe_free(_macro_ptr); \
15         } \
16 }
 ```
This call will allocate memory for the specified type and perform a shallow copy of the data passed. It is mostly used for simple types to avoid manual memory allocation on each insertion.

They both work in two steps because they both are [variadic macros](https://en.cppreference.com/w/c/variadic.html), meaning they have variadic arguments in addition to the required ones.

To pass variadic arguments from the first(**outer**) macro to the second(**inner**) one, a `__VA_ARGS__`  substitution identifier is used. It takes whatever text is inside the variadic arguments of the first macro and injects it directly into the second macro call. The `##` in front of it is just a compiler extension that removes a trailing comma if there were no variadic arguments passed, rendering `__VA_ARGS__` empty. This compiler extension is not a part of the **C standart**, but is supported by virtually every major compiler.

Now, two problems have to be solved: the formatting string should be accessible from the **inner** macro to later pass variadic arguments that come after the formatting string into a `dtrx__parse_format` variadic function, which needs at least one required argument before `...`. Also, the case where no variadic arguments were passed should be handled as well.

To solve these problems, a trick with text substitution is used: the **inner** macro is called with fewer explicit arguments than its definition requires, but the `__VA_ARGS__` and `"\0"` are also passed. After the preprocessing step, `__VA_ARGS__` will expand into a list of actual arguments passed, which will result in either its first argument occupying the position of the `fmt` argument in the **inner** macro's definition, or shifting an empty `"\0"` string into that position.

**For example:**
*   This call **WITH** additional arguments from the [test.c](HERE) file:
    * `dtrx_hm_rinsert(hm,rkey,cdp,"%cb %fp2 %fp1",&rcb,cdp->strval,cdp->fval,cdp->ip)` will expand into a `dtrx_hm_rinsert_step_two(hm,rkey,cdp,"%cb %fp2 %fp1",&rcb,cdp->strval,cdp->fval,cdp->ip,"\0")` call, placing the formatting string exactly in the position of the `fmt` argument in `dtrx_hm_rinsert_step_two`'s definition.
        ![image macro 1](https://github.com/user-attachments/assets/0367d992-44a6-4c89-aeb3-d6a401ee6293)


*   This call **WITHOUT** additional arguments from the [test_h.h](HERE) file:
    * `dtrx_hm_vinsert(hm,"ikey_foo",1,int)` will expand into a `dtrx_hm_vinsert_step_two(hm,"ikey_foo",1,int,"\0")` call, placing an empty `"\0"` string in the place of the `fmt` argument in `dtrx_hm_vinsert_step_two`'s definition. In this case, `__VA_ARGS__` results in an empty expansion because no additional arguments were provided beyond the required ones. 
        ![image macro 2](https://github.com/user-attachments/assets/50d4305e-56e9-4919-80db-62a21ae79a1a)

Every variable defined within the scope of those macro expansions starts with the `_macro_` prefix. This is done because these macros will expand in an arbitrary contexts, which creates a risk of name collisions. If the same name is defined both inside and outside of the macro, the inner definition will shadow the outer one due to the nature of text substitution.

**Now for what each of them does:**
* `dtrx_hm_rinsert_step_two` puts all its logic into the body of a `do{...}while()` loop with a single iteration. This is a widely used trick needed to be able to abort execution mid-way because, unlike a function, a macro expansion cannot `return`. Inside the `do{...}while()` body the macro goes through the following steps:
    * **Step 1:** Saves the address of the data into a `void *`, which allows it to store any information.
    * **Step 2:** Initializes a `dtrx_free_info` struct with a `NULL__INFO`, which is a statically defined constant representing an empty state, it is defined like this: `static const struct dtrx_free_info NULL__INFO = {0};`.
    * **Step 3:** Calls the `dtrx__pase_format` function, passing it the extracted formatting string and other variadic arguments. This function returns a `dtrx__va_args_info` struct, which is declared like this:
        ```
        1 typedef struct dtrx__va_args_info {
        2         uint64_t f_ptrc;
        3         void **free_list;
        4         dtrx_callback *cb_ptr;
        5         dtrx_free_info fi;
        6 } dtrx__va_args_info;
        ```
    * **Step 4:** Based on the returned `dtrx__va_args_info` struct it either fills each field of the `_macro_fi` variable that was defined in **Step 2**, or simply copies the whole struct into it.
    * **Step 5:** The macro tries to add the value into the hashmap using the `dtrx__add_value` function and saves the callback that it returns into its internal variable of the `dtrx_callback` type.
    * **Step 6:** Writes the returned callback into an external `dtrx_callback` variable if the address of one was passed as one of the variadic arguments.
    * **Step 7:** Then, it checks for the actual result, written into inner `_macro_callback` variable. And if the `DTRX_FAILURE` flag is set it safely frees the `void **free_list` within the `_macro_info` struct by calling the `dtrx__safe_free` macro.
    
        It also breaks out of the `do{...}while()` loop. This step is needed to avoid memory leaks and `NULL`ifying the original pointer on **Step 8** if the insertion failed.
    * **Step 8:** It `NULL`ifies the original pointer, blocking access to the data from the caller function through that pointer to avoid accidental data changes. 

* `dtrx_hm_vinsert_step_two` acts similarly, but it does not use the `do{...}while()` loop because, under any condition, it executes all the way to the end. It performs the following steps:
    * **Step 1:** Creates a variable of the cpecified type using text substitution and allocates enough memory for that type.
    * **Step 2:** Initializes a `dtrx_free_info` struct with `NULL__INFO`, which is a statically defined constant representing an empty state, it is defined like this: `static const struct dtrx_free_info NULL__INFO = {0};`.
    * **Step 3:** Calls the `dtrx__parse_format` function, passing it the extracted formatting string and other variadic arguments. This function returns a `dtrx__va_args_info` struct, which is declared like this:
        ```
        1 typedef struct dtrx__va_args_info {
        2         uint64_t f_ptrc;
        3         void **free_list;
        4         dtrx_callback *cb_ptr;
        5         dtrx_free_info fi;
        6 } dtrx__va_args_info;
        ```
    * **Step 4:** Based on the returned `dtrx__va_args_info` struct it either fills each field of the `_macro_fi` variable that was defined on **Step 2**, or just copies the whole struct into it.
    * **Step 5:** Tries to add the value into the hashmap using the `dtrx__add_value` function and saves the callback that it returns into its internal variable of `dtrx_callback` type. 
    * **Step 6:** Writes the returned callback into an external `dtrx_callback` variable if the address of one was passed as one of the variadic arguments.
    * **Step 7:** Then, checks for the actual result, written into the internal `_macro_callback` variable. And if the `DTRX_FAILURE` flag is set, calls the `dtrx__safe_free` macro and safely frees the `void **free_list` within the `_macro_info` struct and the pointer created in **Step 1**.

## **3: Parsing a formatting string**
This section explains how the parsing happens inside the `dtrx__parse_format` function mentioned in the previous part. It's actually pretty simple; here's what the code looks like:
```
 1 static dtrx__va_args_info dtrx__parse_format(char *p,...){
 2         va_list argp;
 3         va_start(argp,p);
 4         dtrx__va_args_info info = {0};
 5         char *endp = NULL;
 6         while(*p != 0){
 7                 if(isspace(*p)) {p++; continue;}
 8                 if(*p == '%'){
 9                         p++;
10                         if(*p == 'c' && *(p + 1) == 'b'){
11                                 p += 2;
12                                 info.cb_ptr = va_arg(argp,dtrx_callback *);
13                         }
14                         else if(*p == 'f' && *(p + 1) == 'i'){
15                                 if(info.f_ptrc != 0) DTRX__PANIC("TRYING TO PASS dtrx_free_info STRUCT AND THE LIST OF POINTERS TO FREE AT THE SAME TIME");
16                                 p += 2;
17                                 info.fi = va_arg(argp,dtrx_free_info);
18                         }
19                         else if(*p == 'f' && *(p + 1) == 'p'){
20                                 if(info.fi.count != 0) DTRX__PANIC("TRYING TO PASS A LIST OF POINTERS TO FREE AND A dtrx_free_info STRUCT AT THE SAME TIME");
21                                 uint64_t val = strtoul(p + 2, &endp, 10);
22                                 p = endp;
23                                 void **vpp = NULL;
24                                 if(info.f_ptrc > 0){
25                                         vpp = (void **)realloc(info.free_list,sizeof(void *) * (info.f_ptrc + val));
26                                 }
27                                 else{
28                                         vpp = (void **)malloc(sizeof(void *) * val);
29                                 }
30                                 if(vpp == NULL) DTRX__PANIC("MEMORY ALLOCATION RETURNED NULL");
31                                 info.f_ptrc += val;
32                                 uint64_t i;
33                                 for(i = info.f_ptrc - val; i < info.f_ptrc; i++){
34                                         vpp[i] = va_arg(argp,void *);
35                                 }
36                                 info.free_list = vpp;
37                         }
38                         else{
39                                 DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "FORMATTING ERROR\n"););
40                         }
41                 }
42                 else p++;
43         }
44         va_end(argp);
45         return info;
46 }
```
The function takes a `char *`,expecting the formatting string, along with some other variadic arguments. The important thing here is that, unlike macro expansions, functions can actually access those variadic arguments. To do so, it utilizes a set of macros defined in the [<stdarg.h>](https://man7.org/linux/man-pages/man3/stdarg.3.html) header file. 

Refer to the link above for an in-depth explanation, but to put it simply: a variable of the `va_list` type acts as a tracker for variadic arguments. `va_start()` initializes the tracker with the first variadic argument. `va_arg()` fetches a variadic argument of the specified type and advances the tracker forward by one argument. `va_end()` matches the `va_start()` call and is required to safely clean up the state.

**This function consists of the following steps:**
* **Step 1:** Initializes the variadic arguments tracker with `va_start()` as explained above.
* **Step 2:** Creates an emtpy `dtrx__va_args_info` struct that will be filled and returned by this function.

>  ```
>    1 typedef struct dtrx__va_args_info {
>    2         uint64_t f_ptrc;
>    3         void **free_list;
>    4         dtrx_callback *cb_ptr;
>    5         dtrx_free_info fi;
>    6 } dtrx__va_args_info;
>   ```


* **Step 3:** Creates an empty `char *` that will be used later by the [strtoul()](https://en.cppreference.com/w/c/string/byte/strtoul) function.
* **Step 4:** Enters a `while(){...}` loop, where the actual parsing begins. It skips all the whitespace using the [`isspace()`](https://en.cppreference.com/w/c/string/byte/isspace) function, that is defined in the `<ctype.h>` header file, in search of the **%** character that symbolizes the beginning of the key.
* **Step 5:** Once the percent sign is found, checks the following symbols for valid key combinations. If it finds them, it executes the following steps:
    * For the `%cb` key, retrieves a `dtrx_callback *` and assigns it to the corresponging field of the `dtrx__va_args_info` struct, created in **Step 2**; moves the formatting string pointer forward by two characters.
    * For the `%fi` key, ensures that no pointers requiring manual deallocation were passed. The function operates exclusively on either a list of pointers or a `dtrx_free_info` struct, so if pointers are passed alongside the `dtrx_free_info` struct, the `DTRX__PANIC` macro is called and the program terminates. 
        If the arguments are passed properly, it retrieves a `dtrx_free_info` struct and assigns it to the corresponging field of the `dtrx__va_args_info` struct, created in **Step 2**; moves the formatting string pointer forward by two characters
    * For the `%fp<number>` key, the function expects the opposite of the condition used for the `%fi` key - it calls the `DTRX__PANIC` macro if the `dtrx_free_info` struct was previously passed to comply with the **"operates exclusively on either a list of pointers or a `dtrx_free_info struct`"** rule.
    
        Then, uses a [`strtoul()`](https://en.cppreference.com/w/c/string/byte/strtoul) function, defined it in the [`<stdlib.h>`](https://en.cppreference.com/w/c/header/stdlib.html) header file to retrieve the number of pointers from the `%fp<number>` key. The `strtoul()` function also writes the address of the first `char` following the number to a variable, in this case - the one mentioned in **Step 3**. The function also assigns this address to the pointer that is used to traverse through the formatting string.

        This key is the only one that branches logic if passed multiple times. The parser checks the `info.f_ptrc` counter; if it's zero, meaning that `%fp<number>` key is the first one, the function allocates memory for a pointer that will store the pointers that were passed to the function as arguments. In the case where `info.f_ptrc` is above zero, it `realloc`ates memory to preserve previously assigned pointers and gain space for the new ones.

        Once the memory is allocated, the parser increments the `info.f_ptrc` counter and enters a `for(){...}` loop, filling the allocated memory with the pointers that were passed to the function as arguments. Then it assigns the pointer-to-pointers to the `.free_list` field of the `dtrx__va_args_info` struct.

        Here's how the logic for this key looks like:
        #   Image here

    * If the parser finds the `%` sign, but no valid combination of characters following it, it will call the `DTRX_DEBUG_MODE` macro if the `-DDTRX_DEBUG` compile flag was specified. If this compile flag was not specified, lines **37-39** will be effectively removed by the compiler, allowing for no overhead in that case.
    * If the character is anything other than `%`, the function simply increments the counter and continues its `while(){...}` loop.
* **Step 6:** Finally, safely cleans up the state using `va_end()` and returns the resulting `dtrx__va_args_info` struct.

## **4: Adding an entry to the hashmap**
After the parsing is complete, the `dtrx__add_value` function is called in both insertion macros. Here's how it looks:
```
 1 static dtrx_callback dtrx__add_value(dtrx_hashmap *hm, const char *key, void *dptr, dtrx_free_info fi){
 2         if(hm->count >= (hm->remap_limit)){dtrx__remap(hm);}
 3         dtrx_callback icb = dtrx__insert_value(hm->data_block,hm->size,key,DTRX__ALLOW,dptr,fi);
 4         if(icb & DTRX_SUCCESS) hm->count++;
 5         return icb;
 6 }
 ```
 The logic is straightforward; it:
* **Step 1:** Checks if the current count of hashmap entries is equal to or above the set limit; if it is, calls the `dtrx__remap()` function to extend the hashmap's storage.
* **Step 2:** Inserts an entry into the hashmap using `dtrx__insert_value()`.
* **Step 3:** If the insertion was successful, increments the hashmap's inner entry counter.
* **Step 4:** Returns the insertion callback.
    
## **5: How the actual insertion happens**
To actually insert a value and create a hashmap entry, the `dtrx__insert_value()` function is used. Here's how it is defined:
``` 
 1 static dtrx_callback dtrx__insert_value(dtrx__hm_entry *in_block, uint64_t map_size, const char *key, dtrx__key_allocation ka, void *dptr, dtrx_free_info fi){
 2         uint64_t hash = dtrx__hash(key,map_size);
 3 
 4         // Copy key
 5         char *key_ptr = NULL;
 6         if(ka & DTRX__ALLOW){
 7                 size_t key_size = strlen(key) + 1;
 8                 key_ptr = (char *)malloc(key_size);
 9                 if(key_ptr == NULL) DTRX__PANIC("MALLOC RETURNED NULL");
10                 memcpy(key_ptr,(const void *)key,key_size);
11         }
12         else{key_ptr = (char *)key;}
13 
14         // Copy all data
15         dtrx__hm_entry *entry = &(in_block[hash]);
16         if(entry->key != NULL){
17                 uint64_t i;
18                 for(i = hash; in_block[i].key != NULL; i = i % map_size) i++;
19                 uint64_t entry_hash = dtrx__hash(entry->key,map_size);
20                 if(entry_hash != hash){
21                         entry->pe->ne = &(in_block[i]);
22                         if(entry->ne != NULL){
23                                 entry->ne->pe = &(in_block[i]);
24                         }
25                         in_block[i] = *entry;
26                         entry->pe = NULL;
27                         DTRX_DEBUG_MODE(
28                                 uint64_t test_hash = dtrx__hash(entry->key,map_size);
29                                 printf("Entry added as replacement for chain element. Hash/key: [%ld]/[%s]. Replacing element with hash/key: [%ld]/[%s]\n",hash,key,test_hash,entry->key);
30                         );
31                 }
32                 else{
33                         dtrx__hm_entry *l_entry = entry;
34                         while(1){
35                                 if(strcmp(l_entry->key,key) == 0){
36                                         DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "TRYING TO ADD AN ELEMENT WITH ALREADY EXISTING KEY: [%s]; ABORTING...\n",key););
37                                         dtrx__safe_free(key_ptr);
38                                         return DTRX_FAILURE;
39                                 }
40                                 if(l_entry->ne == NULL) break;
41                                 l_entry = l_entry->ne;
42                         }
43                         entry = &(in_block[i]);
44                         l_entry->ne = entry;
45                         entry->pe = l_entry;
46                         DTRX_DEBUG_MODE(
47                                 uint64_t p_hash = dtrx__hash(l_entry->key,map_size);
48                                 printf("Entry added as an element of chain. P_hash/P_key => Hash/key/index: [%ld]/[%s] => [%ld]/[%s]/[%ld]\n",p_hash,l_entry->key,hash,key,i);
49                         );
50                 }
51         }
52         else{
53                 DTRX_DEBUG_MODE(printf("Entry added regulary. Hash/key: [%ld]/[%s]\n",hash,key););
54         }
55         entry->fi = fi;
56         entry->value = dptr;
57         entry->key = key_ptr;
58         entry->ne = NULL;
59         return DTRX_SUCCESS;
60 }
```    
The function is meant to be versatile for use inside both the `dtrx__add_value()` and `dtrx__remap()` functions, so the argument list might look abstract at first glance, but this function is also pretty straightforward. Here's how it works:
* **Step 1:** Calls the `dtrx__hash()` function, passing it a key and the map size to generate a hash. The returned hash serves as an index in the memory block that the insertion function receives as its first argument.

The hash function itself uses the [djb2 algorithm](http://www.cse.yorku.ca/~oz/hash.html), which only supports strings as keys. If you want to change this behavior, you'll have to change the hashing function and the key storage logic. The `dtrx__hash()` function itself is pretty small; here's its full definition:
```
 1 static uint64_t dtrx__hash(const  char *string, uint64_t size){
 2         /*   djb2 algorithm   */
 3         const unsigned char *ustring = (const unsigned char *)string;
 4         uint64_t hash = 5381;
 5         uint64_t c;
 6         while((c = *ustring++)){
 7                 hash = ((hash << 5) + hash) + c;
 8         }
 9         return hash % size;
10 }
```
* **Step 2:** Then, the function handles key assignment. Using a bitwise check against the `DTRX__ALLOW` mask, it either: gets the length of the key string by calling [strlen()](https://en.cppreference.com/w/c/string/byte/strlen.html), [malloc](https://en.cppreference.com/w/c/memory/malloc)ates the memory for it, and copies the key into that memory using [memcpy()](https://en.cppreference.com/w/c/string/byte/memcpy.html); or, just copies the `char *` as is. The `if(){...}` body is executed to store the key independently from the caller's key, while the `else{...}`s body is used when this function is called from the `dtrx__remap()` function to avoid allocating and cleaning memory of already encapsulated data.

The `dtrx__key_allocation` enum, which is used for the bitwise check, is defined like this:
```
 1 typedef enum dtrx__key_allocation {
 2         DTRX__ALLOW = 1 << 0,
 3         DTRX__FORBID = 1 << 1
 4 } dtrx__key_allocation;
 ```
* **Step 3:** The core logic of this function is implemented here, but before going further, a couple of concepts have to be explained. 

Most hashmap implementations store data in an array of so-called **buckets**, which usually are structs containing a value, a key, and a pointer to the next bucket. The pointer to the next bucket is needed to handle [hash collision](https://en.wikipedia.org/wiki/Hash_collision)s - a situation where the hash function returns the same index for two different keys, which happens in all hashing algorithms. There are different approaches to hash collision resolution; the two primary strategies are:
    
* **Separate chaining** using linked lists or trees: In this approach, memory is dynamically allocated to store colliding buckets; each bucket contains a pointer to the next one in the chain.
* **Open addressing** method: The algorithm probes the hashmap's array of buckets, saving the data into the first free index encountered.
    
Both strategies have their drawbacks:
* **Separate chaining** requires runtime memory allocation each time a hash collision occurs, which can be costly when the hashmap is almost full and collisions happen often, or if the hashing algorithm produces poor distribution. It can also become complicated to track the lifetime of all the dynamically allocated memory for collided buckets as the hashmap grows.
* **Open addressing** places collided elements outside of their designated slots, potentially taking up slots that would be valid for other keys. This means those elements will have to resort to probing as well, creating a chain reaction of this "layering" known as [primary clustering](https://en.wikipedia.org/wiki/Primary_clustering). It also relies on "tombstone" elements; removed buckets aren't removed per se but are rather just marked as removed so the probing algorithm won't stumble upon an empty element where it shouldn't. These "tombstone" elements persist in the hashmap's bucket array until they no longer interfere with the algorithm and can actually be removed, usually until the hashmap is rehashed or cleared. 
 
This implementation uses a modified version of the **open adressing** method. It avoids using tombstones by establishing and complying with the following rule:

**An entry with an arbitrary ID is guaranteed to be at that ID in the hashmap's entries array and will be the first element of its chain**.

In this implementation, buckets are called hashmap entries and are defined in the `dtrx__hm_entry` struct, which looks like this:
```
1 struct dtrx__hm_entry {
2         void *value;
3         char *key;
4         dtrx_free_info fi; 
5         struct dtrx__hm_entry *ne; 
6         struct dtrx__hm_entry *pe;
7 };
```
It contains a key, a value, freeable information, and two pointers - to the next entry and to the previous one - making the general entry storing structure a [doubly linked list](https://en.wikipedia.org/wiki/Doubly_linked_list).

So, during this step, the function:
* Creates a `dtrx__hm_entry *` variable and initializes it with the address of an entry that is currently in the hashmap's entry array at the index the hashing function returned during **step 1** .
* It checks the entry's key for `NULL`, if it's not `NULL`, this means that the algorithm either encountered the aforementioned guaranteed first link of the chain, or an element of another chain. This is also the part where the 
>  The only part worth noting here is the use of `calloc` when assigning to `hm->data_block`
>
note from **section 1** becomes important, because if the memory were allocated with `malloc()` it would contain non-zero garbage and falsely trigger the `NULL` check. 

* In any case, the algorithm first finds the next free ID in the entries array **(Lines 17-18)**.
* To then determine if the entry it found at the current ID is a part of this ID's chain or not it calls the `dtrx__hash()` function with the entry's key as an argument.
* From here, the logic splits into two paths; the function:
    * **(Lines 20-31)**: Compares the hashes. If they aren't equal, meaning that the ID is occupied by an element of another chain, the function accesses this entry's previous entry and then redirects its "next entry" pointer (which points at the current entry at that time) to point to the newly found free slot in the entries array. It doesn't need to check whether the current entry has a previous entry in its chain because, according to the **"An entry with an arbitrary ID is guaranteed to be at that ID in the hashmap's entries array and will be the first element of its chain"** rule, this entry cannot be the first in its chain by defintion, since it was found at another ID.

        Then it does the same redirection, but in reverse. If the entry has a following entry in the chain, the function accesses it and redirects its "previous entry" pointer to point to the newly found free slot in the entries array.

        Now that both the previous and next entries are pointing to the next free slot in the entries array, the function just copies the entry into that slot, preserving the chain integrity.

        Finally, it `NULL`s the entry's "previous entry" field, leaving other fields of the entry as is; they don't matter anymore and are treated as "empty", since the entry was copied over to the new ID. They will be filled with the new entry's data at the actual insertion part at the end of the function, making this entry a valid first entry in the chain. It also prints some debugging information if the `DTRX_DEBUG` flag was specified.

        Here's how this logic can be visualized: 
        ![image 1](https://github.com/user-attachments/assets/880a595f-76cd-4b90-80fa-d2df7c971852)
    * **(Lines 32-50)**: If the hashes are equal, meaning that the ID is occupied by an entry that is the first in the chain of entries with this hash, the function needs to find the last entry in the chain. 
    
        To do that, it defines another `dtrx__hm_entry *` variable named `l_entry` - a "linked entry" pointer that will be used to descend the chain in a `while(){...}` loop without changing the main entry pointer. In each iteration of this loop, the algorithm:
        
        * Compares the linked entry's key with the one that was passed into the function as an argument using the [strcmp()](https://en.cppreference.com/w/c/string/byte/strcmp) function. Finding an identical key causes a logical error; a hashmap shouldn't contain two elements with the same key. In that case, it prints a warning if the `DTRX_DEBUG` flag was specified, frees the memory that was allocated during **Step 2**, and returns a `DTRX_FAILURE` callback.
        * If the `l_entry`'s next entry field is `NULL`, it breaks out of the loop; the last chain link is now in `l_entry` variable.
        * If the `l_entry`'s next entry field is not `NULL` and the loop continues, the algorithm assigns that next entry to the `l_entry` variable, descending down the chain by one step.

        Once the loop is finished and `l_entry` contains the address of the last link in the chain, the function rewrites the original entry pointer, pointing it to the next found free slot in the hashmap's entries array.

        Then it assigns the entry to the "next entry" field of `l_entry` and `l_entry` to the "previous entry" field of the entry, adding an additional entry to the end of this entry chain. Just like in the case of different hashes, at this point, the data passed to the function hasn't been inserted yet and the entry pointer just points to an empty slot. It also prints some debugging information if the `DTRX_DEBUG` flag was specified.
        
        Here's how this logic can be visualized: 
        ![image 2](https://github.com/user-attachments/assets/e98b9dd7-853c-42f1-b2d1-bfa9a27d4e15)

* **Step 4:** On **Lines 52-54**, there's another `DTRX_DEBUG_MODE` macro call for the case where the key is `NULL`, meaning the entry will be the first in its chain and can be added as-is without any manipulation. This part will be removed by the compiler if the corresponding compile flag is not cpecified, allowing for no performance overhead.
* **Step 5:** Finally, it uses the arguments that were passed to it to fill the corresponding fields of whatever slot the entry pointer points to after all the manipulations from the previous steps; it then returns `DTRX_SUCCESS`.

## **6: Remapping the hashmap**
In **Section 4**, the `dtrx__remap()` function was mentioned. It gets called when the number of entries in the hashmap's entries array exceeds a threshold set during hashmap creation. Here's how this function is defined:
``` 
 1 static void dtrx__remap(dtrx_hashmap *hm){
 2         DTRX_DEBUG_MODE(printf(DTRX__CYAN_COLOR "==========" DTRX__RESET_COLOR "\n THE MAP IS GETTING TOO SMALL (%ld ELEMENTS AT %ld CAPACITY), REMAPPING...\n" DTRX__CYAN_COLOR "==========" DTRX__RESET_COLOR"\n",hm->count,hm->size););
 3         uint64_t new_size = hm->size * hm->remap_multiplier;
 4         dtrx__hm_entry *data_block = (dtrx__hm_entry *)calloc((size_t)new_size, sizeof(dtrx__hm_entry));
 5         if(data_block == NULL) DTRX__PANIC("CALLOC RETURNED NULL");
 6         dtrx__hm_entry *tmp_ptr = hm->data_block;
 7         dtrx__hm_entry *entry = NULL;
 8         uint64_t i;
 9         for(i = 0; i < hm->size; i++){
10                 entry = &(tmp_ptr[i]);
11                 entry->pe = NULL;
12                 entry->ne = NULL;
13 
14                 if(entry->key != NULL){
15                         dtrx__insert_value(data_block,new_size,entry->key,DTRX__FORBID,entry->value,entry->fi);
16                 }
17         }
18         dtrx__safe_free(hm->data_block);
19 
20         hm->data_block = data_block;
21         hm->size = new_size;
22         hm->remap_limit *= hm->remap_multiplier;
23         DTRX_DEBUG_MODE(dtrx__print_hm_info(hm););
24 }
```
Now for what it does; it: 
* **Step 1:** Prints some debugging information with the `DTRX_DEBUG_MODE` call.
* **Step 2:** Prepares some data to later iterate on:
    * A `new_size` value, which is calculated by multiplying the current hashmap size by a multiplier passed during hashmap creation.
    * Three `dtrx__hm_entry *` variables: 
        * `data_block` - a pointer to a new chunk of `calloc()`ated memory that will be the new entries array; if the memory were allocated with `malloc()`, it would contain non-zero garbage and falsely trigger the `NULL` check during the insertion step.
        * `tmp_ptr`, which gets assigned the entries array that the hashmap currently points to.
        * A `NULL` `dtrx__hm_entry` pointer that will be used to iterate over the entries array.
* **Step 3:** Enters a `for(){...}` loop, where it iterates over the whole hashmap's entries array. On each iteration, the function:
    * Assigns the entry at the current ID to the previously created `entry` pointer (this is done purely for readability; an element can be accessed directly by its ID. You can change that and win a tiny bit of performance). It then `NULL`s its "previous entry" and "next entry" fields to avoid having dangling pointers once the entries array is freed.
    * If the current entry has a key (meaning that it's a valid entry), the function calls the `dtrx__insert_value` function, passing it this entry, the newly allocated memory chunk, the recalculated size value, and a `DTRX__FORBID` mask (to avoid reallocating memory for the key, as was mentioned in **Section 5, Step 2**). The rest of the arguments passed are regular entry fields.

        As mentioned above, the function iterates over each element of the hashmap's entries array, including the empty ones. Most hash functions will experience more collisions the closer the load factor gets to ~70-80% of the hashmap capacity, so the threshold for rehashing is usually set at around this percentage. That being said, it will iterate over ~20% of elements that will be empty, which can be costly if the hashmap is very big. To avoid that, a separate array can be created, which will contain only valid IDs. Iterating over said array will allow for constant _O(n)_ access time by avoiding unnecessary empty elements. This is a memory-performance trade-off, and by default, this implementation tries to save memory.
* **Step 4:** Frees the current entries array by calling the `dtrx__safe_free` macro.
* **Step 5:** Assigns the new entries array, size, and remap limit to the corresponding hashmap fields.
* **Step 6:** Prints out debugging information with the `DTRX_DEBUG_MODE` call.
        
## 7: Retrieving a value from the hashmap
To retrieve a value from the hashmap, the `dtrx_get_value()` function is used. It has the following definition:
```
 1 void *dtrx_get_value(dtrx_hashmap *hm, const char *key){
 2         uint64_t hash = dtrx__hash(key,hm->size);
 3         if(hm->data_block[hash].key == NULL){DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "TRYING TO ACCESS A NULL ELEMENT WITH HASH/KEY: [%ld]/[%s], INVALID KEY OR DELETED ENTRY\n",hash,key);); return NULL;}
 4 
 5         dtrx__hm_entry *entry = NULL;
 6         for (entry = &(hm->data_block[hash]); entry != NULL; entry = entry->ne){
 7                 if(strcmp(key,entry->key) == 0){
 8                         DTRX_DEBUG_MODE(printf("Returning an element with hash/key: [%ld]/[%s]\n",hash,entry->key););
 9                         return entry->value;
10                 }
11         }
12         DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "TRYING TO ACCESS A NULL ELEMENT WITH HASH/KEY: [%ld]/[%s], INVALID KEY OR DELETED ENTRY\n",hash,key););
13         return NULL;
14 }
```
It's a simple function; it:
* **Step 1:** Gets the hash of the passed key using the `dtrx__hash()` function and checks if the entry's key at that ID is `NULL`, if it is, it returns `NULL`.
* **Step 2:** If it's not `NULL`, it creates a `dtrx__hm_entry *` iterator variable, initializes it with the address of the entry at the current ID, and goes over all the entries in the chain at that ID in a `for(){...}` loop. On each iteration, it:
    * Checks if the entry is not `NULL`. If it is, the function exits the loop.
    * If it is not, it compares the key of the entry with the key that was passed as an argument using the [strcmp()](https://en.cppreference.com/w/c/string/byte/strcmp) function. And if they are identical, it prints out debugging information with the `DTRX_DEBUG_MODE` call and returns the entry's value.
    * Assigns the entry's next entry to the iterator pointer, descending one link down the chain.
* **Step 3:** If the loop finishes and the key is not found in the chain at its ID, the function prints out debugging information with the `DTRX_DEBUG_MODE` call and returns `NULL`.

## 8: Removing an entry from the hashmap
To remove an entry from the hashmap, the `dtrx_remove_value()` function is used. Here's how this function is defined:
```
 1 dtrx_callback dtrx_remove_value(dtrx_hashmap *hm, const char *key){
 2         uint64_t hash = dtrx__hash(key,hm->size);
 3         if(hm->data_block[hash].key == NULL){
 4                 DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "TRYING TO DELETE AN ELEMENT WITHOUT A KEY, PROBABLY ACCESSING PREVIOUSLY DELETED ENTRY OR THE KEY IS INVALID. KEY: [%s]\n",key););
 5                 return DTRX_FAILURE;
 6         }
 7 
 8         uint64_t entry_hash = dtrx__hash(hm->data_block[hash].key,hm->size);
 9         if(entry_hash != hash){
10                 DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "HASH MISMATCH, AN ELEMENT WITH KEY [%s] WAS PROBABLY NEVER ADDED\n",key););
11                 return DTRX_FAILURE;
12         }
13 
14         dtrx__hm_entry *entry;
15         for(entry = &(hm->data_block[hash]); entry != NULL; entry = entry->ne){
16                 if(strcmp(entry->key,key) == 0) break;
17         }
18         if(entry == NULL){
19                 DTRX_DEBUG_MODE(printf(DTRX__YELLOW_COLOR "WARNING: " DTRX__RESET_COLOR "COULDN'T FIND ENTRY IN THE CHAIN, THE ELEMENT WITH KEY [%s] IS NOT IN HM AND JUST HAPPENS TO HAVE A VALID HASH\n",key););
20                 return DTRX_FAILURE;
21         }
22         DTRX_DEBUG_MODE(
23                 size_t key_size = strlen(entry->key) + 1;
24                 char *deb_key = (char *)malloc(key_size);
25                 if(deb_key == NULL) DTRX__PANIC("MALLOC RETURNED NULL");
26                 memcpy(deb_key,(const void *)entry->key,key_size);
27         );
28         dtrx__free_entry(entry);
29         // Its first entry in chain
30         if(entry->pe == NULL){
31                 // It has following entries
32                 if(entry->ne != NULL){
33                         dtrx__hm_entry *tmp_ptr = entry->ne;
34                         DTRX_DEBUG_MODE(
35                                 uint64_t deb_hash = dtrx__hash(tmp_ptr->key,hm->size);
36                                 printf("Removing first entry in a chain. Swapping entry with hash/key [%ld]/[%s] with entry with hash/key [%ld]/[%s]\n",hash,deb_key,deb_hash,tmp_ptr->key);
37                         );
38                         *entry = *tmp_ptr;
39                         entry->pe = NULL;
40                         if(tmp_ptr->ne != NULL){
41                                 tmp_ptr->ne->pe = entry;
42                         }
43                         entry = tmp_ptr;
44                 }
45                 // If there is no following entries it means its alone and should be left as is
46                 DTRX_DEBUG_MODE(printf("Removing first entry in a chain with entry with hash/key [%ld]/[%s]\n",hash,deb_key););
47         }
48         // Its 2nd or later entry
49         else{
50                 if(entry->ne != NULL){
51                         DTRX_DEBUG_MODE(
52                                 uint64_t deb_ne_hash = dtrx__hash(entry->ne->key,hm->size);
53                                 uint64_t deb_pe_hash = dtrx__hash(entry->pe->key,hm->size);
54                                 printf("Removing entry in a chain: [%ld]/[%s] <= [%ld]/[%s] => [%ld]/[%s]\n",deb_pe_hash,entry->pe->key,hash,deb_key,deb_ne_hash,entry->ne->key);
55                         );
56                         entry->ne->pe = entry->pe;
57                         entry->pe->ne = entry->ne;
58                 }
59                 else{
60                         DTRX_DEBUG_MODE(
61                                 uint64_t deb_pe_hash = dtrx__hash(entry->pe->key,hm->size);
62                                 printf("Removing last entry in a chain: [%ld]/[%s] <= [%ld]/[%s]\n",deb_pe_hash,entry->pe->key,hash,deb_key);
63                         );
64                         entry->pe->ne = NULL;
65                 }
66         }
67         DTRX_DEBUG_MODE(dtrx__safe_free(deb_key););
68         *entry = NULL__ENTRY;
69         hm->count--;
70         return DTRX_SUCCESS;
71 }
```
This function might look overwhelming at first, but it simply has a lot of code for `DTRX_DEBUG_MODE` calls; the logic of the function itself is pretty straightforward; it:
* **Step 1:** Hashes the key using the `dtrx__hash()` function and checks if the entry's key at that ID is `NULL`. If it is, it prints a warning and returns `DTRX_FAILURE`.
* **Step 2:** Gets the hash of the key that an entry at the current hash/ID contains and compares it with the hash of the key that was passed as an argument. If they aren't equal, it prints a warning and returns `DTRX_FAILURE`. As mentioned in the **Section 5, Step 3**:
>**An entry with an arbitrary ID is guaranteed to be at that ID in the hashmap's entries array and will be the first element of its chain**.
>
So the situation where the hash of the key passed and the hash of the key that the entry at that ID contains aren't equal can only mean that the key passed is incorrect.
* **Step 3:** Creates a `dtrx__hm_entry *` iterator variable called `entry` and iterates over the chain of entries at that ID in a `for(){...} loop`. On each iteration, it compares the key with the key of the current entry using the `strcmp()` function, and if it finds an identical key, it breaks out of the loop.
* **Step 4:** Checks if the `entry` iterator is `NULL`; this can be true if the previous step couldn't find an entry in the whole chain. In that case, it prints a warning and returns `DTRX_FAILURE`.
* **Step 5:** The next thing after it establishes that the `entry` iterator is not `NULL` and contains the entry that the function is looking for is to free that entry. For that, the function uses the `dtrx__free_entry()` macro expansion, which is defined like this:
```
 1 #define dtrx__free_entry(entry_ptr) do { \
 2         uint64_t i; \
 3         if(entry_ptr->fi.count > 0){ \
 4                 for(i = 0; i < entry_ptr->fi.count; i++){ \
 5                         if(entry_ptr->fi.free_list[i] == NULL) DTRX__PANIC("ATTEMPTING TO FREE A NULL POINTER, PROBABLY ALREADY BEEN FREED SOMEWHERE IN THE CODE\n"); \
 6                         dtrx__safe_free(entry_ptr->fi.free_list[i]); \
 7                 } \
 8         } \
 9         dtrx__safe_free(entry_ptr->key); \
10         dtrx__safe_free(entry_ptr->value); \
11         dtrx__safe_free(entry_ptr->fi.free_list); \
12 } while(0) 
```
This macro takes a pointer to a `dtrx_hm_entry` struct and frees all of its internal resources.

* **Step 6** Now that the entry only contains auxiliary data regarding its position in the linked list, the function has to relink adjacent nodes before removing it completely:
    * It checks if the entry's "previous entry" field is `NULL`; if it is, meaning that the entry is the first one in its chain, the algorithm also checks its "next entry" field. If the "next entry" field is also `NULL`, meaning that the entry is the only one in its chain at that ID, the entry is left as-is for deletion. 
    * If, however, the entry has a next entry, the function has to move it to the current entry's ID and relink its pointers to keep the linked list integrity and to obey the rule:
    >**An entry with an arbitrary ID is guaranteed to be at that ID in the hashmap's entries array and will be the first element of its chain**.
    
     To achieve that, the function:
    * Creates a `dtrx__hm_entry *` temporary pointer and assigns that next entry to it.
    * Performs a shallow copy from the temporary pointer to the entry. Both entries now contain the same data from the entry's next entry.
    * Sets the entry's "previous entry" field to `NULL` to mark that this entry is now the first one in the chain.
    * If the entry at temporary pointer's address also has a linked entry, it accesses that third entry and redirects its "previous entry" pointer to point to the original entry's address.
    * Assigns the temorary pointer to the entry pointer and leaves it for deletion.
    
  ![image 3](https://github.com/user-attachments/assets/8c063635-10cb-46c9-aeda-54daf4d196be)
    * If the condition for the previous entry is satisfied, meaning that the entry was found somewhere down the chain, the algorithm only relinks the adjacent entries:
        * If the entry has a valid next entry pointer, it bridges its neighbors: the "next entry" pointer of the previous entry is updated to point to the current entry's successor, and vice versa.
        * If the entry doesn't have a valid next entry pointer, it sets the previous entry's "next entry" pointer to `NULL`.

   ![image 4](https://github.com/user-attachments/assets/127f4ab0-cdee-4eb1-9200-43d665933569)
* **Step 7:** Finally, the function "removes" the entry by assigning it a statically defined constant `NULL__ENTRY`, which is defined as follows: 
```
1 static const struct dtrx__hm_entry NULL__ENTRY = {0};
```
It then decreases the hashmap's counter and returns `DTRX_SUCCESS`. It does not deallocate anything, because the hashmap's entries array is a single continuous chunk of memory, and each entry is just an offset in that chunk, not an allocated piece of memory itself; so, an entry cannot be `free()`d individually.

## 9: Deleting the hashmap
To delete the whole hashmap, the `dtrx_delete_hashmap()` function is used. It deallocates all of the hashmap's entries, their associated data, and the hashmap itself. This function's definition looks like this:
```
 1 void dtrx_delete_hashmap(dtrx_hashmap *hm){
 2         uint64_t i;
 3         dtrx__hm_entry *entry;
 4         for(i = 0; i < hm->size; i++){
 5                 entry = &hm->data_block[i];
 6                 if(entry->key != NULL){
 7                         dtrx__free_entry(entry);
 8                 }
 9         }
10         dtrx__safe_free(hm->data_block);
11         dtrx__safe_free(hm);
12 }
```
The algorithm:
* **Step 1:** Iterates over each entry in the hashmap's entries array. If an entry contains a key, it frees all of its members that contain dynamically allocated memory by calling the `dtrx__free_entry` macro.
* **Step 2:** Once the loop completes and all the entries have been freed, it uses the `dtrx__safe_free` macro to first free the hashmap's entries array (which is just a single continuous chunk of allocated memory) and then the hashmap itself.

## 10: File structure
Being a single header-file library and supporting C++ forces a certain file structure. This library's file structure can be generally described as follows:
```
#ifndef DTRX_HASHMAP_H
#define DTRX_HASHMAP_H

#ifdef __cplusplus
extern "C" {
#endif

** "public" declarations, macros **

#ifdef DTRX_HM_IMPLEMENTATION
#ifndef DTRX_HM_IMPLEMENTATION_DONE
#define DTRX_HM_IMPLEMENTATION_DONE

** "private" definitions, macros **

#endif // DTRX_HM_IMPLEMENTATION_DONE
#endif // DTRX_HM_IMPLEMENTATION
#ifdef __cplusplus
}
#endif
#endif // DTRX_HASHMAP_H
```
* When an `#include<some_file.h>` directive is executed in C, the entire content of the file specified in it is inserted as text into the caller file. This behavior will cause a complie-time redifinition errors if multiple different files and header files include the same file. 

To avoid this, an [include guard](https://en.wikipedia.org/wiki/Include_guard) can be used; the `DTRX_HASHMAP_H` definition is one such guard. It wrappes the entire file into conditional preprocessor block, so the content is only processed if the macro has not been defined yet. After the first `include`, the `DTRX_HASHMAP_H` macro is defined, so any following `include` directives will skip the file's content.

* To ensure the compatibility with C++ projects, the `extern "C"` linkage specification is used, it tells the compiler to treat the declarations inside as standart C code by disabling [name mangling](https://en.wikipedia.org/wiki/Name_mangling) to ensure the linker can find the correct function symbols. This allows the file to be used in both C and C++ environments.

It is also wrapped in a conditional preprocessor block, that looks for the `__cplusplus` macro. This is a predefined macro that is automatically injected by C++ compilers.

* The `DTRX_HM_IMPLEMENTATION` macro is an [stb-style implementation guard](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt). The information at the link pretty much explains everything about that kind of pattern. 

In short, it "splits" the file into declaration and implementation parts. By doing so, it allows the user to `#include` the file in multiple places to access the type and function declarations, but only define them once by defining the implementation guard (in this case, the `DTRX_HM_IMPLEMENTATION) in a single place, thus avoiding redefinition errors. This allows for great flexibility and ease of use on the programmers side. 
The `DTRX_HM_IMPLEMENTATION_DONE` macro is a "second line of defence"; it is needed to prevent multiple inclusions by accident.

Apart from looking into the stb libraries, I would also recommend watching [this Tsoding video](https://www.youtube.com/watch?v=kS_GqDp6IT4) to understand the pattern better; it is short and provides a simple visual example.
