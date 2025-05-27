#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 256
#define HASH_SIZE 101

// --------- Node trong Cache (Doubly Linked List) ---------
typedef struct Node {
    int key;
    char value[MAX_LEN];
    struct Node* prev;
    struct Node* next;
} Node;

// --------- Hash Table Entry (key → Node*) ---------
typedef struct MapEntry {
    int key;
    Node* node;
    struct MapEntry* next;
} MapEntry;

typedef struct {
    MapEntry* table[HASH_SIZE];
} NodeMap;

// --------- LRU Cache Struct ---------
typedef struct {
    int capacity;
    int size;
    Node* head;
    Node* tail;
    NodeMap* cacheMap;
} LRUCache;

// --------- Hash Function ---------
int hashFunc(int key) {
    return key % HASH_SIZE;
}

// --------- NodeMap Functions ---------
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

// --------- LRU Cache Functions ---------
LRUCache* initLRUCache(int capacity) {
    LRUCache* cache = (LRUCache*)malloc(sizeof(LRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = cache->tail = NULL;
    cache->cacheMap = createNodeMap();
    return cache;
}

void moveToFront(LRUCache* cache, Node* node) {
    if (cache->head == node) return;

    // Remove node from current position
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;

    if (cache->tail == node)
        cache->tail = node->prev;

    // Move node to head
    node->prev = NULL;
    node->next = cache->head;
    if (cache->head)
        cache->head->prev = node;
    cache->head = node;
    if (!cache->tail)
        cache->tail = node;
}

char* lruGet(LRUCache* cache, int key) {
    Node* node = nodeMapGet(cache->cacheMap, key);
    if (!node) return NULL;
    moveToFront(cache, node);
    return node->value;
}

void lruPut(LRUCache* cache, int key, const char* value) {
    Node* node = nodeMapGet(cache->cacheMap, key);
    if (node) {
        snprintf(node->value, MAX_LEN, "%s", value);
        moveToFront(cache, node);
        return;
    }

    if (cache->size >= cache->capacity) {
        Node* nodeToRemove = cache->tail;
        if (nodeToRemove) {
            if (nodeToRemove->prev)
                nodeToRemove->prev->next = NULL;
            cache->tail = nodeToRemove->prev;
            if (cache->head == nodeToRemove)
                cache->head = NULL;
            nodeMapRemove(cache->cacheMap, nodeToRemove->key);
            free(nodeToRemove);
            cache->size--;
        }
    }

    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->key = key;
    snprintf(newNode->value, MAX_LEN, "%s", value);
    newNode->prev = NULL;
    newNode->next = cache->head;
    if (cache->head) cache->head->prev = newNode;
    cache->head = newNode;
    if (!cache->tail)
        cache->tail = newNode;

    nodeMapPut(cache->cacheMap, key, newNode);
    cache->size++;
}

void printLRUCache(LRUCache* cache) {
    Node* cur = cache->head;
    printf("LRU Cache: ");
    while (cur) {
        printf("(%d,%s) ", cur->key, cur->value);
        cur = cur->next;
    }
    printf("\n");
}

// --------- Main Test Function ---------
int main() {
    printf("=== LRU Cache Test ===\n");
    LRUCache* cache = initLRUCache(3);

    lruPut(cache, 1, "Google");
    lruPut(cache, 2, "YouTube");
    lruPut(cache, 3, "Zalo");
    printLRUCache(cache);  // (3,Zalo) (2,YouTube) (1,Google)

    lruGet(cache, 1);  // Truy cập 1 → Lên đầu
    printLRUCache(cache);  // (1,Google) (3,Zalo) (2,YouTube)

    lruPut(cache, 4, "ChatGPT");  // Xóa (2,YouTube)
    printLRUCache(cache);  // (4,ChatGPT) (1,Google) (3,Zalo)

    printf("get(2) = %s\n", lruGet(cache, 2)); // NULL
    printf("get(3) = %s\n", lruGet(cache, 3)); // Zalo

    lruPut(cache, 5, "Facebook");  // Xóa (1,Google)
    printLRUCache(cache);  // (5,Facebook) (3,Zalo) (4,ChatGPT)

    return 0;
}
