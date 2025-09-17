#include "kv_store.h"
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAPACITY 1024

static size_t hash_function(const char *key, size_t capacity) {
    size_t hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % capacity;
}

static HashEntry *create_entry(const char *key, const char *value) {
    HashEntry *entry = malloc(sizeof(HashEntry));
    if (!entry) return NULL;
    entry->key = strdup(key);
    entry->value = strdup(value);
    entry->next = NULL;
    if (!entry->key || !entry->value) {
        free(entry->key);
        free(entry->value);
        free(entry);
        return NULL;
    }
    return entry;
}

static void free_entry(HashEntry *entry) {
    if (entry) {
        free(entry->key);
        free(entry->value);
        free(entry);
    }
}

KVStore *kv_store_create(size_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = DEFAULT_CAPACITY;
    }
    KVStore *store = malloc(sizeof(KVStore));
    if (!store) return NULL;
    store->buckets = calloc(initial_capacity, sizeof(HashEntry *));
    if (!store->buckets) {
        free(store);
        return NULL;
    }
    store->capacity = initial_capacity;
    store->size = 0;
    return store;
}

void kv_store_destroy(KVStore *store) {
    if (!store) return;
    for (size_t i = 0; i < store->capacity; i++) {
        HashEntry *entry = store->buckets[i];
        while (entry) {
            HashEntry *next = entry->next;
            free_entry(entry);
            entry = next;
        }
    }
    free(store->buckets);
    free(store);
}

bool kv_set(KVStore *store, const char *key, const char *value) {
    if (!store || !key || !value) return false;
    size_t index = hash_function(key, store->capacity);
    HashEntry *entry = store->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            char *new_value = strdup(value);
            if (!new_value) return false;
            free(entry->value);
            entry->value = new_value;
            return true;
        }
        entry = entry->next;
    }
    HashEntry *new_entry = create_entry(key, value);
    if (!new_entry) return false;
    new_entry->next = store->buckets[index];
    store->buckets[index] = new_entry;
    store->size++;
    return true;
}

char *kv_get(KVStore *store, const char *key) {
    if (!store || !key) return NULL;
    size_t index = hash_function(key, store->capacity);
    HashEntry *entry = store->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return strdup(entry->value);
        }
        entry = entry->next;
    }
    return NULL;
}

bool kv_delete(KVStore *store, const char *key) {
    if (!store || !key) return false;
    size_t index = hash_function(key, store->capacity);
    HashEntry *entry = store->buckets[index];
    HashEntry *prev = NULL;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (prev) {
                prev->next = entry->next;
            } else {
                store->buckets[index] = entry->next;
            }
            free_entry(entry);
            store->size--;
            return true;
        }
        prev = entry;
        entry = entry->next;
    }
    return false;
}

size_t kv_size(KVStore *store) {
    return store ? store->size : 0;
}
