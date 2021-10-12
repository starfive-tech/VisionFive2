#include "Driver.h"
#include "mm.tmh"
//#include "vbuddy_allocator.h"

#ifdef __DEBUG
#define DPRINT              printf
#else
#define DPRINT 
#endif

#define VMEM_PAGE_SIZE           (16*1024)

#define P_ALLOC(_x)         ExAllocatePoolWithTag(NonPagedPool, _x, 'VMEM')
#define P_FREE(_x)          ExFreePoolWithTag(_x, 'VMEM')
#if 0
#define ASSERT(_exp)        if (!(_exp)) { printf("ASSERT at %s:%d\n", __FILE__, __LINE__); while(1); }
#endif
#define HEIGHT(_tree)       (_tree==NULL ? -1 : _tree->height)

typedef struct page_struct {
    int             pageno;
    unsigned long   addr;
    int             used;
    int             alloc_pages;
    int             first_pageno;
} page_t;

/*
 * memory allocation for driver
 */

/*
 * doubly linked list
 */
typedef struct link_struct {
    struct link_struct* next;
    struct link_struct* prev;
} link_t;

static void list_init(link_t* l) {
    if (l) {
        l->next = l;
        l->prev = l;
    }
}

static link_t* list_add_after(link_t* ref, link_t* new_node) {
    link_t* next;

    if (ref == NULL || new_node == NULL) return NULL;

    next           = ref->next;
    ref->next      = new_node;
    new_node->prev = ref;
    new_node->next = next;
    if (next) next->prev = new_node;

    return ref;
}

static link_t* list_add_before(link_t* ref, link_t* new_node) {
    link_t* prev;

    if (ref == NULL || new_node == NULL) {
        return NULL;
    }

    prev = ref->prev;
    if (prev) prev->next = new_node;

    new_node->prev = prev;
    new_node->next = ref;
    ref->prev      = new_node;

    return ref;
}

static link_t* list_del(link_t* ref, link_t* node) {
    link_t* prev;
    link_t* next;

	UNREFERENCED_PARAMETER(ref);

    if (node == NULL) return NULL;

    prev = node->prev;
    next = node->next;

    if (prev) prev->next = next;
    if (next) next->prev = prev;

    node->next = NULL;
    node->prev = NULL;

    return node;
}

static int
list_is_empty(
    link_t* head
    )
{
    return (head == head->next ? 1 : 0);
}

static void*
mutex_alloc(
    void
    )
{
	return NULL;
}

static void
mutex_release(
    void* mutex
    )
{
	UNREFERENCED_PARAMETER(mutex);    
}

static void
mutex_lock(
    void* mutex
    )
{
	UNREFERENCED_PARAMETER(mutex);    
}

static void
mutex_unlock(
    void* mutex
    )
{
    UNREFERENCED_PARAMETER(mutex);
}

#define MAX(_a, _b)         (_a >= _b ? _a : _b)

typedef enum {
    LEFT,
    RIGHT
} rotation_dir_t;

#if defined(WIN32) || defined(WIN64)
#if (_MSC_VER == 1200)
typedef _int64              key_t;
#else
typedef unsigned long long  key_t;
#endif
#else
typedef unsigned long long  key_t;
#endif

#define MAKE_KEY(_a, _b)        (((key_t)_a)<<32 | _b)
#define KEY_TO_VALUE(_key)      (_key>>32)

typedef struct avl_node_struct {
    key_t   key;
    int     height;
    page_t* page;
    struct avl_node_struct* left;
    struct avl_node_struct* right;
} avl_node_t;

typedef struct avl_node_data_struct {
    int     key;
    page_t* page;
} avl_node_data_t;

static avl_node_t*
make_avl_node(
    key_t key,
    page_t* page
    )
{
    avl_node_t* node = (avl_node_t*)P_ALLOC(sizeof(avl_node_t));
    node->key     = key;
    node->page    = page;
    node->height  = 0;
    node->left    = NULL;
    node->right   = NULL;

    return node;
}

static int
get_balance_factor(
    avl_node_t* tree
    )
{
    int factor = 0;
    if (tree) {
        factor = HEIGHT(tree->right) - HEIGHT(tree->left);
    }

    return factor;
}

/*
 * Left Rotation
 *
 *      A                      B
 *       \                    / \
 *        B         =>       A   C
 *       /  \                 \
 *      D    C                 D
 *
 */
static avl_node_t*
rotation_left(
    avl_node_t* tree
    )
{
    avl_node_t* rchild;
    avl_node_t* lchild;

    if (tree == NULL) return NULL;
    
    rchild = tree->right;
    if (rchild == NULL) {
        return tree;
    }

    lchild = rchild->left;
    rchild->left = tree;
    tree->right = lchild;

    tree->height   = MAX(HEIGHT(tree->left), HEIGHT(tree->right)) + 1;
    rchild->height = MAX(HEIGHT(rchild->left), HEIGHT(rchild->right)) + 1;

    return rchild;
}

/*
 * Reft Rotation
 *
 *         A                  B
 *       \                  /  \
 *      B         =>       D    A
 *    /  \                     /
 *   D    C                   C
 *
 */
static avl_node_t*
rotation_right(
    avl_node_t* tree
    )
{
    avl_node_t* rchild;
    avl_node_t* lchild;

    if (tree == NULL) return NULL;

    lchild = tree->left;
    if (lchild == NULL) return NULL;

    rchild = lchild->right;
    lchild->right = tree;
    tree->left = rchild;

    tree->height   = MAX(HEIGHT(tree->left), HEIGHT(tree->right)) + 1;
    lchild->height = MAX(HEIGHT(lchild->left), HEIGHT(lchild->right)) + 1;

    return lchild;
}

static avl_node_t*
do_balance(
    avl_node_t* tree
    )
{
    int bfactor = 0, child_bfactor;       /* balancing factor */

    bfactor = get_balance_factor(tree);

    if (bfactor >= 2) {
        child_bfactor = get_balance_factor(tree->right);
        if (child_bfactor == 1 || child_bfactor == 0) {
            tree = rotation_left(tree);
        } else if (child_bfactor == -1) {
            tree->right = rotation_right(tree->right);
            tree        = rotation_left(tree);
        } else {
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
				"%!FUNC! invalid balancing factor: %d", (int)child_bfactor);       
			ASSERT(0);
            return NULL;
        }
    } else if (bfactor <= -2) {
        child_bfactor = get_balance_factor(tree->left);
        if (child_bfactor == -1 || child_bfactor == 0) {
            tree = rotation_right(tree);
        } else if (child_bfactor == 1) {
            tree->left = rotation_left(tree->left);
            tree       = rotation_right(tree);
        } else {
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
				"%!FUNC! invalid balancing factor: %d", (int)child_bfactor); 
			ASSERT(0);
            return NULL;
        }
    }
    
    return tree;
}
static avl_node_t*
unlink_end_node(
    avl_node_t* tree,
    int dir,
    avl_node_t** found_node
    )
{
    avl_node_t* node;
    *found_node = NULL;

    if (tree == NULL) return NULL;

    if (dir == LEFT) {
        if (tree->left == NULL) {
            *found_node = tree;
            return NULL;
        }
    } else {
        if (tree->right == NULL) {
            *found_node = tree;
            return NULL;
        }
    }

    if (dir == LEFT) {
        node = tree->left;
        tree->left = unlink_end_node(tree->left, LEFT, found_node);
        if (tree->left == NULL) {
            tree->left = (*found_node)->right;
            (*found_node)->left  = NULL;
            (*found_node)->right = NULL;
        }
    } else {
        node = tree->right;
        tree->right = unlink_end_node(tree->right, RIGHT, found_node);
        if (tree->right == NULL) {
            tree->right = (*found_node)->left;
            (*found_node)->left  = NULL;
            (*found_node)->right = NULL;
        }
    }

    tree->height = MAX(HEIGHT(tree->left), HEIGHT(tree->right)) + 1;

    return do_balance(tree);
}

/*
 *     A
 *   /  \    => B < A <= C
 *  B    C
 */
static avl_node_t*
avltree_find(
    avl_node_t* tree,
    int key
    )
{
    if (tree == NULL) return NULL;

    if (tree->key == key) {
        return tree;
    } else if (tree->key < key) {
        return avltree_find(tree->right, key);
    } else {
        return avltree_find(tree->left, key);
    }
}

static avl_node_t*
avltree_insert(
    avl_node_t* tree,
    key_t key,
    page_t* page
    )
{
    if (tree == NULL) {
        tree = make_avl_node(key, page);
    } else {
        if (key >= tree->key) {
            tree->right = avltree_insert(tree->right, key, page);
        } else {
            tree->left  = avltree_insert(tree->left, key, page);
        }
    }

    tree = do_balance(tree);

    tree->height = MAX(HEIGHT(tree->left), HEIGHT(tree->right)) + 1;

    return tree;
}

static avl_node_t*
do_unlink(
    avl_node_t* tree
    )
{
    avl_node_t* node;
    avl_node_t* end_node;
    node = unlink_end_node(tree->right, LEFT, &end_node);
    if (node) {
        tree->right = node;
    } else {
        node = unlink_end_node(tree->left, RIGHT, &end_node);
        if (node) tree->left = node;
    }

    if (node == NULL) {
        node = tree->right ? tree->right : tree->left;
        end_node = node;
    }

    if (end_node) {
        end_node->left  = (tree->left != end_node) ? tree->left : end_node->left;
        end_node->right = (tree->right != end_node) ? tree->right : end_node->right;
        end_node->height = MAX(HEIGHT(end_node->left), HEIGHT(end_node->right)) + 1;
    } 

    tree = end_node;

    return tree;
}

static avl_node_t*
avltree_remove(
    avl_node_t* tree,
    avl_node_t** found_node,
    key_t key
    )
{
    *found_node = NULL;
    if (tree == NULL) {
        DPRINT("failed to find key %d\n", key);
        return NULL;
    } 

    if (key == tree->key) {
        *found_node = tree;
        tree = do_unlink(tree);
    } else if (key > tree->key) {
        tree->right = avltree_remove(tree->right, found_node, key);
    } else {
        tree->left  = avltree_remove(tree->left, found_node, key);
    }

    if (tree) tree->height = MAX(HEIGHT(tree->left), HEIGHT(tree->right)) + 1;

    tree = do_balance(tree);

    return tree;
}

void
avltree_free(
    avl_node_t* tree
    )
{
    if (tree == NULL) return;
    if (tree->left == NULL && tree->right == NULL) {
        P_FREE(tree);
        return;
    }

    avltree_free(tree->left);
    tree->left = NULL;
    avltree_free(tree->right);
    tree->right = NULL;
}

#if 0
void
display_tree(
    avl_node_t* tree,
    FILE* fp
    )
{
    link_t Q[2];
    link_t* p, *list, *childQ ;
    struct my_list_node {
        link_t link;
        avl_node_t* data;
    } *node;
    int current_height;
    avl_node_t* a;
    int toggle = 1;

    return;
    if (tree == NULL) {

        printf("Empty tree!!\n");
        return;
    }

    if (fp == NULL) fp = stdout;

    list_init(&Q[0]);
    list_init(&Q[1]);
		
    node = (struct my_list_node*)P_ALLOC(sizeof(struct my_list_node));
    list_init(&node->link);
    node->data = tree;
    list_add_after(&Q[0], &node->link);

    current_height = tree->height;
    list   = &Q[0];
    childQ = &Q[1];
    while (list_is_empty(list) == 0) {
        p = list_del(list, list->next);
        a = ((struct my_list_node*)p)->data;

        fprintf(fp, "%x(pageno:%d, alloc:%d) ", (int)(a->key>>32), a->page->pageno, a->page->alloc_pages);
        if (a->left) {
            node = (struct my_list_node*)P_ALLOC(sizeof(struct my_list_node));
            node->data = a->left;
            list_init(&node->link);
            list_add_after(childQ->prev, &node->link);
        }
        if (a->right) {
            node = (struct my_list_node*)P_ALLOC(sizeof(struct my_list_node));
            node->data = a->right;
            list_init(&node->link);
            list_add_after(childQ->prev, &node->link);
        }
        P_FREE(p);

        if (list_is_empty(list)) {
            list = childQ;
            toggle = !toggle;
            childQ = &Q[toggle];
            fprintf(fp, "\n");
        }
    }
}
#endif

typedef struct _video_mm_struct {
    avl_node_t*     free_tree;
    avl_node_t*     alloc_tree;
    page_t*         page_list;
    int             num_pages;
    unsigned long   base_addr;
    unsigned long   mem_size;
    void*           mutex;
    int             free_page_count;
    int             alloc_page_count;
} video_mm_t;

static video_mm_t* s_mm = NULL;

static avl_node_t*
remove_approx_value(
    avl_node_t* tree,
    avl_node_t** found,
    key_t key
    )
{
    *found = NULL;
    if (tree == NULL) {
        return NULL;
    }

    if (key == tree->key) {
        *found = tree;
        tree = do_unlink(tree);
    } else if (key > tree->key) {
        tree->right = remove_approx_value(tree->right, found, key);
    } else {
        tree->left  = remove_approx_value(tree->left, found, key);
        if (*found == NULL) {
            *found = tree;
            tree = do_unlink(tree);
        }
    }
    if (tree) tree->height = MAX(HEIGHT(tree->left), HEIGHT(tree->right)) + 1;
    tree = do_balance(tree);

    return tree;
}

static void
set_blocks_free(
    int pageno,
    int npages
    )
{
    int last_pageno     = pageno + npages - 1;
    int i;
    page_t* page;
    page_t* last_page;

    ASSERT(npages);

    if (last_pageno >= s_mm->num_pages) {
        DPRINT("set_blocks_free: invalid last page number: %d\n", last_pageno);
       ASSERT(0);
        return;
    }

    for (i=pageno; i<=last_pageno; i++) {
        s_mm->page_list[i].used         = 0;
        s_mm->page_list[i].alloc_pages  = 0;
        s_mm->page_list[i].first_pageno = -1;
    }

    page        = &s_mm->page_list[pageno];
    page->alloc_pages = npages;
    last_page   = &s_mm->page_list[last_pageno];    
    last_page->first_pageno = pageno;

    s_mm->free_tree = avltree_insert(s_mm->free_tree, MAKE_KEY(npages, pageno), page);
}

static void
set_blocks_alloc(
    int pageno,
    int npages
    )
{
    int last_pageno     = pageno + npages - 1;
    int i;
    page_t* page;
    page_t* last_page;

    if (last_pageno >= s_mm->num_pages) {
        DPRINT("set_blocks_free: invalid last page number: %d\n", last_pageno);
        ASSERT(0);
        return;
    }

    for (i=pageno; i<=last_pageno; i++) {
        s_mm->page_list[i].used         = 1;
        s_mm->page_list[i].alloc_pages  = 0;
        s_mm->page_list[i].first_pageno = -1;
    }

    page        = &s_mm->page_list[pageno];
    page->alloc_pages = npages;

    last_page   = &s_mm->page_list[last_pageno];    
    last_page->first_pageno = pageno;

    s_mm->alloc_tree = avltree_insert(s_mm->alloc_tree, MAKE_KEY(page->addr, 0), page);
}

int 
vmem_init(
    unsigned long addr,
    unsigned long size
    )
{
    int i;

    if (s_mm) {
        DPRINT("already initialized\n");
        return -1;
    }

    s_mm = (video_mm_t*)P_ALLOC(sizeof(video_mm_t));
    if (s_mm == NULL) {
        DPRINT("vmem_init: No memory\n");
        return -1;
    }

    s_mm->base_addr  = addr;
    s_mm->mem_size   = size&~VMEM_PAGE_SIZE;
    s_mm->num_pages  = s_mm->mem_size/VMEM_PAGE_SIZE;
    s_mm->page_list  = (page_t*)P_ALLOC(s_mm->num_pages*sizeof(page_t));
    s_mm->free_tree  = NULL;
    s_mm->alloc_tree = NULL;
    s_mm->free_page_count = s_mm->num_pages;
    s_mm->alloc_page_count = 0;

    for (i=0; i<s_mm->num_pages; i++) {
        s_mm->page_list[i].pageno       = i;
        s_mm->page_list[i].addr         = s_mm->base_addr + i*VMEM_PAGE_SIZE;
        s_mm->page_list[i].alloc_pages  = 0;
        s_mm->page_list[i].used         = 0;
        s_mm->page_list[i].first_pageno = -1;
    }

    set_blocks_free(0, s_mm->num_pages);

    s_mm->mutex = mutex_alloc();

    return 0;
}

int
vmem_exit(
    void
    )
{
    if (s_mm == NULL) {
        DPRINT("vmem_exit: invalid handle\n");
        return -1;
    }

    mutex_release(s_mm->mutex);

    if (s_mm->free_tree) {
        avltree_free(s_mm->free_tree);
    }
    if (s_mm->alloc_tree) {
        avltree_free(s_mm->alloc_tree);
    }
    P_FREE(s_mm->page_list);
    P_FREE(s_mm);

    s_mm = NULL;

    return 0;
}

unsigned long
vmem_alloc(
    int size
    )
{
    avl_node_t* node;
    page_t*     free_page;
    int         npages, free_size;
    int         alloc_pageno;
    unsigned long  ptr;

    if (size <= 0) return (unsigned long)-1;

    mutex_lock(s_mm->mutex);

    npages = (size + VMEM_PAGE_SIZE -1)/VMEM_PAGE_SIZE;

    s_mm->free_tree = remove_approx_value(s_mm->free_tree, &node, MAKE_KEY(npages, 0));
    if (node == NULL) {
        mutex_unlock(s_mm->mutex);
        return (unsigned long)-1;
    }
    free_page = node->page;
    free_size = KEY_TO_VALUE(node->key);

    alloc_pageno = free_page->pageno;
    set_blocks_alloc(alloc_pageno, npages);
    if (npages != free_size) {
        int free_pageno = alloc_pageno + npages;
        set_blocks_free(free_pageno, (free_size-npages));
    }

    P_FREE(node);

    ptr = s_mm->page_list[alloc_pageno].addr;
    s_mm->alloc_page_count += npages;
    s_mm->free_page_count  -= npages;

    mutex_unlock(s_mm->mutex);

    return ptr;
}

int
vmem_free(
    unsigned long ptr
    )
{
    unsigned long addr;
    avl_node_t* found;
    page_t* page;
    int pageno, prev_free_pageno, next_free_pageno;
    int prev_size, next_size;
    int merge_page_no, merge_VMEM_PAGE_SIZE, free_VMEM_PAGE_SIZE;

    mutex_lock(s_mm->mutex);

    addr = ptr;

    s_mm->alloc_tree = avltree_remove(s_mm->alloc_tree, &found, MAKE_KEY(addr, 0));
    if (found == NULL) {
        DPRINT("vmem_free: 0x%08x not found\n", addr);
        ASSERT(0);
        mutex_unlock(s_mm->mutex);
        return -1;
    }

    /* find previous free block */
    page = found->page;
    pageno = page->pageno;
    free_VMEM_PAGE_SIZE = page->alloc_pages;
    prev_free_pageno = pageno-1;
    prev_size = -1;
    if (prev_free_pageno >= 0) {
        if (s_mm->page_list[prev_free_pageno].used == 0) {
            prev_free_pageno = s_mm->page_list[prev_free_pageno].first_pageno;
            prev_size = s_mm->page_list[prev_free_pageno].alloc_pages;
        }
    }

    /* find next free block */
    next_free_pageno = pageno + page->alloc_pages;
    next_free_pageno = (next_free_pageno == s_mm->num_pages) ? -1 : next_free_pageno;
    next_size = -1;
    if (next_free_pageno >= 0) {
        if (s_mm->page_list[next_free_pageno].used == 0) {
            next_size = s_mm->page_list[next_free_pageno].alloc_pages;
        }
    }
    P_FREE(found);

    /* merge */
    merge_page_no = page->pageno;
    merge_VMEM_PAGE_SIZE = page->alloc_pages;
    if (prev_size >= 0) {
        s_mm->free_tree = avltree_remove(s_mm->free_tree, &found, MAKE_KEY(prev_size, prev_free_pageno));
        if (found == NULL) {
            ASSERT(0);
            mutex_unlock(s_mm->mutex);
            return -1;
        }
        merge_page_no = found->page->pageno;
        merge_VMEM_PAGE_SIZE += found->page->alloc_pages;
        P_FREE(found);
    }
    if (next_size >= 0) {
        s_mm->free_tree = avltree_remove(s_mm->free_tree, &found, MAKE_KEY(next_size, next_free_pageno));
        if (found == NULL) {
            ASSERT(0);
            mutex_unlock(s_mm->mutex);
            return -1;
        }
        merge_VMEM_PAGE_SIZE += found->page->alloc_pages;
        P_FREE(found);
    }

    page->alloc_pages  = 0;
    page->first_pageno = -1;

    set_blocks_free(merge_page_no, merge_VMEM_PAGE_SIZE);

    s_mm->alloc_page_count -= free_VMEM_PAGE_SIZE;
    s_mm->free_page_count  += free_VMEM_PAGE_SIZE;

    mutex_unlock(s_mm->mutex);

    return 0;
}

int
vmem_get_info(
    vmem_info_t* info
    )
{
    if (info == NULL) {
        return -1;
    }

    info->total_pages = s_mm->num_pages;
    info->alloc_pages = s_mm->alloc_page_count;
    info->free_pages  = s_mm->free_page_count;

    return 0;
}

