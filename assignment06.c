
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_INSERT 100
#define N_SEARCH 50
#define MAX_VAL  1000

typedef struct Node {
    int key;
    struct Node *left, *right;
    int height;            /* AVL에서만 사용 (노드 수 기준) */
} Node;

static Node *new_node(int key) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->key = key;
    n->left = n->right = NULL;
    n->height = 1;
    return n;
}

/* ---------------- BST ---------------- */
static Node *bst_insert(Node *root, int key, long *cmp, int *inserted) {
    if (root == NULL) { *inserted = 1; return new_node(key); }
    (*cmp)++;
    if (key < root->key)      root->left  = bst_insert(root->left,  key, cmp, inserted);
    else if (key > root->key) root->right = bst_insert(root->right, key, cmp, inserted);
    else                      *inserted = 0;   /* 중복 */
    return root;
}

static int bst_search(Node *root, int key, long *cmp) {
    while (root != NULL) {
        (*cmp)++;
        if (key == root->key) return 1;
        root = (key < root->key) ? root->left : root->right;
    }
    return 0;
}

static int tree_height(Node *n) {   /* BST용: 실제로 재귀 계산 */
    if (n == NULL) return 0;
    int l = tree_height(n->left), r = tree_height(n->right);
    return 1 + (l > r ? l : r);
}

/* ---------------- AVL ---------------- */
static int h(Node *n) { return n ? n->height : 0; }
static int max_i(int a, int b) { return a > b ? a : b; }
static void update(Node *n) { n->height = 1 + max_i(h(n->left), h(n->right)); }
static int balance(Node *n) { return n ? h(n->left) - h(n->right) : 0; }

static Node *rotate_right(Node *y) {   /* LL 처리 */
    Node *x = y->left;
    y->left = x->right;
    x->right = y;
    update(y); update(x);
    return x;
}

static Node *rotate_left(Node *x) {    /* RR 처리 */
    Node *y = x->right;
    x->right = y->left;
    y->left = x;
    update(x); update(y);
    return y;
}

static Node *avl_insert(Node *root, int key, long *cmp, int *inserted) {
    if (root == NULL) { *inserted = 1; return new_node(key); }
    (*cmp)++;
    if (key < root->key)      root->left  = avl_insert(root->left,  key, cmp, inserted);
    else if (key > root->key) root->right = avl_insert(root->right, key, cmp, inserted);
    else { *inserted = 0; return root; }   /* 중복: 구조 변화 없음 */

    update(root);
    int bf = balance(root);

    if (bf > 1 && balance(root->left) >= 0) {            /* LL */
        return rotate_right(root);
    }
    if (bf > 1 && balance(root->left) < 0) {             /* LR */
        root->left = rotate_left(root->left);
        return rotate_right(root);
    }
    if (bf < -1 && balance(root->right) <= 0) {          /* RR */
        return rotate_left(root);
    }
    if (bf < -1 && balance(root->right) > 0) {           /* RL */
        root->right = rotate_right(root->right);
        return rotate_left(root);
    }
    return root;
}

static void free_tree(Node *n) {
    if (!n) return;
    free_tree(n->left); free_tree(n->right); free(n);
}

/* ---------------- Array ---------------- */
static int seq_search(const int *a, int n, int key, long *cmp) {
    for (int i = 0; i < n; i++) {
        (*cmp)++;
        if (a[i] == key) return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    unsigned int seed = (argc > 1) ? (unsigned int)strtoul(argv[1], NULL, 10)
                                   : (unsigned int)time(NULL);
    srand(seed);
    printf("Random seed : %u\n\n", seed);

    int generated[N_INSERT];
    int arr[N_INSERT], arr_len = 0;
    Node *bst = NULL, *avl = NULL;
    long arr_cmp = 0, bst_cmp = 0, avl_cmp = 0;
    int dup_count = 0;

    for (int i = 0; i < N_INSERT; i++) {
        int v = rand() % (MAX_VAL + 1);
        generated[i] = v;

        /* 배열: 순차 탐색으로 중복 확인 후 끝에 추가 (정렬하지 않음) */
        if (!seq_search(arr, arr_len, v, &arr_cmp)) arr[arr_len++] = v;
        else dup_count++;

        int ins_b = 0, ins_a = 0;
        bst = bst_insert(bst, v, &bst_cmp, &ins_b);
        avl = avl_insert(avl, v, &avl_cmp, &ins_a);
    }

    printf("Generated %d integers:\n", N_INSERT);
    for (int i = 0; i < N_INSERT; i++)
        printf("%4d%s", generated[i], (i % 10 == 9) ? "\n" : " ");

    printf("\nStored values     : %d\n", arr_len);
    printf("Duplicates skipped: %d\n\n", dup_count);

    printf("Construction\n");
    printf("Array comparisons : %ld\n", arr_cmp);
    printf("BST comparisons   : %ld\n", bst_cmp);
    printf("AVL comparisons   : %ld\n\n", avl_cmp);

    int bst_h = tree_height(bst);
    int avl_h = tree_height(avl);   /* 저장된 height 필드가 아닌 실제 구조로 재계산 */
    printf("Structure\n");
    printf("Array length : %d\n", arr_len);
    printf("BST height   : %d\n", bst_h);
    printf("AVL height   : %d (stored field: %d)\n\n", avl_h, h(avl));

    int keys[N_SEARCH];
    for (int i = 0; i < N_SEARCH; i++) keys[i] = rand() % (MAX_VAL + 1);

    printf("Search keys (%d):\n", N_SEARCH);
    for (int i = 0; i < N_SEARCH; i++)
        printf("%4d%s", keys[i], (i % 10 == 9) ? "\n" : " ");
    printf("\n");

    long tot_s = 0, tot_b = 0, tot_a = 0;
    int found_cnt = 0;
    for (int i = 0; i < N_SEARCH; i++) {
        long cs = 0, cb = 0, ca = 0;
        int rs = seq_search(arr, arr_len, keys[i], &cs);
        int rb = bst_search(bst, keys[i], &cb);
        int ra = bst_search(avl, keys[i], &ca);   /* AVL도 BST 탐색과 동일 */
        if (rs != rb || rs != ra) { printf("ERROR: 탐색 결과 불일치!\n"); return 1; }
        if (rs) found_cnt++;
        tot_s += cs; tot_b += cb; tot_a += ca;

        printf("Search Key : %d\n\n", keys[i]);
        printf("Sequential Search\nResult      : %s\nComparisons : %ld\n\n", rs ? "Found" : "Not Found", cs);
        printf("BST Search\nResult      : %s\nComparisons : %ld\n\n", rb ? "Found" : "Not Found", cb);
        printf("AVL Search\nResult      : %s\nComparisons : %ld\n", ra ? "Found" : "Not Found", ca);
        printf("----------------------------------------\n");
    }

    printf("\nSearches : %d (found %d, not found %d)\n\n", N_SEARCH, found_cnt, N_SEARCH - found_cnt);
    printf("Sequential Search\nTotal comparisons   : %ld\nAverage comparisons : %.2f\n\n", tot_s, (double)tot_s / N_SEARCH);
    printf("BST Search\nTotal comparisons   : %ld\nAverage comparisons : %.2f\n\n", tot_b, (double)tot_b / N_SEARCH);
    printf("AVL Search\nTotal comparisons   : %ld\nAverage comparisons : %.2f\n", tot_a, (double)tot_a / N_SEARCH);

    free_tree(bst);
    free_tree(avl);
    return 0;
}
