#include <stdio.h>
#include <stdlib.h>
#include "citymap.h"

/* ============================ GRAPH BUILDING ============================ */

void map_init(CityMap *m) {
    m->numLoc = 0;
    m->numHosp = 0;
    for (int i = 0; i < MAX_LOC; i++) m->adj[i] = NULL;
}

int map_add_location(CityMap *m, const char *name) {
    if (m->numLoc == MAX_LOC) return -1;
    snprintf(m->names[m->numLoc], sizeof m->names[0], "%s", name);
    return m->numLoc++;
}

static void add_directed(CityMap *m, int a, int b, int w) {
    Edge *e = malloc(sizeof *e);                 /* dynamic memory allocation */
    if (!e) { fprintf(stderr, "Out of memory\n"); exit(1); }
    e->to = b; e->minutes = w; e->blocked = 0;
    e->next = m->adj[a];                         /* insert at head of linked list */
    m->adj[a] = e;
}

int map_add_road(CityMap *m, int a, int b, int minutes) {
    if (a < 0 || b < 0 || a >= m->numLoc || b >= m->numLoc || a == b || minutes <= 0) return 0;
    add_directed(m, a, b, minutes);              /* undirected = two directed edges */
    add_directed(m, b, a, minutes);
    return 1;
}

int map_add_hospital(CityMap *m, const char *name, int loc, int beds) {
    if (m->numHosp == MAX_HOSP || loc < 0 || loc >= m->numLoc) return 0;
    Hospital *h = &m->hosp[m->numHosp++];
    snprintf(h->name, sizeof h->name, "%s", name);
    h->loc = loc; h->beds = beds; h->capacity = beds;
    return 1;
}

void map_free(CityMap *m) {
    for (int i = 0; i < m->numLoc; i++) {
        Edge *e = m->adj[i];
        while (e) { Edge *n = e->next; free(e); e = n; }
        m->adj[i] = NULL;
    }
}

/* a small sample city (travel times in minutes - illustrative) */
void map_load_default(CityMap *m) {
    const char *areas[] = { "Ghatkopar", "Vidyavihar", "Kurla", "Sion", "Chembur",
                            "Dadar", "Bandra", "Andheri", "Powai", "Mulund" };
    for (int i = 0; i < 10; i++) map_add_location(m, areas[i]);

    /*            from to min */
    map_add_road(m, 0, 1,  5);   /* Ghatkopar  - Vidyavihar */
    map_add_road(m, 0, 2,  8);   /* Ghatkopar  - Kurla      */
    map_add_road(m, 0, 4, 11);   /* Ghatkopar  - Chembur    */
    map_add_road(m, 0, 8, 12);   /* Ghatkopar  - Powai      */
    map_add_road(m, 0, 9, 14);   /* Ghatkopar  - Mulund     */
    map_add_road(m, 1, 2,  6);   /* Vidyavihar - Kurla      */
    map_add_road(m, 1, 8, 11);   /* Vidyavihar - Powai      */
    map_add_road(m, 2, 3,  9);   /* Kurla      - Sion       */
    map_add_road(m, 2, 4,  7);   /* Kurla      - Chembur    */
    map_add_road(m, 2, 6, 15);   /* Kurla      - Bandra     */
    map_add_road(m, 3, 5,  8);   /* Sion       - Dadar      */
    map_add_road(m, 3, 4, 10);   /* Sion       - Chembur    */
    map_add_road(m, 5, 6, 12);   /* Dadar      - Bandra     */
    map_add_road(m, 6, 7, 14);   /* Bandra     - Andheri    */
    map_add_road(m, 7, 8, 16);   /* Andheri    - Powai      */
    map_add_road(m, 8, 9, 15);   /* Powai      - Mulund     */

    map_add_hospital(m, "Sion General",    3, 2);
    map_add_hospital(m, "Kurla Care",      2, 1);
    map_add_hospital(m, "Andheri Medical", 7, 3);
    map_add_hospital(m, "Mulund Health",   9, 2);
}

/* ================================ DISPLAY ================================ */

static Edge *find_edge(const CityMap *m, int a, int b) {
    if (a < 0 || a >= m->numLoc) return NULL;
    for (Edge *e = m->adj[a]; e; e = e->next)
        if (e->to == b) return e;
    return NULL;
}

int map_set_blocked(CityMap *m, int a, int b, int blocked) {
    Edge *e1 = find_edge(m, a, b), *e2 = find_edge(m, b, a);
    if (!e1 || !e2) return 0;
    e1->blocked = e2->blocked = blocked;
    return 1;
}

void map_print(const CityMap *m) {
    printf("\n  ADJACENCY LIST  (neighbour and minutes)\n");
    printf("  ----------------------------------------------------------\n");
    for (int i = 0; i < m->numLoc; i++) {
        printf("  %-11s: ", m->names[i]);
        for (const Edge *e = m->adj[i]; e; e = e->next)
            printf("%s(%d)%s  ", m->names[e->to], e->minutes, e->blocked ? "[BLOCKED]" : "");
        printf("\n");
    }
}

void map_print_matrix(const CityMap *m) {
    int mat[MAX_LOC][MAX_LOC] = {{0}};
    for (int i = 0; i < m->numLoc; i++)
        for (const Edge *e = m->adj[i]; e; e = e->next)
            mat[i][e->to] = e->blocked ? -1 : e->minutes;

    printf("\n  ADJACENCY MATRIX  (minutes, . = no road, X = blocked)\n");
    printf("  %-11s ", "");
    for (int j = 0; j < m->numLoc; j++) printf("%-4.3s", m->names[j]);
    printf("\n");
    for (int i = 0; i < m->numLoc; i++) {
        printf("  %-11s ", m->names[i]);
        for (int j = 0; j < m->numLoc; j++) {
            char cell[8];
            if      (mat[i][j] == 0)  snprintf(cell, sizeof cell, ".");
            else if (mat[i][j] == -1) snprintf(cell, sizeof cell, "X");
            else                      snprintf(cell, sizeof cell, "%d", mat[i][j]);
            printf("%-4s", cell);
        }
        printf("\n");
    }
}

void hospitals_print(const CityMap *m) {
    printf("\n  %-3s %-18s %-12s %s\n", "No.", "Hospital", "Area", "Free beds");
    printf("  ---------------------------------------------------\n");
    for (int i = 0; i < m->numHosp; i++)
        printf("  %-3d %-18s %-12s %d / %d%s\n", i + 1, m->hosp[i].name, m->names[m->hosp[i].loc],
               m->hosp[i].beds, m->hosp[i].capacity, m->hosp[i].beds == 0 ? "  (FULL)" : "");
}

/* ===================================================================== *
 *  BFS  (uses a QUEUE)
 *  Visits areas level by level, so the first time we reach an area we have
 *  used the FEWEST roads possible.   hops[v]   = number of roads from src
 *                                    parent[v] = previous area on that route
 *  Blocked roads are skipped.   Complexity : O(V + E)
 * ===================================================================== */
void bfs(const CityMap *m, int src, int hops[], int parent[]) {
    int queue[MAX_LOC], head = 0, tail = 0;     /* every area enters the queue once */

    for (int i = 0; i < m->numLoc; i++) { hops[i] = INF; parent[i] = -1; }
    hops[src] = 0;
    queue[tail++] = src;                        /* enqueue */

    while (head < tail) {
        int u = queue[head++];                  /* dequeue */
        for (const Edge *e = m->adj[u]; e; e = e->next) {
            if (e->blocked) continue;           /* road closed */
            if (hops[e->to] == INF) {           /* not visited yet */
                hops[e->to]   = hops[u] + 1;
                parent[e->to] = u;
                queue[tail++] = e->to;
            }
        }
    }
}

/* total minutes along the route stored in parent[] (walk back from dst) */
static int path_minutes(const CityMap *m, const int parent[], int dst) {
    int total = 0;
    for (int v = dst; parent[v] != -1; v = parent[v]) {
        const Edge *e = find_edge(m, parent[v], v);
        if (e) total += e->minutes;
    }
    return total;
}

void print_route(const CityMap *m, const int hops[], const int parent[], int src, int dst) {
    (void)src;
    if (hops[dst] >= INF) { printf("  No route available (roads blocked).\n"); return; }

    int path[MAX_LOC], n = 0;
    for (int v = dst; v != -1; v = parent[v]) path[n++] = v;   /* walk back to src */

    printf("  Route: ");
    for (int i = n - 1; i >= 0; i--)
        printf("%s%s", m->names[path[i]], i ? " -> " : "");
    printf("   (%d road(s), %d min)\n", hops[dst], path_minutes(m, parent, dst));
}

/* nearest hospital WITH a free bed = fewest roads (ties -> less travel time) */
int nearest_hospital(const CityMap *m, int src, int hops[], int parent[], int verbose) {
    bfs(m, src, hops, parent);

    if (verbose) {
        printf("\n  %-18s %-12s %-5s %-6s %s\n", "Hospital", "Area", "Beds", "Roads", "Time");
        printf("  -------------------------------------------------------------\n");
    }
    int best = -1, bestHops = INF, bestMins = INF;
    for (int i = 0; i < m->numHosp; i++) {
        int d = hops[m->hosp[i].loc];
        int mins = (d < INF) ? path_minutes(m, parent, m->hosp[i].loc) : INF;
        if (verbose) {
            printf("  %-18s %-12s %-5d ", m->hosp[i].name, m->names[m->hosp[i].loc], m->hosp[i].beds);
            if (d >= INF) printf("unreachable\n");
            else printf("%-6d %d min%s\n", d, mins, m->hosp[i].beds == 0 ? "  (full - skipped)" : "");
        }
        if (m->hosp[i].beds > 0 && d < INF &&
            (d < bestHops || (d == bestHops && mins < bestMins))) {
            best = i; bestHops = d; bestMins = mins;
        }
    }
    return best;
}

/* ===================================================================== *
 *  DFS with BACKTRACKING  (uses RECURSION = the call stack)
 *  Finds EVERY simple route from src to dst, then reports the fastest.
 *  Small city -> fine.  (Worst case is exponential on big graphs.)
 * ===================================================================== */
#define SHOW_LIMIT 10

typedef struct {
    int count, bestMin, bestLen;
    int bestPath[MAX_LOC];
} RouteStats;

static void print_path(const CityMap *m, const int path[], int len) {
    for (int i = 0; i < len; i++)
        printf("%s%s", m->names[path[i]], i < len - 1 ? " -> " : "");
}

static void dfs_routes(const CityMap *m, int u, int dst, int visited[],
                       int path[], int len, int minutes, RouteStats *st) {
    path[len++] = u;                             /* choose  */
    visited[u] = 1;

    if (u == dst) {                              /* reached the destination */
        st->count++;
        if (st->count <= SHOW_LIMIT) {
            printf("  %2d. ", st->count);
            print_path(m, path, len);
            printf("   (%d min)\n", minutes);
        }
        if (minutes < st->bestMin) {
            st->bestMin = minutes;
            st->bestLen = len;
            for (int i = 0; i < len; i++) st->bestPath[i] = path[i];
        }
    } else {
        for (const Edge *e = m->adj[u]; e; e = e->next)
            if (!e->blocked && !visited[e->to])
                dfs_routes(m, e->to, dst, visited, path, len, minutes + e->minutes, st);
    }
    visited[u] = 0;                              /* un-choose (backtrack) */
}

void all_routes(const CityMap *m, int src, int dst) {
    if (src == dst) { printf("  Source and destination are the same area.\n"); return; }

    int visited[MAX_LOC] = {0}, path[MAX_LOC];
    RouteStats st = { 0, INF, 0, {0} };

    printf("\n  Routes from %s to %s:\n", m->names[src], m->names[dst]);
    dfs_routes(m, src, dst, visited, path, 0, 0, &st);

    if (st.count == 0) { printf("  No route available (roads blocked).\n"); return; }
    if (st.count > SHOW_LIMIT) printf("  ... and %d more\n", st.count - SHOW_LIMIT);
    printf("\n  Total routes found : %d\n  FASTEST route      : ", st.count);
    print_path(m, st.bestPath, st.bestLen);
    printf("   (%d min)\n", st.bestMin);
}

/* DFS again - which areas can still be reached from src? */
static void dfs_mark(const CityMap *m, int u, int seen[], int *count) {
    seen[u] = 1;
    (*count)++;
    for (const Edge *e = m->adj[u]; e; e = e->next)
        if (!e->blocked && !seen[e->to]) dfs_mark(m, e->to, seen, count);
}

int map_reachable(const CityMap *m, int src, int seen[]) {
    for (int i = 0; i < m->numLoc; i++) seen[i] = 0;
    int count = 0;
    dfs_mark(m, src, seen, &count);
    return count;
}
