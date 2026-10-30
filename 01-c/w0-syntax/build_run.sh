#!/bin/sh
# 一键编译并运行 w0-syntax 下的全部练习
# 用法：  ./build_run.sh            编译 + 运行，结果同时写入 运行结果.txt
#         ./build_run.sh -c         只编译，不运行
set -u
cd "$(dirname "$0")" || exit 1

CC="${CC:-gcc}"
CFLAGS="-Wall"
OUT="运行结果.txt"

# 每个程序需要的输入（没有输入就留空）
stdin_for() {
    case "$1" in
        p01_temp*)    echo "37" ;;          # 摄氏 37 度
        p02_base*)    echo "A5" ;;          # 16 进制字符串
        p03_bits*)    printf '0xA5\n3\n' ;; # 初值 + 位号
        *)            echo "" ;;
    esac
}

fail=0
for f in p*.c; do
    [ -e "$f" ] || continue
    bin="${f%.c}.exe"
    printf '编译 %-20s ' "$f"
    if $CC $CFLAGS "$f" -o "$bin" 2>build.err; then
        if [ -s build.err ]; then
            echo "OK（有警告）"
            sed 's/^/    /' build.err
        else
            echo "OK"
        fi
        rm -f build.err
    else
        echo "失败"
        sed 's/^/    /' build.err
        rm -f build.err
        fail=1
    fi
done

[ "${1:-}" = "-c" ] && exit $fail

{
    echo "运行环境：$($CC --version | head -1)"
    echo "生成时间：$(date '+%Y-%m-%d %H:%M')"
    echo "说明：p09 含未定义行为，结果可能随编译器/优化级别变化"
    echo
} > "$OUT"

for bin in p*.exe; do
    [ -e "$bin" ] || continue
    name="${bin%.exe}"
    echo "===================== $name =====================" >> "$OUT"
    stdin_for "$name" | ./"$bin" >> "$OUT" 2>&1
    echo >> "$OUT"
done

echo
echo "运行结果已写入 $OUT"
exit $fail
