/* ==========================================================================
 *  FUNCTION 1 : EMERGENCY TRIAGE QUEUE
 *  DSA used   : Priority Queue implemented as a binary MAX-HEAP (array)
 *  Rule       : higher severity is served first; same severity -> who came first
 * ========================================================================== */
#ifndef TRIAGE_H
#define TRIAGE_H
#include "patient.h"

#define MAX_QUEUE 100

typedef struct {
    Patient data[MAX_QUEUE];
    int     size;
} TriageQueue;

void er_init(TriageQueue *q);
int  er_push(TriageQueue *q, Patient p);          /* 0 if queue is full  */
int  er_pop(TriageQueue *q, Patient *out);        /* 0 if queue is empty */
int  er_peek(const TriageQueue *q, Patient *out); /* 0 if queue is empty */
int  er_sorted(const TriageQueue *q, Patient out[]); /* priority order, returns count */

#endif
