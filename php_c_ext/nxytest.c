/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_nxytest.h"


#define UTF8_STR_LEN		5
#define TRIE_CHILDREN	      4
#define TRIE_NOT_LAST	      -1
struct trie;
struct child{
	char symb[UTF8_STR_LEN];
	zend_ulong last;
	struct trie *  next;
	struct trie *  fail;
};

struct trie{
	unsigned int children_size;
	unsigned int children_count;
	struct child *  children;
	struct child *  parent;
};

/* Allocate a new empty trie.  */
struct trie * trie_new (){
	struct trie *  trie = (struct trie *) malloc (sizeof (struct trie));
	trie->children_size = TRIE_CHILDREN;
	trie->children_count = 0;
	trie->children = (struct child *)malloc (TRIE_CHILDREN * sizeof (struct child));
	memset (trie->children, 0, TRIE_CHILDREN * sizeof (struct child));
	return trie;
}


/* Helper for bsearch and qsort.  */
static inline int cmp_children (const void *  k1, const void *  k2){
	struct child *  c1 = (struct child *)k1;
	struct child *  c2 = (struct child *)k2;
	//printf("cmp:%s?%s=%d\n",c1->symb,c2->symb,strcmp(c1->symb,c2->symb));
	return strcmp(c1->symb,c2->symb);
}


/* Search for a symbol in a children of a certain trie.  Uses binary search
   as the children are kept sorted.  */
static struct child * trie_search_child (struct trie * trie, char symb[UTF8_STR_LEN]){
	struct child s;

	if (trie->children_count == 0)
		return NULL;

	strcpy(s.symb,symb);
	//printf("search:%s\n",symb);
	return  (struct child *)bsearch (&s, trie->children, trie->children_count,sizeof (struct child), cmp_children);
}

#define set_utf8_with_offset(des,src,offset)              \
	do {                                              \
		if((src&0xFF&0xC0) != 0){	\
			if((src&0xF0) == 0xF0){	\
				if((src&0xFF) > 0xF7){	\
					offset = 0;	\
				}else{	\
					offset = 4;	\
				}	\
			}else if((src&0xE0) == 0xE0){	\
				offset = 3;	\
			}else{	\
				offset = 2;	\
			}	\
		}else{	\
			offset = 1;	\
			break;	\
		}	\
	} while (0)

/* Add a word to the trie.  */
void trie_add_word (struct trie * trie, const char * word, size_t length, zend_ulong info){
	struct child *  child;
	struct trie *  nxt = NULL;
	char symb[UTF8_STR_LEN];
	int offset = 0;
	//printf("%s,%x,%x,%s\n",word,word[0],word[0]&0xFF,((word[0]&0xF0) != 0)?"y":"n");
	set_utf8_with_offset(symb,word[0],offset);
	if(offset > 0){
		strncpy(symb,&word[0],offset);
		symb[offset] = '\0';
	}else{
		return NULL;
	}
	//printf("%d,%s\n",offset,symb);
	//if(0 <= word[0] && word[0] <= 127){
	//	offset = 1;
	//	strncpy(symb,&word[0],offset);
	//	symb[offset] = '\0';
	//}else{
	//	offset = 3;
	//	strncpy(symb,&word[0],offset);
	//	symb[offset] = '\0';
	//}

	assert (trie != NULL);

	//printf("content:%s\n",symb);
	//if(trie->parent!=NULL){
	//	printf("parent:%s;",trie->parent->symb);
	//}
	child = trie_search_child (trie, symb);

	if (child){
		//printf("exist:%d,%s,%d,%s\n",offset,child->symb,length,word);
		if (length == offset)
			child->last = info;
		if (length > offset && child->next == NULL){
			child->next = trie_new ();
			child->next->parent = child;
			//printf("test_parent:%s\n",child->next->parent->symb);
		}

		nxt = child->next;
	}else{
		//printf("noexist:%d,%s,%d,%s\n",offset,symb,length,word);
		//printf("realloc:%d,%d\n",trie->children_count,trie->children_size);
		if (trie->children_count >= trie->children_size){
			//printf("size:%d\n",trie->children_size);
			trie->children_size *= 2;
			//printf("size:%d\n",trie->children_size);
			trie->children = (struct child *)realloc (trie->children,trie->children_size * sizeof (struct child));
		}

		strncpy(trie->children[trie->children_count].symb,symb,offset);
		trie->children[trie->children_count].symb[offset] = '\0';
		trie->children[trie->children_count].fail = NULL;
		if (length > offset){
			trie->children[trie->children_count].next = trie_new ();
			trie->children[trie->children_count].last = TRIE_NOT_LAST;
		}else{
			trie->children[trie->children_count].next = NULL;
			trie->children[trie->children_count].last = info;
		}

		nxt = trie->children[trie->children_count].next;
		trie->children_count++;

		/* XXX This qsort may not perform ideally, as actually we are always
		   just shifting a number of elements a the end of the array one
		   element to the left.  Possibly qsort, can figure it out and work
		   in O (N) time.  Otherwise better alternative is needed.  */
		qsort (trie->children, trie->children_count,sizeof (struct child), cmp_children);
		unsigned int i;
		for(i=0;i<trie->children_count;i++){
			if(trie->children[i].next!=NULL){
				trie->children[i].next->parent = &trie->children[i];
			}
		}
	}

	//trie_print(trie);
	//printf("//////\n");

	if (length > offset){
		trie_add_word (nxt, &word[offset], length - offset, info);
	}
}


#define tab(n)			  \
do {				  \
  int __i;			  \
  for (__i = 0; __i < n; __i++)	  \
    printf ("  ");		  \
} while (0)

/* Print the trie.  */
static void _trie_print (struct trie *  t, int level){
	unsigned int i;
	if (!t)
		return;

	for (i = 0; i < t->children_count; i++){
		tab (level);
		if(t->parent!=NULL){
			printf("parent:%s;",t->parent->symb);
		}
		if(t->children[i].fail != NULL){
			if(t->children[i].fail->parent==NULL){
				printf("fail:root;");
			}else{
				printf("fail:%s;",t->children[i].fail->parent->symb);
			}
		}else{
				printf("fail:error;");
		}
		printf("%s %s\n", t->children[i].symb,t->children[i].last != TRIE_NOT_LAST ? "[last]" : "");
		_trie_print (t->children[i].next, level+1);
	}
}

/* Wrapper for print.  */
void trie_print (struct trie *  t){
	_trie_print (t, 0);
}

/* Print the trie.  */
static void _trie_build_fail(struct trie *  t,struct trie * root){
	unsigned int i;
	struct trie * p;
	struct child* child;
	if (!t)
		return;
	for (i = 0; i < t->children_count; i++){
		if(t->parent == NULL){
			t->children[i].fail = root;
		}else{
			p = t->parent->fail;
			while(p != NULL){
				child = trie_search_child (p, t->children[i].symb);
				if(child){
					if(child->next != NULL){
						t->children[i].fail = child->next;
						break;
					}
				}
				if(p->parent == NULL){
					p = NULL;
				}else{
					p = p->parent->fail;
				}
			}
			if (p == NULL){
				t->children[i].fail = root;
			}
		}
	}
	for (i = 0; i < t->children_count; i++){
		_trie_build_fail(t->children[i].next,root);
	}
}

/* Build Fail Point.  */
void trie_build_fail(struct trie *  t){
	_trie_build_fail(t,t);
}


/* Deallocate memory used for trie.  */
void trie_free (struct trie *  trie){
	unsigned int  i;
	if (!trie)
		return;

	for (i = 0; i < trie->children_count; i++)
		trie_free (trie->children[i].next);

	if (trie->children)
		free (trie->children);
	free (trie);
}

/* Search for word in trie.  Returns true/false.  */
ssize_t trie_search (struct trie *  trie, const char *  word, size_t length){
	struct child *  child;

	assert (length > 0);
	if (trie == NULL)
		return TRIE_NOT_LAST;

	char symb[UTF8_STR_LEN];
	int offset = 0;
	set_utf8_with_offset(symb,word[0],offset);
	if(offset > 0){
		strncpy(symb,&word[0],offset);
		symb[offset] = '\0';
	}else{
		return TRIE_NOT_LAST;
	}
	//if(0 <= word[0] && word[0] <= 127){
	//	offset = 1;
	//	strncpy(symb,&word[0],offset);
	//	symb[offset] = '\0';
	//}else{
	//	offset = 3;
	//	strncpy(symb,&word[0],offset);
	//	symb[offset] = '\0';
	//}

	child = trie_search_child (trie, symb);

	if (!child)
		return TRIE_NOT_LAST;

	if (length == offset)
		return child->last;
	else
		return trie_search (child->next, &word[offset], length - offset);
}

#define SEARCH_RES_SIZE 4
struct search_res{
	unsigned int size;
	unsigned int count;
	zend_ulong* res;
};

struct search_res* str_search(struct trie *  trie, const char *  word, size_t length){
	struct search_res* res = (struct search_res*)malloc(sizeof(struct search_res));
	res->count = 0;
	res->size = SEARCH_RES_SIZE;
	res->res = malloc(SEARCH_RES_SIZE*sizeof(zend_ulong));
	memset(res->res,0,SEARCH_RES_SIZE*sizeof(zend_ulong));
	
	struct child*  child = NULL;
	struct trie* p = trie;
	struct child* t;
	unsigned int i = 0;
	char symb[UTF8_STR_LEN];
	int offset = 0;
	for(;i<length;){
		set_utf8_with_offset(symb,word[i],offset);
		if(offset > 0){
			strncpy(symb,&word[i],offset);
			symb[offset] = '\0';
		}else{
			return NULL;
		}
		//printf("%d,%s\n",offset,symb);
		//if(0 <= word[i]&& word[i]<= 127){
		//	offset = 1;
		//	strncpy(symb,&word[i],offset);
		//	symb[offset] = '\0';
		//}else{
		//	offset = 3;
		//	strncpy(symb,&word[i],offset);
		//	symb[offset] = '\0';
		//}
		i += offset;
		while(p->parent!=NULL){
			child = trie_search_child(p, symb);
			if(child)break;
			if(p->parent!=NULL)p = p->parent->fail;
		}
		if(p->parent==NULL)child = trie_search_child(p, symb);
		if(child){
			if(child->next == NULL){
				p = child->fail;
			}else{
				p = child->next;
			}
			t = child;
			while(t != NULL){
				if(t->last != -1){
					if(res->count >= res->size){
						res->size *= 2;
						res->res = realloc(res->res,res->size*sizeof(zend_ulong));
					}
					res->res[res->count] = t->last;
					res->count++;
				}
				t = t->fail->parent;
			}
		}
	}
	return res;
}

/* Searches if a given word can be found in the database and if a trie follows
   it.  Returns trie, which follows a given prefix, and sets LAST to true if
   the word itself can be found inside the trie.  */
struct trie * trie_check_prefix (struct trie *  trie, const char *  word, size_t length,zend_ulong*  last){
	struct child *  child;

	assert (length > 0);
	if (trie == NULL){
		*last = TRIE_NOT_LAST;
		return NULL;
	}

	char symb[UTF8_STR_LEN];
	int offset = 0;
	set_utf8_with_offset(symb,word[0],offset);
	if(offset > 0){
		strncpy(symb,&word[0],offset);
		symb[offset] = '\0';
	}else{
		return NULL;
	}
	//if(0 <= word[0] && word[0] <= 127){
	//	offset = 1;
	//	strncpy(symb,&word[0],offset);
	//	symb[offset] = '\0';
	//}else{
	//	offset = 3;
	//	strncpy(symb,&word[0],offset);
	//	symb[offset] = '\0';
	//}

	child = trie_search_child (trie, symb);

	if (!child){
		*last = TRIE_NOT_LAST;
		return NULL;
	}

	if (length == offset){
		*last = child->last;
		return child->next;
	}else
		return trie_check_prefix (child->next, &word[offset], length - offset, last);
}

/* If you declare any globals in php_nxytest.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(nxytest)
*/

/* True global resources - no need for thread safety here */
static int le_nxytest;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("nxytest.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_nxytest_globals, nxytest_globals)
    STD_PHP_INI_ENTRY("nxytest.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_nxytest_globals, nxytest_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_nxytest_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(nxytest)
{
	zend_string *strg;

	strg = strpprintf(0, "hello");

	RETURN_STR(strg);
}

PHP_FUNCTION(confirm_nxytest_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "nxytest", arg);

	RETURN_STR(strg);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_nxytest_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_nxytest_init_globals(zend_nxytest_globals *nxytest_globals)
{
	nxytest_globals->global_value = 0;
	nxytest_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */

zend_class_entry *testobj_ce;

PHP_METHOD(testobj,learn){
	char *love;
	size_t love_len=0;
	if(zend_parse_parameters(ZEND_NUM_ARGS(),"s",&love,&love_len)==FAILURE){
		return;
	}
	zend_update_property_string(testobj_ce,getThis(),"memory",sizeof("memory")-1,love);
}

PHP_METHOD(testobj,__construct){
	zval     *input,*entry;
	if(zend_parse_parameters(ZEND_NUM_ARGS(),"a",&input)==FAILURE){
		return;
	}
	zend_update_property(testobj_ce,getThis(),"keys",sizeof("keys")-1,input);
	struct trie *  dict = trie_new();
	dict->parent = NULL;
        zend_string *string_key;
        zend_ulong num_key;
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(input),num_key,string_key, entry) {
		if (UNEXPECTED(Z_ISREF_P(entry) && Z_REFCOUNT_P(entry) == 1)) {
			entry = Z_REFVAL_P(entry);
		}
		//php_var_dump(zval_get_long(entry));
		trie_add_word (dict,Z_STRVAL_P(entry), strlen (Z_STRVAL_P(entry)), num_key);
	} ZEND_HASH_FOREACH_END();
	trie_build_fail(dict);

	zval zv;
	ZVAL_OBJ(&zv, dict);
	zend_update_property(testobj_ce,getThis(),"actree",sizeof("actree")-1,&zv);

	//trie_print (dict);
}

PHP_METHOD(testobj,__destruct){
	struct trie *  dict;
	zval *actree;
	actree = zend_read_property(testobj_ce,getThis(),"actree",sizeof("actree")-1,0,&actree);
	dict = Z_OBJ_P(actree);
	trie_free (dict);
	efree(actree);
}

//typedef _node{
//	
//} node;

PHP_METHOD(testobj,check_keys){
	//struct trie *  dict;

	zval *keys,*entry;		/* An entry in the input array */
	//zval *keys,*entry,*actree;		/* An entry in the input array */
	keys = zend_read_property(testobj_ce,getThis(),"keys",sizeof("keys")-1,0,&keys);
	//actree = zend_read_property(testobj_ce,getThis(),"actree",sizeof("actree")-1,0,&actree);
	//dict = Z_OBJ_P(actree);

	/* Initialize return array */
	array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(keys)));

	if (!zend_hash_num_elements(Z_ARRVAL_P(keys))) {
		return;
	}

	zend_hash_real_init(Z_ARRVAL_P(return_value), 1);

	//unsigned int i = 0;

	/* Go through input array and add values to the return array */
	ZEND_HASH_FILL_PACKED(Z_ARRVAL_P(return_value)) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(keys), entry) {
			if (UNEXPECTED(Z_ISREF_P(entry) && Z_REFCOUNT_P(entry) == 1)) {
				entry = Z_REFVAL_P(entry);
			}
			Z_TRY_ADDREF_P(entry);
			ZEND_HASH_FILL_ADD(entry);
			//trie_add_word (dict,Z_STRVAL_P(entry), strlen (Z_STRVAL_P(entry)), i++);
		} ZEND_HASH_FOREACH_END();
	} ZEND_HASH_FILL_END();

	//trie_build_fail(dict);
	//trie_print (dict);
	//str_search(dict,"fdsherfdssayfds",15);
	//trie_free (dict);
}


PHP_METHOD(testobj,search){
	struct search_res* res;
	char *searchstr;
	size_t str_len=0;
	if(zend_parse_parameters(ZEND_NUM_ARGS(),"s",&searchstr,&str_len)==FAILURE){
		return;
	}
	struct trie *  dict;

	zval *actree;		/* An entry in the input array */
	actree = zend_read_property(testobj_ce,getThis(),"actree",sizeof("actree")-1,0,&actree);
	dict = Z_OBJ_P(actree);
	res = str_search(dict,searchstr,str_len);
	//printf("%s\n",searchstr);
	//printf("total:%d\n",res->count);
	array_init_size(return_value, res->count);
	zend_hash_real_init(Z_ARRVAL_P(return_value), 1);
	if(res->count>0){
		zval *keys,*entry;		/* An entry in the input array */
		keys = zend_read_property(testobj_ce,getThis(),"keys",sizeof("keys")-1,0,&keys);
		unsigned int i;
		ZEND_HASH_FILL_PACKED(Z_ARRVAL_P(return_value)) {
			for(i=0;i<res->count;i++){
				entry = zend_hash_index_find(Z_ARRVAL_P(keys), res->res[i]);
				if (UNEXPECTED(Z_ISREF_P(entry) && Z_REFCOUNT_P(entry) == 1)) {
					entry = Z_REFVAL_P(entry);
				}
				Z_TRY_ADDREF_P(entry);
				ZEND_HASH_FILL_ADD(entry);
				//php_var_dump(zend_hash_index_find(Z_ARRVAL_P(keys), res->res[i]));
			}
		} ZEND_HASH_FILL_END();
	}
	free(res->res);
	free(res);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_testobj_learn,0,0,1)
	ZEND_ARG_INFO(0,love)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_testobj_keys,0,0,1)
	ZEND_ARG_INFO(0,keys)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_testobj_searchstr,0,0,1)
	ZEND_ARG_INFO(0,searchstr)
ZEND_END_ARG_INFO()

const zend_function_entry testobj_methods[] = {
	PHP_ME(testobj,learn,arginfo_testobj_learn,ZEND_ACC_PUBLIC)
	PHP_ME(testobj,__construct,arginfo_testobj_keys,ZEND_ACC_PUBLIC)
	PHP_ME(testobj,__destruct,NULL,NULL)
	PHP_ME(testobj,check_keys,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(testobj,search,arginfo_testobj_searchstr,ZEND_ACC_PUBLIC)
	{NULL,NULL,NULL}
};
PHP_MINIT_FUNCTION(nxytest)
{
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce,"testobj",testobj_methods);
	testobj_ce = zend_register_internal_class(&ce);
	zend_declare_property_string(testobj_ce,"memory",sizeof("memory")-1,"",ZEND_ACC_PUBLIC);
	zend_declare_property_null(testobj_ce,"keys",sizeof("keys")-1,ZEND_ACC_PUBLIC);
	zend_declare_property_null(testobj_ce,"actree",sizeof("actree")-1,ZEND_ACC_FINAL);
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(nxytest)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(nxytest)
{
#if defined(COMPILE_DL_NXYTEST) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(nxytest)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(nxytest)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "nxytest support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ nxytest_functions[]
 *
 * Every user visible function must have an entry in nxytest_functions[].
 */
const zend_function_entry nxytest_functions[] = {
	PHP_FE(nxytest,	NULL)		/* For testing, remove later. */
	PHP_FE(confirm_nxytest_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in nxytest_functions[] */
};
/* }}} */

/* {{{ nxytest_module_entry
 */
zend_module_entry nxytest_module_entry = {
	STANDARD_MODULE_HEADER,
	"nxytest",
	nxytest_functions,
	PHP_MINIT(nxytest),
	PHP_MSHUTDOWN(nxytest),
	PHP_RINIT(nxytest),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(nxytest),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(nxytest),
	PHP_NXYTEST_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_NXYTEST
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(nxytest)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
