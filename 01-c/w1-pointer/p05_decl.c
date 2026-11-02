/* p05_decl.c —— 今天的产出题：把 int *p[3] 和 int (*p)[3] 讲清楚
 *
 * 计划里 11-02 的"当天产出"是：一段能当面讲清 int *p[3] 与 int (*p)[3]
 * 的代码。就是这一题。
 *
 * 判断法则只有一条 —— 从变量名出发，先看右边，再看左边：
 *
 *   int *p[3];      p 右边是 [3]  → p 是数组，3 个元素
 *                   再看左边 *   → 元素类型是 int *
 *                   ⇒ 3 个 int* 组成的数组（指针数组）
 *                   sizeof(p) == 3 * 8 == 24
 *
 *   int (*p)[3];    p 被括号包住，先看括号里 *  → p 是指针
 *                   再看右边 [3]                → 指向"3 个 int 的数组"
 *                   ⇒ 指向数组的指针（数组指针）
 *                   sizeof(*p) == 3 * 4 == 12
 *
 * 一句话记法：**括号改变结合顺序。无括号先跟数组走，有括号先跟星号走。**
 *
 * 为什么会有人写晕：int (*p)[3] 的 *p 就是那个数组本身，
 *   所以 (*p)[0] 才是第一个 int，而 p[0] 是"整个 int[3] 数组"。
 *
 * 嵌入式里的实例：二维数组传参
 *   void f(int a[][3], int rows)  ≡  void f(int (*a)[3], int rows)
 *   参数里的"数组"退化成指针，但指向的仍然是"一行 3 个 int"，
 *   所以第二维不能丢 —— 靠它才知道每行多长、怎么算下一行地址。
 */
#include <stdio.h>

/* ============================================================
 * TODO 1：指针数组求和 —— 形参类型是 int *const *names
 *
 * 语义：names 是"指针数组"，里面每个元素都是 int *。
 *      求 *names[i] 的总和。
 *      形参里的 const 只是说"不许改 names[i] 这个指针元素本身"。
 * ============================================================ */
int sum_ptr_array(int *const *names, int n)
{
    if ((names == NULL)||(n <= 0))
    {
        return 0;
    }

    int sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum = sum + *names[i];
    }
    return sum;

}

/* ============================================================
 * TODO 2：数组指针求和 —— 形参类型是 int (*row)[3]
 *
 * 语义：row 指向"一行 3 个 int"。访问第 i 行第 j 列：
 *           row[i][j]     或     (*(row + i))[j]
 *       因为它们等价，所以二维数组传参必须带第二维的长度。
 * 求 rows 行 3 列的矩阵总和。
 * ============================================================ */
int sum_matrix(int (*row)[3], int rows)
{
    if ((row == NULL)||(rows <= 0))
    {
        return 0;
    }

    int sum = 0;
    for (int i = 0; i < rows; i++)
    {
        int j = 0;
        for (j = 0; j < 3; j++)
        {
            sum = sum + row[i][j];
        }

    }
    return sum;
}

/* ============================================================
 * TODO 3：同一个矩阵，用"指针数组 + 二级指针"的方式求和
 *
 * 这是二维数组的另一种表达法：不是连续的 rows*3 块内存，
 * 而是 rows 个单独的 int[3]，再用一个数组把这些首地址串起来。
 *
 * 什么时候用哪种？
 *   尽量用连续二维数组（缓存友好、只 malloc 一次）；
 *   每行长度不等、或者行是分散的，才用指针数组。
 * ============================================================ */
int sum_ptr_array_2d(int *const *rows, int n)
{
    if ((rows == NULL) || (n <= 0))
    {
        return 0;
    }

    int sum = 0;

    for (int i = 0; i < n; i++)
    {
        int j = 0;
        for (j = 0; j < 3; j++)
        {
            sum = sum + *(*(rows + i) + j);
        }

    }
    return sum;
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check(const char *name, int got, int want)
{
    if (got == want) {
        printf("  [PASS] %-46s = %d\n", name, got);
        g_pass++;
    } else {
        printf("  [FAIL] %-46s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

static void check_size(const char *name, size_t got, size_t want)
{
    if (got == want) {
        printf("  [PASS] %-46s = %zu\n", name, got);
        g_pass++;
    } else {
        printf("  [FAIL] %-46s 期望 %zu，实际 %zu\n", name, want, got);
        g_fail++;
    }
}

int main(void)
{
    int a = 1, b = 2, c = 3;

    /* 指针数组：3 个 int* */
    int *ptr_arr[3] = {&a, &b, &c};

    /* 连续二维数组：3 行 3 列，内存里是连续的 9 个 int */
    int mat[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9},
    };

    /* 分散的三行，用指针数组串起来 */
    int r0[3] = {1, 2, 3};
    int r1[3] = {4, 5, 6};
    int r2[3] = {7, 8, 9};
    int *row_ptrs[3] = {r0, r1, r2};

    printf("=== p05 类型辨析：int *p[3] vs int (*p)[3] ===\n\n");

    printf("sizeof(*ptr_arr) = %zu   ← ptr_arr 的元素是 int*，8 字节\n",
           sizeof(*ptr_arr));
    printf("sizeof(ptr_arr)  = %zu   ← 3 个 int*，3*8 = 24\n\n", sizeof(ptr_arr));

    printf("sizeof(*mat)     = %zu   ← *mat 就是那一行 int[3]，3*4 = 12\n",
           sizeof(*mat));
    printf("sizeof(mat)      = %zu   ← 整个 3x3，9*4 = 36\n", sizeof(mat));
    printf("sizeof(mat[0])   = %zu   ← 第一行，同样 12\n\n", sizeof(mat[0]));

    /* mat[0] 会退化成 int *，而 &mat[0] 是 int (*)[3] —— 类型不一样 */
    printf("注意：mat[0] 是 int *（退化），&mat[0] 是 int (*)[3]（数组指针）\n");
    printf("      mat[0]      = %p\n", (const void *)mat[0]);
    printf("      (void*)&mat = %p   （同一片内存，值相同）\n\n",
           (const void *)&mat);

    printf("--- 测试 ---\n");

    check_size("sizeof(int *p[3]) 元素 = int*", sizeof(*ptr_arr), sizeof(int *));
    check_size("sizeof(int *p[3]) 整体 = 24", sizeof(ptr_arr), 24);
    check_size("sizeof(*(int (*p)[3])) = 12", sizeof(*mat), 12);
    check_size("sizeof(int (*p)[3] 指向的 3x3) = 36", sizeof(mat), 36);

    check("sum_ptr_array = 6", sum_ptr_array(ptr_arr, 3), 6);
    check("sum_ptr_array n=0 = 0", sum_ptr_array(ptr_arr, 0), 0);

    check("sum_matrix 3x3 = 45", sum_matrix(mat, 3), 45);
    check("sum_matrix 只求前 2 行 = 21", sum_matrix(mat, 2), 21);
    check("sum_matrix rows=0 = 0", sum_matrix(mat, 0), 0);

    check("sum_ptr_array_2d = 45", sum_ptr_array_2d(row_ptrs, 3), 45);
    check("sum_ptr_array_2d 前 2 行 = 21", sum_ptr_array_2d(row_ptrs, 2), 21);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
