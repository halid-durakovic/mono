/*
 * mono-conc-hashtable.h: A mostly concurrent hashtable
 *
 * Author:
 *	Rodrigo Kumpera (kumpera@gmail.com)
 *
 * (C) 2014 Xamarin
 */

#include "mono-conc-hashtable.h"
#include <mono/utils/hazard-pointer.h>

/* Configuration knobs. */

#define INITIAL_SIZE 32
#define LOAD_FACTOR 0.75f

typedef struct {
	gpointer key;
	gpointer value;
} key_value_pair;

typedef struct {
	int table_size;
	key_value_pair *kvs;
} conc_table;

struct _MonoConcurrentHashTable {
	volatile conc_table *table; /* goes to HP0 */
	GHashFunc hash_func;
	GEqualFunc equal_func;
	mono_mutex_t *mutex;
	int element_count;
	int overflow_count;
};

static conc_table*
conc_table_new (int size)
{
	conc_table *res = g_new (conc_table, 1);
	res->table_size = size;
	res->kvs = g_new0 (key_value_pair, size);
	return res;
}

static void
conc_table_free (gpointer ptr)
{
	conc_table *table = ptr;
	g_free (table->kvs);
	g_free (table);
}

static void
conc_table_lf_free (conc_table *table)
{
	mono_thread_hazardous_free_or_queue (table, conc_table_free, TRUE, FALSE);
}


/*
A common problem with power of two hashtables is that it leads of bad clustering when dealing
with aligned numbers.

The solution here is to mix the bits from two primes plus the hash itself, it produces a better spread
than just the numbers.
*/

static MONO_ALWAYS_INLINE int 
mix_hash (int hash)
{
	return ((hash * 215497) >> 16) ^ (hash * 1823231) + hash;
}

static MONO_ALWAYS_INLINE void
insert_one_local (conc_table *table, GHashFunc hash_func, gpointer key, gpointer value)
{
	key_value_pair *kvs = table->kvs;
	int table_mask = table->table_size - 1;
	int hash = mix_hash (hash_func (key));
	int i = hash & table_mask;

	while (table->kvs [i].key)
		i = (i + 1) & table_mask;

	kvs [i].key = key;
	kvs [i].value = value;
}

/* LOCKING: Must be called holding hash_table->mutex */
static void
expand_table (MonoConcurrentHashTable *hash_table)
{
	conc_table *old_table = (conc_table*)hash_table->table;
	conc_table *new_table = conc_table_new (old_table->table_size * 2);
	int i;

	for (i = 0; i < old_table->table_size; ++i) {
		if (old_table->kvs [i].key)
			insert_one_local (new_table, hash_table->hash_func, old_table->kvs [i].key, old_table->kvs [i].value);
	}
	mono_memory_barrier ();
	hash_table->table = new_table;
	hash_table->overflow_count = (int)(new_table->table_size * LOAD_FACTOR);
	conc_table_lf_free (old_table);
}


MonoConcurrentHashTable*
mono_conc_hashtable_new (mono_mutex_t *mutex, GHashFunc hash_func, GEqualFunc key_equal_func)
{
	MonoConcurrentHashTable *res = g_new0 (MonoConcurrentHashTable, 1);
	res->mutex = mutex;
	res->hash_func = hash_func ? hash_func : g_direct_hash;
	res->equal_func = key_equal_func;
	// res->equal_func = g_direct_equal;
	res->table = conc_table_new (INITIAL_SIZE);
	res->element_count = 0;
	res->overflow_count = (int)(INITIAL_SIZE * LOAD_FACTOR);	
	return res;
}

void
mono_conc_hashtable_destroy (MonoConcurrentHashTable *hash_table)
{
	conc_table_free ((gpointer)hash_table->table);
	g_free (hash_table);
}

gpointer
mono_conc_hashtable_lookup (MonoConcurrentHashTable *hash_table, gpointer key)
{
	MonoThreadHazardPointers* hp;
	conc_table *table;
	int hash, i, table_mask;
	key_value_pair *kvs;
	hash = mix_hash (hash_table->hash_func (key));
	hp = mono_hazard_pointer_get ();

retry:
	table = get_hazardous_pointer ((gpointer volatile*)&hash_table->table, hp, 0);
	table_mask = table->table_size - 1;
	kvs = table->kvs;
	i = hash & table_mask;

	if (G_LIKELY (!hash_table->equal_func)) {
		while (kvs [i].key) {
			if (key == kvs [i].key) {
				gpointer value;
				/* The read of keys must happen before the read of values */
				mono_memory_barrier ();
				value = kvs [i].value;
				/* FIXME check for NULL if we add suppport for removal */
				mono_hazard_pointer_clear (hp, 0);
				return value;
			}
			i = (i + 1) & table_mask;
		}
	} else {
		GEqualFunc equal = hash_table->equal_func;

		while (kvs [i].key) {
			if (equal (key, kvs [i].key)) {
				gpointer value;
				/* The read of keys must happen before the read of values */
				mono_memory_barrier ();
				value = kvs [i].value;
				/* FIXME check for NULL if we add suppport for removal */
				mono_hazard_pointer_clear (hp, 0);
				return value;
			}
			i = (i + 1) & table_mask;
		}
	}

	/* The table might have expanded and the value is now on the newer table */
	mono_memory_barrier ();
	if (hash_table->table != table)
		goto retry;

	mono_hazard_pointer_clear (hp, 0);
	return NULL;
}

gboolean
mono_conc_hashtable_insert (MonoConcurrentHashTable *hash_table, gpointer key, gpointer value)
{
	conc_table *table;
	key_value_pair *kvs;
	int hash, i, table_mask;

	hash = mix_hash (hash_table->hash_func (key));

	mono_mutex_lock (hash_table->mutex);

	if (hash_table->element_count >= hash_table->overflow_count)
		expand_table (hash_table);

	table = (conc_table*)hash_table->table;
	kvs = table->kvs;
	table_mask = table->table_size - 1;
	i = hash & table_mask;

	if (!hash_table->equal_func) {
		for (;;) {
			if (!kvs [i].key) {
				kvs [i].value = value;
				/* The write to values must happen after the write to keys */
				mono_memory_barrier (); 
				kvs [i].key = key;
				++hash_table->element_count;
				mono_mutex_unlock (hash_table->mutex);
				return TRUE;
			}
			if (key == kvs [i].key) {
				mono_mutex_unlock (hash_table->mutex);
				return FALSE;
			}
			i = (i + 1) & table_mask;
		}
	} else {
		GEqualFunc equal = hash_table->equal_func;
		for (;;) {
			if (!kvs [i].key) {
				kvs [i].value = value;
				/* The write to values must happen after the write to keys */
				mono_memory_barrier (); 
				kvs [i].key = key;
				++hash_table->element_count;
				mono_mutex_unlock (hash_table->mutex);
				return TRUE;
			}
			if (equal (key, kvs [i].key)) {
				mono_mutex_unlock (hash_table->mutex);
				return FALSE;
			}
			i = (i + 1) & table_mask;
		}
	}
}


















