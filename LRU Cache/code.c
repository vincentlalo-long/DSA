#include <unordered_map>
#include <cstdio>
#include <cstdlib>

#define MAX_LEN 256

typedef struct Node {
    int key;
    char value[MAX_LEN];
    struct Node* prev;
    struct Node* next;
} Node;

typedef struct MFUCache {
    int capacity;
    int size;
    std::unordered_map<int, Node*> cacheMap;
    std::unordered_map<int, int> freqMap;
    Node* head;
    Node* tail;
} MFUCache;

// Khởi tạo cache
MFUCache* initCache(int capacity){
    MFUCache* cache = (MFUCache*)malloc(sizeof(MFUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = cache->tail = NULL;
    return cache;
}

// In cache debug
void printCache(MFUCache* cache) {
    Node* cur = cache->head;
    printf("Cache: ");
    while (cur) {
        printf("(%d,%s) ", cur->key, cur->value);
        cur = cur->next;
    }
    printf("\n");
}

// Lấy giá trị, tăng tần suất
char* get(MFUCache* cache, int key) {
    auto it = cache->cacheMap.find(key);
    if (it == cache->cacheMap.end()) {
        return NULL;
    }
    cache->freqMap[key]++;
    return it->second->value;
}

// Thêm hoặc cập nhật cache
void put(MFUCache* cache, int key, const char* value) {
    if (cache->cacheMap.find(key) != cache->cacheMap.end()) {
        Node* node = cache->cacheMap[key];
        snprintf(node->value, MAX_LEN, "%s", value);
        cache->freqMap[key]++;
        return;
    }

    if (cache->size >= cache->capacity) {
        int maxFreq = -1;
        Node* nodeToRemove = NULL;
        Node* cur = cache->tail;

        while (cur) {
            int freq = cache->freqMap[cur->key];
            if (freq > maxFreq) {
                maxFreq = freq;
                nodeToRemove = cur;
            }
            cur = cur->prev;
        }

        if (nodeToRemove == NULL) {
            printf("Error: No node to remove found!\n");
            return;
        }

        // Xóa node khỏi danh sách
        if (nodeToRemove->prev)
            nodeToRemove->prev->next = nodeToRemove->next;
        if (nodeToRemove->next)
            nodeToRemove->next->prev = nodeToRemove->prev;
        if (cache->head == nodeToRemove)
            cache->head = nodeToRemove->next;
        if (cache->tail == nodeToRemove)
            cache->tail = nodeToRemove->prev;

        // Xóa khỏi map và giải phóng bộ nhớ
        cache->cacheMap.erase(nodeToRemove->key);
        cache->freqMap.erase(nodeToRemove->key);
        free(nodeToRemove);
        cache->size--;
    }

    // Thêm node mới vào đầu danh sách
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

    cache->cacheMap[key] = newNode;
    cache->freqMap[key] = 1;
    cache->size++;
}

int main() {
    MFUCache* cache = initCache(3);

    put(cache, 1, "Google");
    put(cache, 2, "youtube");
    put(cache, 3, "Zalo");
    printCache(cache);

    get(cache, 2);
    get(cache, 2);

    get(cache, 1);
    get(cache, 1);
    get(cache, 1);

    get(cache, 3);

    put(cache, 4, "Chat GPT");
    printCache(cache);

    char* r2 = get(cache, 2);
    char* r1 = get(cache, 1);
    printf("get(2) = %s\n", r2 ? r2 : "(null)");
    printf("get(1) = %s\n", r1 ? r1 : "(null)");

    put(cache, 5, "five");
    printCache(cache);

    return 0;
}
