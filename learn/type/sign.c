#include <stdio.h>
#include <stdint.h>

typedef struct NodeID
{
    uint64_t plogID;
    uint16_t offset;
} nodeId_T; 

#define INVALID_ID ((nodeId_T){-1, -1})

#define getPlogId(id) (id.plogID)
#define getOffset(id) (id.offset)

#define NODE_PTR_BIT 1UL
#define NODE_MASK_BITS (3UL)
#define NODE_PTR_MASK (((1UL) << NODE_MASK_BITS) - 1)
#define NODE_TO_ID(node) ((nodeId_T){((uint64_t)node) | NODE_PTR_BIT, -1})

/**
 * c语言无法用==判断两个结构体变量是否相等
 */
#define IS_NODE_DIRTY(node) (id.plogID == INVALID_ID.plogID && id.offset == INVALID_ID.offset)

int main(){
    nodeId_T id = NODE_TO_ID(8);
    printf("plogID:%llu offset:%u\n", getPlogId(id), getOffset(id));

    id = (nodeId_T){(uint64_t)-1, -1};
    printf("id == INVALID_ID: %d\n", IS_NODE_DIRTY(id));

    printf("NODE_PTR_MASK: %lu\n", NODE_PTR_MASK);
    return 0;
}