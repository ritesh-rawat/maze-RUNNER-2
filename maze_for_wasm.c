#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emscripten/emscripten.h> // <-- Import for WASM

static char* g_maze = NULL;
static int g_N = 0;

#define WALL '#'
#define PATH ' '
#define BFS_PATH '*'
#define DFS_PATH '.'

// --- Point struct
typedef struct {
    int x, y;
} Point;

// Stack Implementation (from stack.c/stack.h)
typedef struct StackNode {
    Point point;
    struct StackNode* next;
} StackNode;

typedef struct {
    StackNode* top;
} Stack;

Stack* createStack() {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->top = NULL;
    return stack;
}

void push(Stack* stack, Point p) {
    StackNode* node = (StackNode*)malloc(sizeof(StackNode));
    node->point = p;
    node->next = stack->top;
    stack->top = node;
}

Point pop(Stack* stack) {
    Point p = { -1, -1 };
    if (stack->top) {
        StackNode* temp = stack->top;
        p = temp->point;
        stack->top = temp->next;
        free(temp);
    }
    return p;
}

int isStackEmpty(Stack* stack) {
    return stack->top == NULL;
}

void destroyStack(Stack* stack) {
    while (!isStackEmpty(stack)) pop(stack);
    free(stack);
}

void generateMaze() {
    int i; 
    Stack* stack; 
    Point start = {1, 1}; 
    int dirs[4][2] = {{0, 2}, {0, -2}, {2, 0}, {-2, 0}}; 
    Point cur; 
    int shuffled[4]; 
    int nx, ny, j, tmp; 
    Point next_point;
    int loopsToCreate; 
    int wallsBroken; 
    int x, y; 

    for (i = 0; i < g_N * g_N; i++) {
        g_maze[i] = WALL;
    }

    stack = createStack();
    g_maze[start.y * g_N + start.x] = PATH;
    push(stack, start);
    
    srand(time(NULL));

    while (!isStackEmpty(stack)) {
        cur = pop(stack);
        shuffled[0] = 0; shuffled[1] = 1; shuffled[2] = 2; shuffled[3] = 3;
        for (i = 3; i > 0; i--) {
            j = rand() % (i + 1);
            tmp = shuffled[i];
            shuffled[i] = shuffled[j];
            shuffled[j] = tmp;
        }

        for (i = 0; i < 4; i++) {
            nx = cur.x + dirs[shuffled[i]][0];
            ny = cur.y + dirs[shuffled[i]][1];
            if (nx > 0 && ny > 0 && nx < g_N - 1 && ny < g_N - 1 && g_maze[ny * g_N + nx] == WALL) {
                g_maze[(cur.y + dirs[shuffled[i]][1] / 2) * g_N + (cur.x + dirs[shuffled[i]][0] / 2)] = PATH;
                g_maze[ny * g_N + nx] = PATH;
                push(stack, cur);
                
                next_point.x = nx; next_point.y = ny; 
                push(stack, next_point); 
                break;
            }
        }
    }
    destroyStack(stack);
    
    loopsToCreate = g_N; 
    wallsBroken = 0;

    while (wallsBroken < loopsToCreate) {
        x = rand() % (g_N - 2) + 1; 
        y = rand() % (g_N - 2) + 1; 

        if (g_maze[y * g_N + x] == WALL) {
            g_maze[y * g_N + x] = PATH;
            wallsBroken++;
        }
    }
}

// --- BFS Solver
typedef struct Node {
    int x, y;
    struct Node* parent;
} Node;

int solveMazeBFS(Point start, Point end) {
    int* visited; 
    Node** queue; 
    int front = 0, rear = 0; 
    Node* startNode; 
    int dirs[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}}; 
    Node* endNode = NULL; 
    Node* cur; 
    int i, nx, ny; 
    Node* next; 
    Node* path; 

    visited = (int*)calloc(g_N * g_N, sizeof(int));
    queue = (Node**)malloc(g_N * g_N * sizeof(Node*));
    
    startNode = (Node*)malloc(sizeof(Node));
    startNode->x = start.x;
    startNode->y = start.y;
    startNode->parent = NULL;
    queue[rear++] = startNode;
    visited[start.y * g_N + start.x] = 1;

    while (front < rear) {
        cur = queue[front++];
        if (cur->x == end.x && cur->y == end.y) {
            endNode = cur;
            break;
        }
        for (i = 0; i < 4; i++) {
            nx = cur->x + dirs[i][0];
            ny = cur->y + dirs[i][1];
            if (nx >= 0 && ny >= 0 && nx < g_N && ny < g_N && g_maze[ny * g_N + nx] != WALL && !visited[ny * g_N + nx]) {
                visited[ny * g_N + nx] = 1;
                next = (Node*)malloc(sizeof(Node));
                next->x = nx;
                next->y = ny;
                next->parent = cur;
                queue[rear++] = next;
            }
        }
    }

    if (endNode) {
        path = endNode;
        while (path != NULL) {
            if (g_maze[path->y * g_N + path->x] == PATH) { 
                 g_maze[path->y * g_N + path->x] = BFS_PATH;
            }
            path = path->parent;
        }
    }

    for (i = 0; i < rear; i++) free(queue[i]);
    free(queue);
    free(visited);
    return (endNode != NULL);
}

// --- DFS Solver
int dfsRecursive(int* visited, Point cur, Point end) {
    int dirs[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}}; 
    int i; 
    Point next; 

    if (cur.x < 0 || cur.x >= g_N || cur.y < 0 || cur.y >= g_N ||
        g_maze[cur.y * g_N + cur.x] == WALL || visited[cur.y * g_N + cur.x]) {
        return 0;
    }

    visited[cur.y * g_N + cur.x] = 1;
    if (cur.x == end.x && cur.y == end.y) return 1;

    for (i = 0; i < 4; i++) {
        next.x = cur.x + dirs[i][0]; // C99 init fix
        next.y = cur.y + dirs[i][1]; // C99 init fix
        if (dfsRecursive(visited, next, end)) {
            if (g_maze[cur.y * g_N + cur.x] == PATH) { 
                g_maze[cur.y * g_N + cur.x] = DFS_PATH;
            }
            return 1;
        }
    }
    return 0;
}

int dfsSolve(Point start, Point end) {
    int* visited; 
    int found; 

    visited = (int*)calloc(g_N * g_N, sizeof(int));
    found = dfsRecursive(visited, start, end);
    if (found) {
        if (g_maze[start.y * g_N + start.x] == PATH) {
            g_maze[start.y * g_N + start.x] = DFS_PATH;
        }
    }
    free(visited);
    return found;
}



EMSCRIPTEN_KEEPALIVE
void init_maze(int n) {
    if (g_maze != NULL) {
        free(g_maze);
    }
    g_N = n;
    g_maze = (char*)malloc(g_N * g_N * sizeof(char));
}

EMSCRIPTEN_KEEPALIVE
void run_generation() {
    if (g_maze == NULL) return;
    generateMaze();

    g_maze[1 * g_N + 1] = PATH;
    g_maze[(g_N - 2) * g_N + (g_N - 2)] = PATH;
}

EMSCRIPTEN_KEEPALIVE
int run_solver(int solver_type) {
    Point start = {1, 1};
    Point end = {g_N - 2, g_N - 2};
    int i = 0; 
    
    if (g_maze == NULL) return 0;
    
    for(i=0; i < g_N * g_N; i++) {
        if(g_maze[i] == BFS_PATH || g_maze[i] == DFS_PATH) {
            g_maze[i] = PATH;
        }
    }

    if (solver_type == 1) { // BFS
        return solveMazeBFS(start, end);
    } else if (solver_type == 2) { // DFS
        return dfsSolve(start, end);
    }
    return 0;
}

EMSCRIPTEN_KEEPALIVE
const char* get_maze_buffer() {
    return g_maze;

}
