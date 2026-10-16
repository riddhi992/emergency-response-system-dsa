/* ==========================================================================
 *  EMERGENCY RESPONSE SYSTEM - DSA Mini Project (pure C, console based)
 *
 *  Function 1 : Emergency triage queue      (triage.c  -> priority queue using a max-heap)
 *  Function 2 : Ambulance routing & finder  (citymap.c -> graph, BFS, DFS)
 *
 *  Both work together: the heap decides WHO is served first,
 *  the graph decides WHERE the ambulance takes them.
 *
 *  Build : gcc -Wall -Wextra -o hospital main.c triage.c citymap.c
 * ========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "triage.h"
#include "citymap.h"

static TriageQueue queue;
static CityMap     city;
static int         arrivalCounter = 0;
static int         nextId = 101;

/* ---------- input helpers ---------- */
static void read_line(const char *prompt, char *buf, size_t n) {
    printf("%s", prompt);
    if (!fgets(buf, (int)n, stdin)) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\r\n")] = '\0';
}
static int read_int(const char *prompt, int lo, int hi) {
    char b[64];
    for (;;) {
        read_line(prompt, b, sizeof b);
        char *end; long v = strtol(b, &end, 10);
        if (end != b && *end == '\0' && v >= lo && v <= hi) return (int)v;
        printf("  Enter a number between %d and %d.\n", lo, hi);
    }
}
static void show_areas(void) {
    printf("\n");
    for (int i = 0; i < city.numLoc; i++) {
        printf("  %2d. %-11s", i + 1, city.names[i]);
        if (i % 4 == 3) printf("\n");
    }
    printf("\n");
}

/* ---------- FUNCTION 1 helpers ---------- */
static int admit(const char *name, int age, int severity, int loc) {
    Patient p;
    p.id = nextId; p.age = age; p.severity = severity;
    p.arrival = arrivalCounter + 1; p.loc = loc;
    snprintf(p.name, sizeof p.name, "%s", name);
    if (!er_push(&queue, p)) { printf("  Queue is full!\n"); return 0; }
    arrivalCounter++;
    return nextId++;
}

static void do_register(void) {
    char name[40];
    read_line("  Name: ", name, sizeof name);
    int age = read_int("  Age: ", 0, 120);
    printf("  Severity: 1=Minor 2=Mild 3=Moderate 4=Serious 5=Critical\n");
    int sev = read_int("  Severity (1-5): ", 1, 5);
    show_areas();
    int loc = read_int("  Patient is in area (number): ", 1, city.numLoc) - 1;
    int id = admit(name[0] ? name : "Unknown", age, sev, loc);
    if (id) printf("  Registered with ID %d at %s. Patients waiting: %d\n", id, city.names[loc], queue.size);
}

static void do_waiting(void) {
    Patient list[MAX_QUEUE];
    int n = er_sorted(&queue, list);
    if (n == 0) { printf("  No patients waiting.\n"); return; }
    printf("\n  %-4s %-5s %-18s %-4s %-4s %s\n", "No.", "ID", "Name", "Age", "Sev", "Area");
    printf("  ------------------------------------------------------\n");
    for (int i = 0; i < n; i++)
        printf("  %-4d %-5d %-18s %-4d %-4d %s\n", i + 1, list[i].id, list[i].name,
               list[i].age, list[i].severity, city.names[list[i].loc]);
}

/* ---------- both functions together ---------- */
static void do_dispatch(void) {
    Patient p;
    if (!er_pop(&queue, &p)) { printf("  No patients waiting.\n"); return; }   /* FUNCTION 1 */

    printf("\n  Dispatching ambulance for ID %d  %s (age %d, severity %d)\n"
           "  Pick-up area: %s\n", p.id, p.name, p.age, p.severity, city.names[p.loc]);

    int hops[MAX_LOC], parent[MAX_LOC];
    int h = nearest_hospital(&city, p.loc, hops, parent, 1);                  /* FUNCTION 2 (BFS) */
    if (h < 0) {
        printf("\n  No reachable hospital with a free bed!\n"
               "  Patient put back in the queue (keeps original priority).\n");
        er_push(&queue, p);
        return;
    }
    city.hosp[h].beds--;
    printf("\n  >> Taking patient to %s. Beds left there: %d\n",
           city.hosp[h].name, city.hosp[h].beds);
    print_route(&city, hops, parent, p.loc, city.hosp[h].loc);
    printf("  Patients still waiting: %d\n", queue.size);
}

/* ---------- FUNCTION 2 actions ---------- */
static void do_route(void) {
    show_areas();
    int a = read_int("  From area: ", 1, city.numLoc) - 1;
    int b = read_int("  To area  : ", 1, city.numLoc) - 1;
    int hops[MAX_LOC], parent[MAX_LOC];
    bfs(&city, a, hops, parent);
    printf("\n  %s to %s (fewest roads)\n", city.names[a], city.names[b]);
    print_route(&city, hops, parent, a, b);
}

static void do_all_routes(void) {
    show_areas();
    int a = read_int("  From area: ", 1, city.numLoc) - 1;
    int b = read_int("  To area  : ", 1, city.numLoc) - 1;
    all_routes(&city, a, b);
}

static void do_block(void) {
    show_areas();
    int a = read_int("  Road between area: ", 1, city.numLoc) - 1;
    int b = read_int("  and area         : ", 1, city.numLoc) - 1;
    printf("  1. Block this road (accident / traffic)\n  2. Re-open this road\n");
    int act = read_int("  Choice: ", 1, 2);
    if (!map_set_blocked(&city, a, b, act == 1))
        printf("  There is no direct road between %s and %s.\n", city.names[a], city.names[b]);
    else
        printf("  Road %s <-> %s is now %s.\n", city.names[a], city.names[b],
               act == 1 ? "BLOCKED" : "OPEN");
}

static void do_connectivity(void) {
    show_areas();
    int s = read_int("  Check from area: ", 1, city.numLoc) - 1;
    int seen[MAX_LOC];
    int cnt = map_reachable(&city, s, seen);
    printf("  From %s you can reach %d of %d areas.\n", city.names[s], cnt, city.numLoc);
    if (cnt < city.numLoc) {
        printf("  CUT OFF: ");
        for (int i = 0; i < city.numLoc; i++) if (!seen[i]) printf("%s  ", city.names[i]);
        printf("\n");
    }
}

static void do_free_bed(void) {
    hospitals_print(&city);
    int h = read_int("  Free a bed at hospital (number): ", 1, city.numHosp) - 1;
    if (city.hosp[h].beds >= city.hosp[h].capacity)
        printf("  All beds at %s are already free.\n", city.hosp[h].name);
    else {
        city.hosp[h].beds++;
        printf("  Bed freed at %s. Free beds now: %d\n", city.hosp[h].name, city.hosp[h].beds);
    }
}

static void do_demo(void) {
    admit("Ravi Kumar",   45, 3, 4);   /* Chembur    */
    admit("Sneha Patil",  28, 5, 0);   /* Ghatkopar  */
    admit("Amit Shah",    60, 2, 6);   /* Bandra     */
    admit("Priya Nair",   34, 4, 8);   /* Powai      */
    admit("Karan Mehta",  19, 5, 1);   /* Vidyavihar */
    admit("Fatima Khan",  52, 1, 5);   /* Dadar      */
    printf("  Demo data loaded. Patients waiting: %d\n", queue.size);
}

int main(void) {
    er_init(&queue);
    map_init(&city);
    map_load_default(&city);

    for (;;) {
        printf("\n======= EMERGENCY RESPONSE SYSTEM =======\n"
               " --- Triage queue (Function 1) ---\n"
               "  1. Register patient\n"
               "  2. View waiting queue\n"
               " --- Both together ---\n"
               "  3. Dispatch ambulance for next critical patient\n"
               " --- City map & hospitals (Function 2) ---\n"
               "  4. Route with fewest roads (BFS)\n"
               "  5. All possible routes + fastest (DFS)\n"
               "  6. Show hospitals and free beds\n"
               "  7. Free a bed (patient discharged)\n"
               "  8. Block / re-open a road\n"
               "  9. Show city map (list + matrix)\n"
               " 10. Check which areas are reachable (DFS)\n"
               " 11. Load demo data\n"
               "  0. Exit\n");
        switch (read_int("Choice: ", 0, 11)) {
            case 1:  do_register();     break;
            case 2:  do_waiting();      break;
            case 3:  do_dispatch();     break;
            case 4:  do_route();        break;
            case 5:  do_all_routes();   break;
            case 6:  hospitals_print(&city); break;
            case 7:  do_free_bed();     break;
            case 8:  do_block();        break;
            case 9:  map_print(&city); map_print_matrix(&city); break;
            case 10: do_connectivity(); break;
            case 11: do_demo();         break;
            case 0:  map_free(&city); printf("Bye.\n"); return 0;
        }
    }
}
