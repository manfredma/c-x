#ifndef KV_STORE_H
#define KV_STORE_H

#include <stddef.h>
#include <stdbool.h>

// 哈希表条目结构
typedef struct HashEntry {
    char *key;
    char *value;
    struct HashEntry *next; // 用于解决哈希冲突（链地址法）
} HashEntry;

// KV 存储结构
typedef struct {
    HashEntry **buckets;
    size_t capacity;
    size_t size;
} KVStore;

// KV 存储接口
KVStore* kv_store_create(size_t initial_capacity);
void kv_store_destroy(KVStore *store);
bool kv_set(KVStore *store, const char *key, const char *value);
char* kv_get(KVStore *store, const char *key);
bool kv_delete(KVStore *store, const char *key);
size_t kv_size(KVStore *store);

#endif // KV_STORE_H

