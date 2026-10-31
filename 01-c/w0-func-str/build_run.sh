#!/bin/sh
# 一键编译并运行 w0-func-str 下的全部练习
# 用法：  ./build_run.sh            编译 + 运行，结果同时写入 运行结果.txt
#         ./build_run.sh -c         只编译，不运行
set -u
cd "$(dirname "$0")" || exit 1

CC="${CC:-gcc}"
CFLAGS="-Wall"
OUT="运行结果.txt"

pass=0
fail=0
build_fail=0

for f in f*.c; do
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
    else
        echo "失败"
        sed 's/^/    /' build.err
        build_fail=1
    fi
    rm -f build.err
done

[ "${1:-}" = "-c" ] && exit $build_fail

{
    echo "运行环境：$($CC --version | head -1)"
    echo "生成时间：$(date '+%Y-%m-%d %H:%M')"
    echo
} > "$OUT"

echo
for bin in f*.exe; do
    [ -e "$bin" ] || continue
    name="${bin%.exe}"
    line=$(./"$bin")
    rc=$?
    echo "===================== $name =====================" >> "$OUT"
    echo "$line" >> "$OUT"
    echo >> "$OUT"

    summary=$(echo "$line" | grep '^结果:' | tail -1)
    if [ $rc -eq 0 ]; then
        printf '%-20s %s\n' "$name" "${summary:-通过}"
        pass=$((pass+1))
    else
        printf '%-20s %s\n' "$name" "${summary:-有失败}"
        fail=$((fail+1))
    fi
done

echo
echo "--------------------------------------------"
echo "全过 $pass 个，未全过 $fail 个"
echo "运行结果已写入 $OUT"
[ $fail -eq 0 ] && [ $build_fail -eq 0 ]
