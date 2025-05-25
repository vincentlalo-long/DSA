//Up coming :ĐĐ
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 1024

// Cau truc luu thuoc tinh cua the
typedef struct Attribute {
    char* name;            // ten thuoc tinh
    char* value;           // gia tri thuoc tinh
    struct Attribute* next;
} Attribute;

// Cau truc mot nut trong cay XML
typedef struct TreeNode {
    char* tag_name;        // ten the (VD: book, title, ...)
    Attribute* attributes; // danh sach thuoc tinh
    char* text;            // noi dung trong the
    struct TreeNode* first_child;   // con dau tien
    struct TreeNode* next_sibling;  // anh em ben phai
    struct TreeNode* parent;        // cha
} TreeNode;

// Cau truc ngan xep de luu cac nut trong khi doc the
typedef struct StackNode {
    TreeNode* node;
    struct StackNode* next;
} StackNode;

typedef struct Stack {
    StackNode* top;
} Stack;

// Khoi tao ngan xep
void initStack(Stack* stack) {
    stack->top = NULL;
}

// Kiem tra ngan xep rong
int isEmpty(Stack* stack) {
    return stack->top == NULL;
}

// Day nut vao ngan xep
void push(Stack* stack, TreeNode* node) {
    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
    newNode->node = node;
    newNode->next = stack->top;
    stack->top = newNode;
}

// Lay nut o dinh ngan xep ra
TreeNode* pop(Stack* stack) {
    if (isEmpty(stack)) return NULL;
    StackNode* temp = stack->top;
    TreeNode* node = temp->node;
    stack->top = temp->next;
    free(temp);
    return node;
}

// Xem nut o dinh ngan xep ma khong lay ra
TreeNode* peek(Stack* stack) {
    return isEmpty(stack) ? NULL : stack->top->node;
}

// Ham loai bo khoang trang dau/cuoi
char* trim(char* str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;

    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

// Tao node moi trong cay voi ten the
TreeNode* createNode(const char* tag_name) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->tag_name = strdup(tag_name);
    node->attributes = NULL;
    node->text = NULL;
    node->first_child = NULL;
    node->next_sibling = NULL;
    node->parent = NULL;
    return node;
}

// Tao thuoc tinh moi
Attribute* createAttribute(const char* name, const char* value) {
    Attribute* attr = (Attribute*)malloc(sizeof(Attribute));
    attr->name = strdup(name);
    attr->value = strdup(value);
    attr->next = NULL;
    return attr;
}

// Them thuoc tinh vao node
void addAttribute(TreeNode* node, const char* name, const char* value) {
    Attribute* attr = createAttribute(name, value);
    attr->next = node->attributes;
    node->attributes = attr;
}

// Phan tich chuoi thuoc tinh trong the
void parseAttributes(TreeNode* node, char* attr_str) {
    char name[256], value[256];
    while (*attr_str) {
        while (isspace(*attr_str)) attr_str++;
        if (sscanf(attr_str, "%[^= \t\r\n]=\"%[^\"]\"", name, value) == 2) {
            addAttribute(node, name, value);
            char* next = strstr(attr_str, "\"");
            next = strstr(next + 1, "\"");
            if (next) attr_str = next + 1;
        } else {
            break;
        }
    }
}

// Doc file XML va xay dung cay, dong thoi kiem tra the hop le
TreeNode* readXML(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Khong the mo file: %s\n", filename);
        return NULL;
    }

    Stack stack;
    initStack(&stack);
    TreeNode* root = NULL;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), file)) {
        char* p = trim(line);

        // Bo qua dong header "<?xml ..." hoac comment "<!-- ... -->"
        if (strncmp(p, "<?", 2) == 0 || strncmp(p, "<!--", 4) == 0) continue;

        // Duyet tung the trong dong
        while ((p = strchr(p, '<')) != NULL) {
            if (*(p + 1) == '/') {
                // Xu ly the dong
                char tag[256];
                sscanf(p, "</%[^> \t\n\r]>", tag);
                TreeNode* node = pop(&stack);
                if (!node || strcmp(node->tag_name, tag) != 0) {
                    printf("The dong khong khop: </%s>\n", tag);
                    fclose(file);
                    return NULL;
                }
                p = strchr(p, '>') + 1;
            } else {
                // Xu ly the mo
                char tag[256], attr_str[512] = "";
                sscanf(p, "<%[^>]>%*[^<]", tag); // Lay phan trong the

                char* space = strchr(tag, ' ');
                TreeNode* node;
                if (space) {
                    *space = '\0'; // tach ten the va thuoc tinh
                    node = createNode(tag);
                    parseAttributes(node, space + 1); // phan tich thuoc tinh
                } else {
                    node = createNode(tag);
                }

                TreeNode* parent = peek(&stack);
                if (parent) {
                    node->parent = parent;
                    if (!parent->first_child)
                        parent->first_child = node;
                    else {
                        TreeNode* sib = parent->first_child;
                        while (sib->next_sibling) sib = sib->next_sibling;
                        sib->next_sibling = node;
                    }
                } else {
                    if (!root) root = node;
                }

                push(&stack, node);

                // Lay noi dung trong the (text)
                char* end_tag = strchr(p, '>');
                if (end_tag && *(end_tag + 1) != '<') {
                    char* text_start = end_tag + 1;
                    char* text_end = strchr(text_start, '<');
                    if (text_end) {
                        *text_end = '\0';
                        node->text = strdup(trim(text_start));
                    }
                }

                p = strchr(p, '>') + 1;
            }
        }
    }

    fclose(file);

    if (!isEmpty(&stack)) {
        printf("Loi: Con the chua duoc dong.\n");
        return NULL;
    }

    printf("XML hop le.\n");
    return root;
}
