/**
 ******************************************************************************
 * @file      heapLib.c
 * @brief     �ڴ����ģ��
 * @details   ���Ĺ���̬�ڴ���估�ͷ�(���Ĳ���ʹ���ź���)
 *
 * @copyright
 ******************************************************************************
 */
 
/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <listLib.h>
#include <intLib.h>
#include <debug.h>
#include <memLib.h>
#include <maths.h>

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
typedef struct _heap
{
    uint32_t presize;       /**< ��һ�ڵ��С,���λ1��ʾused 0 free */
    uint32_t cursize;       /**< ��ǰ�ڵ��С,���λ1��ʾused 0 free */
    struct ListNode node;   /**< ͨ������ڵ� */
} heap_t;

/*-----------------------------------------------------------------------------
 Section: Constant Definitions
 ----------------------------------------------------------------------------*/
#define WORD_SIZE               sizeof(uint32_t)
#define ALIGN_UP(addr)          (((addr) + WORD_SIZE - 1) & ~(WORD_SIZE - 1))
#define ALIGN_DOWN(addr)        ((addr) & ~(WORD_SIZE - 1))

#define DWORD_SIZE              (WORD_SIZE << 1)

#define MIN_HEAP_LEN            1024

/* �жϿ����ڴ�ڵ�����:���λ�Ƿ�Ϊ1 */
#define IS_FREE(nSize)          (((nSize) & (WORD_SIZE - 1)) == 0)
#define GET_SIZE(size)          (size & ~(WORD_SIZE - 1))

/*-----------------------------------------------------------------------------
 Section: Global Variables
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Local Variables
 ----------------------------------------------------------------------------*/
static struct ListNode the_heap_list = {NULL, NULL};    /**< ָ���ڴ����� */
static uint32_t the_totle_size = 0u;

/*-----------------------------------------------------------------------------
 Section: Local Function Prototypes
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Global Function Prototypes
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Function Definitions
 ----------------------------------------------------------------------------*/
/**
 ******************************************************************************
 * @brief   ��ȡ������һ���ڵ��ַ
 * @param[in]  *pheap   : ��ǰ�ڵ�
 *
 * @retval  ��һ���ڵ��ַ
 ******************************************************************************
 */
static inline heap_t *
heap_next(heap_t *pheap)
{
    return (heap_t *)((uint8_t *)pheap + ALIGN_UP(sizeof(heap_t))
            + GET_SIZE(pheap->cursize));
}

/**
 ******************************************************************************
 * @brief   ��ȡ������һ���ڵ��ַ
 * @param[in]  *pheap   : ��ǰ�ڵ�
 *
 * @retval  ��һ���ڵ��ַ
 ******************************************************************************
 */
static inline heap_t *
heap_pre(heap_t *pheap)
{
    return (heap_t *)((uint8_t *)pheap - ALIGN_UP(sizeof(heap_t))
            - GET_SIZE(pheap->presize));
}

/**
 ******************************************************************************
 * @brief   ���ýڵ��С
 * @param[in]  *pheap   : ��ǰ�ڵ�
 * @param[in]  size     : ��С
 *
 * @return  None
 ******************************************************************************
 */
static inline void
region_set_size(heap_t *pheap, size_t size)
{
    heap_t *pnext;

    pheap->cursize = size;
    pnext = heap_next(pheap);
    pnext->presize = size;
}

/**
 ******************************************************************************
 * @brief   ��ʼ����
 * @param[in]  start    : ����ʼ��ַ
 * @param[in]  end      : ��ĩ��ַ
 *
 * @retval     OK
 * @retval     ERROR
 *
 * @details  ��ʼ��������ڴ��������������㣬ͷ����β�ڵ�
 ******************************************************************************
 */
status_t
heaplib_add(uint32_t start, uint32_t end)
{
    heap_t *pfirst = NULL;
    heap_t *ptail = NULL;

    //todo: ��ֹ�ظ�����
    start = ALIGN_UP(start);   /* malloc�����׵�ַ��up�ֽڶ��� */
    end   = ALIGN_DOWN(end);   /* malloc����ĩ��ַ��down�ֽڶ��� */

    if (start + MIN_HEAP_LEN >= end)    /* ���ɷ���Ķѿռ�С����С�ѳߴ磬�򷵻� ���� */
    {
        return ERROR;
    }

    pfirst = (heap_t *)start;
    ptail = (heap_t *)(end - MOFFSET(heap_t, node));

    pfirst->cursize = end - start - 2 * ALIGN_UP(MOFFSET(heap_t, node));
    pfirst->presize = 0x01;

    ptail->presize = pfirst->cursize;
    ptail->cursize = 0x01;

    if (the_heap_list.pNextNode == NULL)
    {
        InitListHead(&the_heap_list);
    }
    intLock();
    ListAddTail(&pfirst->node, &the_heap_list);
    intUnlock();
    the_totle_size += end - start;

    return OK;
}

/**
 ******************************************************************************
 * @brief   malloc������ʵ��
 * @param[in]  nSize    : ��Ҫ����Ĵ�С
 *
 * @retval  ����ɹ����ص�ַ��ʧ�ܷ���NULL
 ******************************************************************************
 */
void *
malloc(size_t size)
{
    void *p = NULL;
    struct ListNode *piter;

    size_t alloc_size;
    size_t rest_size;
    heap_t *pheap;
    heap_t *pnext;

    if (the_heap_list.pNextNode == NULL)
    {
        return NULL;
    }

    /* ����ʵ����Ҫ�Ĵ�С(4�ֽڶ���)  */
    alloc_size = ALIGN_UP(size);

    intLock();    /* �����ٽ��� */

    LIST_FOR_EACH(piter, &the_heap_list)
    {
        pheap = MemToObj(piter, heap_t, node);
        if (pheap->cursize >= alloc_size)
        {
            goto do_alloc;
        }
    }

    intUnlock();  /* �˳��ٽ��� */

    return NULL;

do_alloc:
    ListDelNode(piter);  /* ����ɾ����ǰ�ڵ� */

    /* ��������ڴ��ʣ��ֵ */
    rest_size = GET_SIZE(pheap->cursize) - alloc_size;

    /* �������ʣ��ռ䲻�����ٷ���ڵ� */
    if (rest_size <= ALIGN_UP(MOFFSET(heap_t, node)))
    {
        /* ������ʣ���ڴ�С�ڽ���Сʱ��ȫ���ڴ����*/
        region_set_size(pheap, pheap->cursize | 0x01);
    }
    else
    {
        /* �ڵ�ǰ�ڵ����ռ�,����־ʹ��λ */
        region_set_size(pheap, alloc_size | 0x01);

        /* �����һ�ڵ��ַ */
        pnext = heap_next(pheap);

        /* ���¼�����һ�ڵ�ɷ����ڴ� */
        region_set_size(pnext, rest_size - ALIGN_UP(MOFFSET(heap_t, node)));

        /* ������ڵ����ӵ��ڴ�free���������� */
        ListAddTail(&pnext->node, &the_heap_list);
    }
    intUnlock();  /* �˳��ٽ��� */

    /* ����ڴ��ַ ��������,��ֱ��дlnMemRegion*/
    p = &pheap->node;

    return p;
}

/**
 ******************************************************************************
 * @brief   �ڴ��ͷ�
 * @param[in]  None
 *
 * @retval     None
 ******************************************************************************
 */
void
free(void *p)
{
    heap_t *pheap;
    heap_t *ptmp;

    if (the_heap_list.pNextNode == NULL)
    {
        return;
    }

    D_ASSERT((uint32_t)p & 0x03);

    pheap = (heap_t *)((size_t)p - ALIGN_UP(MOFFSET(heap_t, node)));


    intLock();    /* �����ٽ��� */

    /* ����һ���ڵ�Ϊfree״̬ */
    ptmp = heap_next(pheap);
    if (IS_FREE(ptmp->cursize))
    {
        /* ��ϲ���ǰ�ڵ����һ���ڵ㣬�����ܿ���ռ� */
        region_set_size(pheap, GET_SIZE(pheap->cursize)
                + ALIGN_UP(MOFFSET(heap_t, node)) + ptmp->cursize);
        /* ɾ����һ���ڵ� */
        ListDelNode(&ptmp->node);
    }
    else
    {
        /* ����λ��Ϊ0����ʾ����״̬ */
        region_set_size(pheap, GET_SIZE(pheap->cursize));
    }

    /* ����һ���ڵ��ǿ���״̬ */
    if (IS_FREE(pheap->presize))
    {
        ptmp = heap_pre(pheap);
        region_set_size(ptmp, pheap->cursize + pheap->presize
                + ALIGN_UP(MOFFSET(heap_t, node)));
    }
    else
    {
        /* �����ڵ���ӵ��ڴ�free������ */
        ListAddTail(&pheap->node, &the_heap_list);
    }

    intUnlock();   /* �˳��ٽ��� */
}

/**
 ******************************************************************************
 * @brief      ����ڴ�ʹ����Ϣ
 * @param[in]  None
 * @param[out] None
 * @retval     None
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
void
showMenInfo(void)
{
    struct ListNode *piter;
    heap_t *pheap;
    uint32_t TotalFreeSize = 0u;
    uint32_t MaxSize = 0u;
    uint32_t MinSize = 0xffffffff;

    if (the_heap_list.pNextNode == NULL)
    {
        printf(" Heap not initialized! Please call 'mem_init()'.\n");
        return ;
    }

    intLock();    /* �����ٽ��� */
    /* ����free�ڴ����� */
    LIST_FOR_EACH(piter, &the_heap_list)
    {
        /* ȡ�ñ������Ķ��� */
        pheap = MemToObj(piter, heap_t, node);
        if (pheap->cursize > MaxSize)
        {
            MaxSize = pheap->cursize;
        }
        if (pheap->cursize < MinSize)
        {
            MinSize = pheap->cursize;
        }
        TotalFreeSize += pheap->cursize;
    }
    intUnlock();   /* �˳��ٽ��� */

    printf("********** Heap Monitor ***********\n");
    printf(" TotalHeapMem = %4d Kb  %4d Byte\n", the_totle_size / 1024, the_totle_size % 1024);
    printf(" TotalFreeMem = %4d Kb  %4d Byte\n", TotalFreeSize / 1024, TotalFreeSize % 1024);
    printf(" MaxFreeMem   = %4d Kb  %4d Byte\n", MaxSize / 1024, MaxSize % 1024);
    printf(" MinFreeMem   = %4d Kb  %4d Byte\n", MinSize / 1024, MinSize % 1024);
    //printf(" Fragindices  = %.2f\n", 1-(float)MaxSize / (float)TotalFreeSize);
    printf("***********************************\n");
}
/*----------------------------heapLib.c--------------------------------*/
