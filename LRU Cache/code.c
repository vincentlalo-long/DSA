#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 256
#define HASH_SIZE 101  // Số lượng bucket cho bảng băm

// --------- Struct Node (mỗi phần tử trong Cache) ---------
typedef struct Node {
    int key;
    char value[MAX_LEN];
    struct Node* prev;
    struct Node* next;
} Node;

// --------- Bảng băm ánh xạ key → Node* ---------
typedef struct MapEntry {
    int key;
    Node* node;
    struct MapEntry* next;
} MapEntry;

typedef struct {
    MapEntry* table[HASH_SIZE];
} NodeMap;

// --------- Bảng băm ánh xạ key → frequency ---------
typedef struct FreqEntry {
    int key;
    int freq;
    struct FreqEntry* next;
} FreqEntry;

typedef struct {
    FreqEntry* table[HASH_SIZE];
} FreqMap;

// --------- MFU Cache Struct ---------
typedef struct {
    int capacity;
    int size;
    Node* head;
    Node* tail;
    NodeMap* cacheMap;
    FreqMap* freqMap;
} MFUCache;

// --------- Hàm băm ---------
int hashFunc(int key) {
    return key % HASH_SIZE;
}

// ---------------- Map Node* ----------------
NodeMap* createNodeMap() {
    NodeMap* map = (NodeMap*)malloc(sizeof(NodeMap));
    memset(map->table, 0, sizeof(map->table));
    return map;
}

void nodeMapPut(NodeMap* map, int key, Node* node) {
    int h = hashFunc(key);
    MapEntry* entry = map->table[h];
    while (entry) {
        if (entry->key == key) {
            entry->node = node;
            return;
        }
        entry = entry->next;
    }
    // Không tìm thấy, thêm mới
    MapEntry* newEntry = (MapEntry*)malloc(sizeof(MapEntry));
    newEntry->key = key;
    newEntry->node = node;
    newEntry->next = map->table[h];
    map->table[h] = newEntry;
}

Node* nodeMapGet(NodeMap* map, int key) {
    int h = hashFunc(key);
    MapEntry* entry = map->table[h];
    while (entry) {
        if (entry->key == key)
            return entry->node;
        entry = entry->next;
    }
    return NULL;
}

void nodeMapRemove(NodeMap* map, int key) {
    int h = hashFunc(key);
    MapEntry* prev = NULL;
    MapEntry* curr = map->table[h];
    while (curr) {
        if (curr->key == key) {
            if (prev)
                prev->next = curr->next;
            else
                map->table[h] = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

// ---------------- Map Freq ----------------
FreqMap* createFreqMap() {
    FreqMap* map = (FreqMap*)malloc(sizeof(FreqMap));
    memset(map->table, 0, sizeof(map->table));
    return map;
}

int freqMapGet(FreqMap* map, int key) {
    int h = hashFunc(key);
    FreqEntry* entry = map->table[h];
    while (entry) {
        if (entry->key == key)
            return entry->freq;
        entry = entry->next;
    }
    return 0;
}

void freqMapInc(FreqMap* map, int key) {
    int h = hashFunc(key);
    FreqEntry* entry = map->table[h];
    while (entry) {
        if (entry->key == key) {
            entry->freq++;
            return;
        }
        entry = entry->next;
    }
    // Không có thì thêm mới
    FreqEntry* newEntry = (FreqEntry*)malloc(sizeof(FreqEntry));
    newEntry->key = key;
    newEntry->freq = 1;
    newEntry->next = map->table[h];
    map->table[h] = newEntry;
}

void freqMapRemove(FreqMap* map, int key) {
    int h = hashFunc(key);
    FreqEntry* prev = NULL;
    FreqEntry* curr = map->table[h];
    while (curr) {
        if (curr->key == key) {
            if (prev)
                prev->next = curr->next;
            else
                map->table[h] = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

// ---------------- MFU Cache ----------------
MFUCache* initCache(int capacity) {
    MFUCache* cache = (MFUCache*)malloc(sizeof(MFUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = cache->tail = NULL;
    cache->cacheMap = createNodeMap();
    cache->freqMap = createFreqMap();
    return cache;
}

char* get(MFUCache* cache, int key) {
    Node* node = nodeMapGet(cache->cacheMap, key);
    if (!node) return NULL;
    freqMapInc(cache->freqMap, key);
    return node->value;
}

void removeNode(MFUCache* cache, Node* node) {
    if (!node) return;
    if (node->prev)
        node->prev->next = node->next;
    if (node->next)
        node->next->prev = node->prev;
    if (cache->head == node)
        cache->head = node->next;
    if (cache->tail == node)
        cache->tail = node->prev;
}

void put(MFUCache* cache, int key, const char* value) {
    Node* node = nodeMapGet(cache->cacheMap, key);
    if (node) {
        snprintf(node->value, MAX_LEN, "%s", value);
        freqMapInc(cache->freqMap, key);
        return;
    }

    if (cache->size >= cache->capacity) {
        Node* cur = cache->tail;
        Node* nodeToRemove = NULL;
        int maxFreq = -1;

        while (cur) {
            int f = freqMapGet(cache->freqMap, cur->key);
            if (f > maxFreq) {
                maxFreq = f;
                nodeToRemove = cur;
            }
            cur = cur->prev;
        }

        if (nodeToRemove) {
            removeNode(cache, nodeToRemove);
            nodeMapRemove(cache->cacheMap, nodeToRemove->key);
            freqMapRemove(cache->freqMap, nodeToRemove->key);
            free(nodeToRemove);
            cache->size--;
        }
    }

    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->key = key;
    snprintf(newNode->value, MAX_LEN, "%s", value);
    newNode->prev = NULL;
    newNode->next = cache->head;
    if (cache->head)
        cache->head->prev = newNode;
    cache->head = newNode;
    if (!cache->tail)
        cache->tail = newNode;

    nodeMapPut(cache->cacheMap, key, newNode);
    freqMapInc(cache->freqMap, key);
    cache->size++;
}

void printCache(MFUCache* cache) {
    Node* cur = cache->head;
    printf("Cache: ");
    while (cur) {
        printf("(%d,%s) ", cur->key, cur->value);
        cur = cur->next;
    }
    printf("\n");
}

// ---------------- Main (test) ----------------
int main() {
    MFUCache* cache = initCache(3);

    put(cache, 1, "Google");
    put(cache, 2, "YouTube");
    put(cache, 3, "Zalo");
    printCache(cache);

    get(cache, 2);
    get(cache, 2);
    get(cache, 1);
    get(cache, 1);
    get(cache, 1);
    get(cache, 3);

    put(cache, 4, "ChatGPT"); // Xóa key=1 (tần suất cao nhất)
    printCache(cache);

    printf("get(2) = %s\n", get(cache, 2));
    printf("get(1) = %s\n", get(cache, 1)); // null

    put(cache, 5, "Facebook"); // Xóa key=2
    printCache(cache);

    return 0;
}
