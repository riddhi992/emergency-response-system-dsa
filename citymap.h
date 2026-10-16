/* ==========================================================================
 *  FUNCTION 2 : AMBULANCE ROUTING & HOSPITAL FINDER
 *  DSA used   : GRAPH  - adjacency LIST (linked lists) + adjacency MATRIX view
 *               BFS    - uses a QUEUE  -> route with fewest roads / nearest hospital
 *               DFS    - uses RECURSION (backtracking) -> all routes, connectivity
 * ========================================================================== */
#ifndef CITYMAP_H
#define CITYMAP_H

#define MAX_LOC   12
#define MAX_HOSP   6
#define INF  1000000000

/* one road = one edge in the adjacency list */
typedef struct Edge {
    int          to;
    int          minutes;      /* travel time on this road            */
    int          blocked;      /* 1 = closed (accident / traffic)     */
    struct Edge *next;
} Edge;

typedef struct {
    char name[32];
    int  loc;                  /* which area it is located in         */
    int  beds;                 /* free beds right now                 */
    int  capacity;
} Hospital;

typedef struct {
    int      numLoc;
    char     names[MAX_LOC][20];
    Edge    *adj[MAX_LOC];     /* adjacency list : one list per area  */
    int      numHosp;
    Hospital hosp[MAX_HOSP];
} CityMap;

/* ---- building the graph ---- */
void map_init(CityMap *m);
int  map_add_location(CityMap *m, const char *name);
int  map_add_road(CityMap *m, int a, int b, int minutes);
int  map_add_hospital(CityMap *m, const char *name, int loc, int beds);
void map_load_default(CityMap *m);
void map_free(CityMap *m);

/* ---- display ---- */
void map_print(const CityMap *m);          /* adjacency list   */
void map_print_matrix(const CityMap *m);   /* adjacency matrix */
void hospitals_print(const CityMap *m);

/* ---- operations ---- */
int  map_set_blocked(CityMap *m, int a, int b, int blocked);   /* 0 if no such road */

void bfs(const CityMap *m, int src, int hops[], int parent[]);
void print_route(const CityMap *m, const int hops[], const int parent[], int src, int dst);
int  nearest_hospital(const CityMap *m, int src, int hops[], int parent[], int verbose);

void all_routes(const CityMap *m, int src, int dst);           /* DFS + backtracking */
int  map_reachable(const CityMap *m, int src, int seen[]);     /* DFS, returns count */

#endif
