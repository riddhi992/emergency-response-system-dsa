#ifndef PATIENT_H
#define PATIENT_H

typedef struct {
    int  id;            /* auto-generated hospital ID                   */
    char name[40];
    int  age;
    int  severity;      /* 1 (minor) .. 5 (critical)                    */
    int  arrival;       /* arrival order number, used to break ties     */
    int  loc;           /* index of the area where the patient is       */
} Patient;

#endif
