//Up coming
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

// Hàm khởi tạo MFU cache 
MFUCache* initCache(int capacity){
    MFUCache* cache = (MFUCache*)malloc(sizeof(MFUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = cache->tail = NULL;
    return cache;
}

// In cache (Debug)
void printCache(MFUCache* cache) {
    Node* cur = cache->head;
    printf("Cache: ");
    while (cur) {
        printf("(%d,%s) ", cur->key, cur->value);
        cur = cur->next;
    }
    printf("\n");
}

// Ham get 
char* get(MFUCache* cache, int key) {
    auto it = cache->cacheMap.find(key);
    if (it == cache->cacheMap.end()) {
        // Không tìm thấy key
        return NULL;
    }

    // Tăng tần suất truy cập
    cache->freqMap[key]++;

    // Trả về giá trị
    return it->second->value;
}
void put(MFUCache* cache, int key, const char* value) {
    // Nếu key đã tồn tại → cập nhật value và tăng tần suất
    if (cache->cacheMap.find(key) != cache->cacheMap.end()) {
        Node* node = cache->cacheMap[key];
        snprintf(node->value, MAX_LEN, "%s", value);
        cache->freqMap[key]++;
        return;
    }

    // Nếu cache đầy → tìm và xóa phần tử MFU (theo FIFO nếu cần)
    if (cache->size >= cache->capacity) {
        int maxFreq = -1;
        Node* nodeToRemove = NULL;
        Node* cur = cache->tail;  // duyệt từ node cũ nhất

        while (cur) {
            int freq = cache->freqMap[cur->key];
            if (freq > maxFreq) {
                maxFreq = freq;
                nodeToRemove = cur;
            }
            cur = cur->prev;
        }

        // Xóa node khỏi danh sách liên kết đôi
        if (nodeToRemove->prev)
            nodeToRemove->prev->next = nodeToRemove->next;
        if (nodeToRemove->next)
            nodeToRemove->next->prev = nodeToRemove->prev;
        if (cache->head == nodeToRemove)
            cache->head = nodeToRemove->next;
        if (cache->tail == nodeToRemove)
            cache->tail = nodeToRemove->prev;

        // Xóa khỏi map
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
    // Khởi tạo cache có capacity = 3
    MFUCache* cache = initCache(3);

    // Thêm một số phần tử
    put(cache, 1, "one");
    put(cache, 2, "two");
    put(cache, 3, "three");
    printCache(cache);  // Cache: (3,three) (2,two) (1,one)

    // Truy cập 2 lần key = 2 → tăng tần suất
    get(cache, 2);
    get(cache, 2);

    // Truy cập 3 lần key = 1 → tần suất cao hơn key 2
    get(cache, 1);
    get(cache, 1);
    get(cache, 1);

    // Truy cập key = 3 1 lần
    get(cache, 3);

    // Cache hiện tại:
    // key=1: freq=4
    // key=2: freq=3
    // key=3: freq=2

    // Thêm phần tử mới → cache đầy → xóa key=1 (tần suất cao nhất)
    put(cache, 4, "four");
    printCache(cache);  // Expect: key=4 xuất hiện, key=1 bị xóa

    // Thử lấy giá trị từ cache
    printf("get(2) = %s\n", get(cache, 2)); // Expect: two
    printf("get(1) = %s\n", get(cache, 1)); // Expect: (null)

    // Thêm 1 key mới nữa → xóa key=2 (vì sau đó là freq cao nhất)
    put(cache, 5, "five");
    printCache(cache);

    return 0;
}
