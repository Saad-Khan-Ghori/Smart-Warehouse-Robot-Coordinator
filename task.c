/* ================================================================
   task.c  --  Priority queue implementation (binary max-heap)

   Heap refresher for the viva:
     - A max-heap is a complete binary tree where every parent
       has a priority >= its children.
     - We store it in a flat array; tree structure is implicit
       via index arithmetic (no pointers needed).
     - Insert: place new element at the end, then "sift up"
       (swap with parent while parent.priority < child.priority).
     - Extract-max: swap root with last element, shrink size,
       then "sift down" (swap with the larger child while
       parent < child).
   ================================================================ */

#include "task.h"
#include <stdio.h>
#include <time.h>

/* ----------------------------------------------------------------
   Internal helpers: swap two tasks in the array
   ---------------------------------------------------------------- */
static void swap_tasks(Task *a, Task *b) {
    Task tmp = *a;
    *a = *b;
    *b = tmp;
}

/* ----------------------------------------------------------------
   sift_up: restore heap property after inserting at index i
   Called after appending a new element at heap[size-1].
   ---------------------------------------------------------------- */
static void sift_up(PriorityQueue *pq, int i) {
    /* While we're not at the root and parent has lower priority */
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (pq->heap[parent].priority < pq->heap[i].priority) {
            swap_tasks(&pq->heap[parent], &pq->heap[i]);
            i = parent;  /* continue checking upward */
        } else {
            break;  /* heap property restored */
        }
    }
}

/* ----------------------------------------------------------------
   sift_down: restore heap property after removing the root.
   Called after moving the last element to index 0.
   ---------------------------------------------------------------- */
static void sift_down(PriorityQueue *pq, int i) {
    int n = pq->size;
    while (1) {
        int left    = 2 * i + 1;
        int right   = 2 * i + 2;
        int largest = i;           /* assume current node is largest */

        /* Check if left child exists and is larger */
        if (left < n && pq->heap[left].priority > pq->heap[largest].priority)
            largest = left;

        /* Check if right child exists and is larger */
        if (right < n && pq->heap[right].priority > pq->heap[largest].priority)
            largest = right;

        if (largest != i) {
            swap_tasks(&pq->heap[i], &pq->heap[largest]);
            i = largest;  /* continue checking downward */
        } else {
            break;  /* heap property restored */
        }
    }
}

/* ----------------------------------------------------------------
   pq_init: set size to zero (array already allocated in struct)
   ---------------------------------------------------------------- */
void pq_init(PriorityQueue *pq) {
    pq->size = 0;
}

/* ----------------------------------------------------------------
   pq_insert: add a task and restore heap property via sift_up
   ---------------------------------------------------------------- */
int pq_insert(PriorityQueue *pq, Task t) {
    if (pq->size >= MAX_TASKS) {
        fprintf(stderr, "[PQ] ERROR: heap full, cannot insert Task-%d\n", t.id);
        return -1;
    }
    /* Record the time this task entered the queue (for wait-time stats) */
    t.enqueueTime = time(NULL);

    pq->heap[pq->size] = t;    /* place at end   */
    pq->size++;
    sift_up(pq, pq->size - 1); /* restore heap   */
    return 0;
}

/* ----------------------------------------------------------------
   pq_extract_max: remove and return the highest-priority task.
   The root (index 0) is always the max in a max-heap.
   ---------------------------------------------------------------- */
Task pq_extract_max(PriorityQueue *pq) {
    /* Root is the highest-priority task */
    Task max_task = pq->heap[0];

    /* Move last element to root position, shrink heap */
    pq->size--;
    pq->heap[0] = pq->heap[pq->size];

    /* Restore heap property downward */
    if (pq->size > 0)
        sift_down(pq, 0);

    return max_task;
}

/* ----------------------------------------------------------------
   pq_is_empty: returns 1 if empty
   ---------------------------------------------------------------- */
int pq_is_empty(const PriorityQueue *pq) {
    return pq->size == 0;
}

/* ----------------------------------------------------------------
   pq_apply_aging: scan all tasks and boost priority if they've
   been waiting longer than AGING_THRESHOLD_SEC seconds.

   WHY AGING?
   Without aging, a flood of priority-10 tasks could starve a
   priority-1 task indefinitely.  Aging ensures every task
   eventually reaches priority 10 and gets served.

   After boosting any tasks we call sift_up on those elements to
   maintain the heap invariant.
   ---------------------------------------------------------------- */
void pq_apply_aging(PriorityQueue *pq) {
    time_t now = time(NULL);
    int i;
    for (i = 0; i < pq->size; i++) {
        double wait = difftime(now, pq->heap[i].enqueueTime);
        if (wait >= AGING_THRESHOLD_SEC &&
            pq->heap[i].priority < MAX_PRIORITY)
        {
            pq->heap[i].priority++;
            /* Re-establish heap property for this element */
            sift_up(pq, i);
        }
    }
}
