#include"list.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>

typedef struct list_entry
{
    int num;
    struct list_head point;
} ListEntry;

ListEntry *NewListEntry(int num) {
    ListEntry *entry = malloc(sizeof(ListEntry));
    entry->num = num;
    INIT_LIST_HEAD(&entry->point);
    return entry;
}

// 这里的参数一定要是指针，因为该指针会被用来判断链表的结束
void DestroyList(struct list_head *head) {
    ListEntry *cur = NULL, *next = NULL;
    list_for_each_entry_safe(cur, next, head, point) {
        list_del(&cur->point);
        free(cur);
    }
}

int main() {
    LIST_HEAD(head);
    // 等同于：
    // struct list_head head;
    // INIT_LIST_HEAD(&head);
    assert(&head == head.prev);
    assert(&head == head.next);

    ListEntry *pentry1 = NewListEntry(1);
    ListEntry *tmp = container_of(&(pentry1->num), ListEntry, num);
    assert(pentry1 == tmp);
    list_add_tail(&pentry1->point, &head);

    // 关于块语句的使用
    char *str = ({   // 外面的圆括号不能少
        char *tmp = (char*)malloc(7);
        strcpy(tmp, "Hello.");
        (char*)tmp;
    });
    printf("%s\n", str);
    free(str);

    list_add_tail(&NewListEntry(2)->point, &head);

    tmp = NULL;
    tmp = list_prepare_entry(tmp, head.next->next, point);
    printf("num: %d\n", tmp->num);

    // 关于缺省的三元运算符?:
    int num = 0;
    num = num ? : num++;
    printf("num: %d\n", num);

    int count = list_entry_number(&head);
    printf("count: %d\n", count);

    DestroyList(&head);
    return 0;
}