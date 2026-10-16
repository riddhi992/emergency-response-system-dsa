#include <stdio.h>
#include "triage.h"

/* returns 1 if a must be served before b */
static int higher(const Patient *a, const Patient *b) {
    if (a->severity != b->severity) return a->severity > b->severity;
    return a->arrival < b->arrival;
}

static void swap(Patient *a, Patient *b) { Patient t = *a; *a = *b; *b = t; }

void er_init(TriageQueue *q) { q->size = 0; }

int er_push(TriageQueue *q, Patient p) {
    if (q->size == MAX_QUEUE) return 0;
    int i = q->size++;
    q->data[i] = p;
    while (i > 0) {                                  /* sift UP */
        int parent = (i - 1) / 2;
        if (!higher(&q->data[i], &q->data[parent])) break;
        swap(&q->data[i], &q->data[parent]);
        i = parent;
    }
    return 1;
}

int er_pop(TriageQueue *q, Patient *out) {
    if (q->size == 0) return 0;
    *out = q->data[0];                               /* root = most urgent */
    q->data[0] = q->data[--q->size];
    int i = 0;
    for (;;) {                                       /* sift DOWN */
        int l = 2*i + 1, r = 2*i + 2, best = i;
        if (l < q->size && higher(&q->data[l], &q->data[best])) best = l;
        if (r < q->size && higher(&q->data[r], &q->data[best])) best = r;
        if (best == i) break;
        swap(&q->data[i], &q->data[best]);
        i = best;
    }
    return 1;
}

int er_peek(const TriageQueue *q, Patient *out) {
    if (q->size == 0) return 0;
    *out = q->data[0];
    return 1;
}

/* heap-sort style: pop everything from a COPY so the real queue is untouched */
int er_sorted(const TriageQueue *q, Patient out[]) {
    TriageQueue copy = *q;
    int n = 0;
    while (er_pop(&copy, &out[n])) n++;
    return n;
}
