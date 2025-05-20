#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_NAME 50
#define MAX_NOTE 100
// Trạng thái đơn hàng
typedef enum {
 DANG_PHUC_VU,
 DA_THANH_TOAN,
 DON_HUY
} TrangThai;
// Cấu trúc món ăn
typedef struct Dish {
 char maMon[MAX_NAME];
 char thoiGian[20];
 int soLuongDat;
 int soLuongTra;
 char thoiGianCapNhat[20];
 char ghiChu[MAX_NOTE];
 struct Dish* next;
} Dish;
// Cấu trúc đơn hàng
typedef struct Order {
 char thoiGian[20];
 char tenNhanVien[MAX_NAME];
 int maBan;
 Dish* danhSachMon;
 int tongSoMon;
 int tongSoDiaDat;
 int tongSoMonTra;
 int tongSoDiaTra;
 char thoiGianCapNhat[20];
 TrangThai trangThai;
 struct Order* next;
} Order;
// Trả về con trỏ tới order hiện có của bàn có <mã bàn> theo đúng thứ tự.
Order* search_order (int table_id,Order* head){
    Order* current=head;
    while(current!=NULL){
        if(current->maBan==table_id&&current->trangThai==DANG_PHUC_VU){ // Check xem trạng thái của Order
            return current;
        }
        current=current->next;
    }
    printf("Chua co order\n");
    return NULL;
}
// Hàm in ra các order 
void print_order(int table_id,order* head){
    
}

int main() {
	// your code goes here
	

}

