#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 256
#define HASH_SIZE 101

typedef struct Node {
    int key;
    char value[MAX_LEN];
    struct Node* prev;
    struct Node* next;
} Node;

typedef struct MapEntry {
    int key;
    Node* node;
    struct MapEntry* next;
} MapEntry;

typedef struct {
    MapEntry* table[HASH_SIZE];
} NodeMap;

typedef struct {
    int capacity;
    int size;
    Node* head;
    Node* tail;
    NodeMap* cacheMap;
} MRUCache;

// ---------- Hash Function ----------
int hashFunc(int key) {
    return key % HASH_SIZE;
}

// ---------- NodeMap Functions ----------
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

// ---------- Cache Core ----------
MRUCache* initCache(int capacity) {
    MRUCache* cache = (MRUCache*)malloc(sizeof(MRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = cache->tail = NULL;
    cache->cacheMap = createNodeMap();
    return cache;
}

void moveToFront(MRUCache* cache, Node* node) {
    if (cache->head == node) return;

    // Detach node
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    if (cache->tail == node) cache->tail = node->prev;

    // Move to front
    node->prev = NULL;
    node->next = cache->head;
    if (cache->head) cache->head->prev = node;
    cache->head = node;
    if (!cache->tail)
        cache->tail = node;
}

char* get(MRUCache* cache, int key) {
    Node* node = nodeMapGet(cache->cacheMap, key);
    if (!node) return NULL;
    moveToFront(cache, node);
    return node->value;
}

void removeNode(MRUCache* cache, Node* node) {
    if (!node) return;
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    if (cache->head == node) cache->head = node->next;
    if (cache->tail == node) cache->tail = node->prev;
}

void put(MRUCache* cache, int key, const char* value) {
    Node* node = nodeMapGet(cache->cacheMap, key);
    if (node) {
        snprintf(node->value, MAX_LEN, "%s", value);
        moveToFront(cache, node);
        return;
    }

    if (cache->size >= cache->capacity) {
        // MRU: Remove head (most recently used)
        Node* nodeToRemove = cache->head;
        if (nodeToRemove) {
            removeNode(cache, nodeToRemove);
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
    if (cache->head)
        cache->head->prev = newNode;
    cache->head = newNode;
    if (!cache->tail)
        cache->tail = newNode;

    nodeMapPut(cache->cacheMap, key, newNode);
    cache->size++;
}

void printCache(MRUCache* cache) {
    Node* cur = cache->head;
    printf("Cache: ");
    while (cur) {
        printf("(%d,%s) ", cur->key, cur->value);
        cur = cur->next;
    }
    printf("\n");
}

// ---------- Main ----------
int main() {
    MRUCache* cache = initCache(3);

    put(cache, 1, "Google");
    put(cache, 2, "YouTube");
    put(cache, 3, "Zalo");
    printCache(cache);

    get(cache, 2); // 2 lên đầu
    get(cache, 1); // 1 lên đầu

    put(cache, 4, "ChatGPT"); // Xóa key=1 (MRU)
    printCache(cache);

    printf("get(2) = %s\n", get(cache, 2));
    printf("get(1) = %s\n", get(cache, 1)); // null nếu bị xóa

    put(cache, 5, "Facebook"); // Xóa key=2 (vì vừa dùng lại)
    printCache(cache);

    return 0;
}
