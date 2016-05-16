#ifndef __TRIE_H__
#define __TRIE_H__

#define TRIE_CHILDREN	      4

#define TRIE_NOT_LAST	      -1

struct trie;
struct child
{
  int symb;
  ssize_t last;
  struct trie *  next;
};

struct trie
{
  unsigned int children_size;
  unsigned int children_count;
  struct child *  children;
};

struct trie *  trie_new (void);
void trie_add_word (struct trie *, const char *, size_t, ssize_t);
void trie_print (struct trie *);
void trie_free (struct trie *);
ssize_t trie_search (struct trie *, const char *, size_t);
struct trie * trie_check_prefix (struct trie *, const char *, size_t, ssize_t *);

#endif
