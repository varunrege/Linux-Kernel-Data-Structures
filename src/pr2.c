#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/hashtable.h>
#include <linux/radix-tree.h>
#include <linux/xarray.h>
#include <linux/rbtree.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static char *int_str;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("VARUN");
MODULE_DESCRIPTION("LKP Project 2");


module_param(int_str, charp, S_IRUSR | S_IRGRP | S_IROTH);


MODULE_PARM_DESC(int_str, "A comma-separated list of integers");

static LIST_HEAD(mylist);

static DEFINE_HASHTABLE(htable,3);

RADIX_TREE (radix, GFP_KERNEL);

DEFINE_XARRAY (xarray);

struct rb_root rbroot = RB_ROOT;

struct entry {
	int val;
	struct list_head list;
};

struct hlist_entry {
	int val;
	struct hlist_node node;
};

struct rbtree_entry {
	int val;
	struct rb_node rbentry;

};


unsigned long k = 1;
unsigned long k2 = 1;
unsigned long radix_index = 1;
unsigned long xarray_index = 1;


static int store_value(int val)
{
	struct entry* x;
	x = kmalloc(sizeof(struct entry),GFP_KERNEL);
	if(x == NULL)
	{
		return -ENOMEM;
	}
	else
	{
		x -> val = val;
		list_add_tail(&x->list, &mylist);
		return 0;
	}
}

static int store_hlist_value(int val)
{
	struct hlist_entry* h;
	h = kmalloc(sizeof(struct hlist_entry),GFP_KERNEL);
	if(h == NULL)
	{
		return -ENOMEM;
	}
	else
	{
		h->val = val;
		hash_add(htable,&h->node,val);
		return 0;
	}
}

static int store_radix_value(int val)
{
	int* r;
	r = kmalloc(sizeof(int), GFP_KERNEL);
	if(r == NULL)
	{
		return -ENOMEM;
	}
	else
	{
		*r = val;
		radix_tree_insert(&radix,k,r);
		k++;
		return 0;
	}
}

static int store_xarray_value(int val)
{
	int* xa;
	xa = kmalloc(sizeof(int), GFP_KERNEL);
	if(xa == NULL)
	{
		return -ENOMEM;
	}
	else
	{
		*xa = val;
		xa_store(&xarray,k2,xa,GFP_KERNEL);
		k2++;
		return 0;
	}
}

static int store_rbtree_value(int val)
{
	struct rbtree_entry *node3, *anode;
	struct rb_node **cnode, *parent = NULL;

	node3 = kmalloc(sizeof(struct rbtree_entry),GFP_KERNEL);
	if(node3 != NULL)
	{
		node3->val = val;
		cnode = &rbroot.rb_node;
		while(*cnode != NULL)
		{
			parent = *cnode;
			anode = rb_entry(parent, struct rbtree_entry, rbentry);
			if(anode->val < val)
			{
				cnode = &((*cnode)->rb_right);
			}
			else
			{
				cnode = &((*cnode)->rb_left);
			}
		}
		rb_link_node(&node3->rbentry, parent, cnode);
		rb_insert_color(&(node3->rbentry),&rbroot);
	}
	else
	{
		printk(KERN_INFO "No memory to add\n");
		return -ENOMEM;
	}
}


//static void test_linked_list(void)
static void test_linked_list(struct seq_file* proc)
{
	
	struct entry *current1;
	seq_printf(proc,"Linked list:");
	list_for_each_entry(current1,&mylist,list)
	{
		if(current1 != NULL)
		{
			printk(KERN_INFO "Linked list: %d\n", current1->val);
		        seq_printf(proc,"%d,", current1->val);	
		}
	}
	seq_printf(proc,"\n");
}

//static void test_htable(void)
static void test_htable(struct seq_file* proc)
{
	int bkt;
	struct hlist_entry *h1;
	seq_printf(proc,"Hash table:");
	hash_for_each(htable,bkt,h1,node)
	{
		if(h1 != NULL)
		{
			printk(KERN_INFO "Hash table: %d\n",h1->val);
			seq_printf(proc,"%d,",h1->val);
		}
	}
	seq_printf(proc,"\n");
}

//static void test_radix(void)
static void test_radix(struct seq_file* proc)
{
	k = 1;
	int* tmp;
	seq_printf(proc,"Radix tree:");
	while((tmp = radix_tree_lookup(&radix,k)) != NULL)
	{
		printk(KERN_INFO "Radix tree: %d\n",*tmp);
		seq_printf(proc,"%d,",*tmp);
		k++;
	}
	seq_printf(proc,"\n");	
}

//static void test_xarray(void)
static void test_xarray(struct seq_file* proc)
{
	k2 = 1;
	int* tmp2;
	seq_printf(proc,"XArray:");
	while((tmp2 = xa_load(&xarray,k2)) != NULL)
	{
		printk(KERN_INFO "XArray: %d\n",*tmp2);
		seq_printf(proc,"%d,",*tmp2);
		k2++;
	}
	seq_printf(proc,"\n");	
}

//static void test_rbtree(void)
static void test_rbtree(struct seq_file* proc)
{
	struct rb_node *cur;
	struct rbtree_entry* rdnode;
	cur = rb_first(&rbroot);
	seq_printf(proc,"Red-black tree:");
	while(cur)
	{
		rdnode = rb_entry(cur, struct rbtree_entry, rbentry);
		printk(KERN_INFO "Red-black tree: %d\n",rdnode->val);
		seq_printf(proc,"%d,",rdnode->val);
		cur = rb_next(cur);
	}
	seq_printf(proc,"\n");
}

static void destroy_linked_list_and_free(void)
{	
	struct entry* current1;
	struct entry* next;
	printk(KERN_INFO "Destroying Linked list\n");
	list_for_each_entry_safe(current1,next,&mylist,list)
	{
		list_del(&current1->list);
		kfree(current1);
	}
}

static void destroy_htable_and_free(void)
{
	struct hlist_entry *hlist_node;
	struct hlist_node *hlist_node2;
	int bkt;
	printk(KERN_INFO "Destroying Hash table\n");
	hash_for_each_safe(htable,bkt,hlist_node2,hlist_node,node)
	{
		hash_del(&hlist_node->node);
		kfree(hlist_node);
	}
}

static void destroy_radix_and_free(void)
{
	int* tmp2;
	radix_index = 1;
	printk(KERN_INFO "Destroying Radix tree\n");
	while((tmp2 = radix_tree_delete(&radix,radix_index))!= NULL)
	{
		kfree(tmp2);
		radix_index++;
	}
}
static void destroy_xarray_and_free(void)
{
	int* tmp3;
	xarray_index = 1;
	printk(KERN_INFO "Destroying XArray\n");
	while((tmp3 = xa_erase(&xarray,xarray_index)) != NULL)
	{
		kfree(tmp3);
		xarray_index++;
	}
}
static void destroy_rbtree_and_free(void)
{
	struct rbtree_entry *rbdnode;
	struct rb_node *rbdnode2;
	printk(KERN_INFO "Destroying red-black tree\n");
	rbdnode2 = rb_last(&rbroot);
	while(rbdnode2)
	{
		rbdnode = rb_entry(rbdnode2,struct rbtree_entry,rbentry);
		rbdnode2 = rb_prev(rbdnode2);
		rb_erase(&rbdnode->rbentry,&rbroot);
		kfree(rbdnode);
	}
}
static int parse_params(void)
{
	int val, err = 0;
	char *p, *orig, *params;


	params = kstrdup(int_str, GFP_KERNEL);
	if (!params)
		return -ENOMEM;
	orig = params;

	while ((p = strsep(&params, ",")) != NULL) {
		if (!*p)
			continue;
		err = kstrtoint(p, 0, &val);
		if (err)
			break;

		
		err = store_value(val);
		store_hlist_value(val);
		store_radix_value(val);
		store_xarray_value(val);
		store_rbtree_value(val);
		if (err)
			break;
	}

	
	kfree(orig);
	return err;
}

static void run_tests(struct seq_file* proc)
{
	test_linked_list(proc);
	test_htable(proc);
	test_radix(proc);
	test_xarray(proc);
	test_rbtree(proc);
}

static void cleanup(void)
{
	printk(KERN_INFO "Cleaning up...\n");

	destroy_linked_list_and_free();
	destroy_htable_and_free();
	destroy_radix_and_free();
	destroy_xarray_and_free();
	destroy_rbtree_and_free();
}


static int proj_init(struct seq_file* proc, void *v)
{
	int err = 0;

	if (!int_str) {
		printk(KERN_INFO "Missing \'int_str\' parameter, exiting\n");
		return -1;
	}

	err = parse_params();
	if (err)
		goto out;


	run_tests(proc);
out:
	cleanup();
	return err;
}

static int proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proj_init, NULL);
}

static const struct file_operations proc_fops = {
	  .owner = THIS_MODULE,
	    .open = proc_open,
	      .read = seq_read,
	        .llseek = seq_lseek,
		  .release = single_release,
};

static int __init pr2_init(void) {
	  proc_create("proj2", 0, NULL, &proc_fops);
	    return 0;
}

static void __exit pr2_exit(void) {
	  remove_proc_entry("proj2", NULL);
}

module_init(pr2_init);


module_exit(pr2_exit);
