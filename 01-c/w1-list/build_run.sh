#!/bin/sh
# 一键编译并运行单链表模块（今天是多文件：list.c + test_list.c）
# 用法：  ./build_run.sh        编译 + 运行，结果同时写入 运行结果.txt
#         ./build_run.sh -c     只编译，不运行
set -u
cd "$(dirname "$0")" || exit 1

CC="${CC:-gcc}"
CFLAGS="-Wall -Wextra"
OUT="运行结果.txt"
BIN="test_list.exe"

printf '编译 %-26s ' "list.c + test_list.c"
if $CC $CFLAGS list.c test_list.c -o "$BIN" 2>build.err; then
    if [ -s build.err ]; then
        echo "OK（有警告）"
        sed 's/^/    /' build.err
    else
        echo "OK"
    fi
else
    echo "失败"
    sed 's/^/    /' build.err
    rm -f build.err
    exit 1
fi
rm -f build.err

[ "${1:-}" = "-c" ] && exit 0

./"$BIN" > run.tmp 2>&1
rc=$?
cat run.tmp

{
    echo "运行环境：$($CC --version | head -1)"
    echo "生成时间：$(date '+%Y-%m-%d %H:%M')"
    echo
    echo "===================== test_list ====================="
    cat run.tmp
    echo
} > "$OUT"
rm -f run.tmp

echo
echo "--------------------------------------------"
grep '^结果:' "$OUT" | tail -1
echo "运行结果已写入 $OUT"
exit $rc
