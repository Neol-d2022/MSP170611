#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUGP printf("%u\n", __LINE__);

typedef struct
{
    unsigned int a;
    unsigned int b;
} edge_t;

typedef struct
{
    edge_t e;
    unsigned int w;
} weighted_edge_t;

typedef struct queue_obj_struct_t
{
    weighted_edge_t *we;
    struct queue_obj_struct_t *next;
} queue_obj_t;

typedef struct
{
    queue_obj_t *head;
    unsigned int count;
} queue_head_t;

int SetEdge(edge_t *e, unsigned int a, unsigned int b)
{
    if (a > b)
    {
        e->a = b;
        e->b = a;
    }
    else if (b > a)
    {
        e->a = a;
        e->b = b;
    }
    else
        return 1;
    return 0;
}

int EdgeContainV(const edge_t *e, unsigned int v)
{
    if (e->a == v || e->b == v)
        return 1;
    else
        return 0;
}

queue_head_t *CreateQueue(void)
{
    queue_head_t *p;

    p = (queue_head_t *)malloc(sizeof(queue_head_t));
    memset(p, 0, sizeof(*p));
    return p;
}

void DestroyQueue(queue_head_t *q)
{
    queue_obj_t *head = q->head, *next;

    while (head)
    {
        free(head->we);
        next = head->next;
        free(head);
        head = next;
    }
    free(q);
}

int cmpWE_byEdge(const void *a, const void *b)
{
    weighted_edge_t *c = (weighted_edge_t *)a;
    weighted_edge_t *d = (weighted_edge_t *)b;

    return memcmp(&(c->e), &(d->e), sizeof(c->e));
}

void QueueAdd(queue_head_t *q, weighted_edge_t *we)
{
    queue_obj_t **cur = &(q->head);
    queue_obj_t *n;

    while (*cur)
    {
        if (we->w < (*cur)->we->w)
            break;
        else if (we->w == (*cur)->we->w)
        {
            if (cmpWE_byEdge(&(we->e), &((*cur)->we->e)) < 0)
                break;
        }
        cur = &((*cur)->next);
    }

    n = malloc(sizeof(*n));
    n->we = we;
    n->next = *cur;
    *cur = n;
    q->count += 1;
}

void QueueAdd_noSort(queue_head_t *q, weighted_edge_t *we)
{
    queue_obj_t **cur = &(q->head);
    queue_obj_t *n;

    while (*cur)
        cur = &((*cur)->next);

    n = malloc(sizeof(*n));
    n->we = we;
    n->next = *cur;
    *cur = n;
    q->count += 1;
}

queue_obj_t *QueueFind(queue_head_t *q, const weighted_edge_t *target)
{
    queue_obj_t *head = q->head, *next;

    while (head)
    {
        next = head->next;
        if ((head->we->e).a == (target->e).a && (head->we->e).b == (target->e).b)
            break;
        head = next;
    }

    return head;
}

int cmpUint(const void *a, const void *b)
{
    unsigned int c = *(unsigned int *)a;
    unsigned int d = *(unsigned int *)b;

    if (c > d)
        return 1;
    else if (c < d)
        return -1;
    else
        return 0;
}

int main(int argc, char **argv)
{
    char buf[256];
    edge_t target;
    FILE *f;
    weighted_edge_t *we_base, *we;
    queue_head_t *q = CreateQueue();
    queue_head_t *d = CreateQueue();
    queue_obj_t *item, **cur, **next, *pobj;
    unsigned int *v_base, *v_undone, *done_v;
    unsigned int fileLength, i, a, b, w, we_length, v_length, undone_offset, newv, j, total_weight;

    if (argc != 2)
        return 1;

    f = fopen(argv[1], "r");
    if (!f)
        return 2;

    fileLength = 0;
    while (fgets(buf, sizeof(buf), f))
        fileLength += 1;

    if (!fileLength)
    {
        fclose(f);
        return 0;
    }
    rewind(f);

    i = 0;
    v_length = 0;
    we_base = (weighted_edge_t *)malloc(sizeof(*we_base) * fileLength);
    v_base = (unsigned int *)malloc(sizeof(*v_base) * fileLength * 2);
    while (fgets(buf, sizeof(buf), f))
    {
        if (sscanf(buf, "(%u,%u)=%u", &a, &b, &w) == 3)
        {
            SetEdge(&(we_base[i].e), a, b);
            we_base[i].w = w;
            i += 1;

            if (!bsearch(&a, v_base, v_length, sizeof(*v_base), cmpUint))
            {
                v_base[v_length] = a;
                v_length += 1;
                qsort(v_base, v_length, sizeof(*v_base), cmpUint);
            }
            if (!bsearch(&b, v_base, v_length, sizeof(*v_base), cmpUint))
            {
                v_base[v_length] = b;
                v_length += 1;
                qsort(v_base, v_length, sizeof(*v_base), cmpUint);
            }
        }
    }
    we_length = i;

    undone_offset = 1;
    v_undone = (unsigned int *)malloc(sizeof(*v_base) * v_length);
    memcpy(v_undone, v_base, sizeof(*v_base) * v_length);
    for (i = 0; i < we_length; i += 1)
    {
        if (EdgeContainV(&(we_base[i].e), v_undone[0]))
        {
            we = (weighted_edge_t *)malloc(sizeof(*we));
            memcpy(we, we_base + i, sizeof(*we));
            QueueAdd(q, we);
        }
    }
    qsort(v_undone + undone_offset, v_length - undone_offset, sizeof(*v_undone), cmpUint);
    //printf("%u processed\n", v_undone[0]);

    while (q->head)
    {
        item = q->head;
        q->head = item->next;
        q->count -= 1;
        //printf("(%u,%u)=%u picked\n", (item->we->e).a, (item->we->e).b, (item->we)->w);

        if ((done_v = bsearch(&((item->we->e).a), v_undone + undone_offset, v_length - undone_offset, sizeof(*v_undone), cmpUint)))
            newv = (item->we->e).a;
        else
        {
            done_v = bsearch(&((item->we->e).b), v_undone + undone_offset, v_length - undone_offset, sizeof(*v_undone), cmpUint);
            newv = (item->we->e).b;
        }

        i = v_undone[undone_offset];
        v_undone[undone_offset] = *done_v;
        *done_v = i;
        //printf("%u processed\n", newv);
        undone_offset += 1;
        qsort(v_undone + undone_offset, v_length - undone_offset, sizeof(*v_undone), cmpUint);

        for (i = undone_offset; i < v_length; i += 1)
        {
            SetEdge(&target, newv, v_undone[i]);
            for (j = 0; j < we_length; j += 1)
            {
                if (cmpWE_byEdge(&target, we_base + j) == 0)
                {
                    we = (weighted_edge_t *)malloc(sizeof(*we));
                    memcpy(we, we_base + j, sizeof(*we));
                    if (QueueFind(q, we))
                        free(we);
                    else
                    {
                        QueueAdd(q, we);
                        //printf("(%u,%u)=%u added\n", (we_base[j].e).a, (we_base[j].e).b, we_base[j].w);
                    }
                }
            }
        }

        cur = &(q->head);
        while (*cur)
        {
            if (
                !bsearch(&(((*cur)->we->e).a), v_undone + undone_offset, v_length - undone_offset, sizeof(*v_undone), cmpUint) &&
                !bsearch(&(((*cur)->we->e).b), v_undone + undone_offset, v_length - undone_offset, sizeof(*v_undone), cmpUint))
            {
                //printf("(%u,%u)=%u deleted\n", (((*cur)->we)->e).a, (((*cur)->we)->e).b, ((*cur)->we)->w);
                free((*cur)->we);
                pobj = *cur;
                *cur = (*cur)->next;
                free(pobj);
                q->count -= 1;
                if (!(*cur))
                    break;
                else
                    continue;
            }
            cur = &((*cur)->next);
        }

        QueueAdd_noSort(d, item->we);
    }

    total_weight = 0;
    cur = &(d->head);
    while (*cur)
    {
        next = &((*cur)->next);
        total_weight += (*cur)->we->w;
        printf("(%u,%u)=%u\n", ((*cur)->we->e).a, ((*cur)->we->e).b, (*cur)->we->w);
        cur = next;
    }
    printf("\nTotal weight = %u\n", total_weight);

    free(v_undone);
    free(v_base);
    free(we_base);
    DestroyQueue(q);
    DestroyQueue(d);

    fclose(f);
}