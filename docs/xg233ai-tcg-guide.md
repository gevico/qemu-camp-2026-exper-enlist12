# Xg233ai TCG 翻译指南

这份说明总结了如何在这个仓库里为 QEMU TCG 添加一个自定义 RISC-V 指令扩展，并结合当前 `xg233ai` 的实现背景进行说明。

## 1. 整体图景

QEMU TCG 处理一条 guest 指令时，分成两个阶段：

1. 翻译阶段
   QEMU 对 guest 指令做译码，并生成 TCG IR。
2. 执行阶段
   已翻译好的 TB 运行。TCG IR 被执行，如果其中有 helper 调用，则在此阶段真正调用 helper。

这意味着：

- `trans_*()` 函数运行在翻译阶段。
- `helper_*()` 函数运行在执行阶段。

这是做 TCG 开发时最重要的区别。

## 2. 给一个新的 RISC-V 扩展通常要改哪些文件

对于一个新的 vendor/custom 扩展，例如 `Xg233ai`，通常会涉及这些文件：

- `target/riscv/cpu_cfg_fields.h.inc`
  增加扩展开关位，例如 `BOOL_FIELD(ext_Xg233ai)`。
- `target/riscv/cpu.c`
  注册扩展名和 CPU 属性。
- `target/riscv/xg233ai.decode`
  定义 decodetree 的 pattern。
- `target/riscv/insn_trans/trans_xg233ai.c.inc`
  实现翻译函数，例如 `trans_dma()`。
- `target/riscv/helper.h`
  如果需要 helper，在这里声明 helper 签名。
- `target/riscv/op_helper.c`
  实现运行时 helper 逻辑。
- `target/riscv/meson.build`
  把 `.decode` 文件接入 decodetree 生成流程。
- `target/riscv/translate.c`
  include 生成的 decoder 和翻译文件，并加入 decoder 分发逻辑。

## 3. 翻译流程

一条指令完整的路径如下：

1. `riscv_translate_code()` 启动 TB 翻译。
2. `riscv_tr_translate_insn()` 处理一条 guest 指令。
3. `decode_opc()` 取指。
4. 某个 decoder，例如 `decode_Xg233ai()`，匹配 opcode。
5. decoder 调用匹配到的 `trans_*()`。
6. `trans_*()` 生成 TCG IR，或者生成一条 helper 调用。

在这棵源码树里，最上层入口在 `target/riscv/translate.c`。

## 4. `.decode` 是怎么和 `trans_*()` 连起来的

Decodetree 通过命名约定把 pattern 和 translator 绑定起来。

例如：

```text
dma   0000110 ..... ..... 110 ..... 1111011    @r
```

如果 pattern 名字叫 `dma`，decodetree 就会生成调用：

```c
trans_dma(ctx, ...)
```

所以这个映射是自动的：

- `.decode` 里的 pattern 名：`foo`
- 对应 translator 函数名：`trans_foo`

这个关系不是在 `translate.c` 里一条条手工注册的。

## 5. `ext_Xg233ai` 表示什么

像下面这样的字段：

```c
BOOL_FIELD(ext_Xg233ai)
```

表示的是 CPU 是否开启了这个扩展，而不是某一条单独的指令。

在 translator 里的典型用法是：

```c
if (!ctx->cfg_ptr->ext_Xg233ai) {
    return false;
}
```

含义是：

- 指令 pattern 已经匹配到了
- 但当前 CPU 没有开启这个扩展
- 所以这条指令不能按这个扩展去翻译

## 6. `TCGv` 和 `target_ulong`

这是最容易混淆的点之一。

### `TCGv`

`TCGv` 是 TCG IR 里的操作数或虚拟寄存器句柄，它不是 guest 的真实值本身。

你在翻译代码里这样使用它：

```c
TCGv src = get_gpr(ctx, a->rs1, EXT_NONE);
```

### `target_ulong`

`target_ulong` 是普通的 C 整数类型，用来表示“运行时的 guest 字长整数值”。

你在 helper 里这样使用它：

```c
void helper_xg233ai_dma(CPURISCVState *env,
                        target_ulong dst_addr,
                        target_ulong src_addr,
                        target_ulong selector)
```

### 基本规则

- 在 `trans_*()` 里主要操作 `TCGv`
- 在 `helper_*()` 里操作真正的 C 值，例如 `target_ulong`

不要把 `TCGv` 当成 `int` 或 `target_ulong` 直接用。

错误示例：

```c
TCGv x = get_gpr(ctx, a->rs1, EXT_NONE);
if (x == 0) { ... }
```

正确做法：

- 要么生成对应的条件 IR
- 要么把这段逻辑放进 helper

## 7. 为什么需要 helper

Helper 是一种运行时的 C 函数，TCG 可以在生成的代码里调用它。

涉及两部分：

1. 在 `helper.h` 里声明

```c
DEF_HELPER_4(xg233ai_dma, void, env, tl, tl, tl)
```

2. 在 `op_helper.c` 里实现

```c
void helper_xg233ai_dma(CPURISCVState *env,
                        target_ulong dst_addr,
                        target_ulong src_addr,
                        target_ulong selector)
{
    ...
}
```

在翻译阶段，你不是直接调用 `helper_xg233ai_dma()`，而是发出一条 helper call IR：

```c
gen_helper_xg233ai_dma(tcg_env, dst_addr, src_addr, selector);
```

它的含义是：

- 翻译阶段：生成一条“调用 helper”的 IR
- 执行阶段：TCG 真正调用 `helper_xg233ai_dma()`，并传入运行时的值

## 8. Helper 的重要约束

`helper.h` 里的声明和 `op_helper.c` 里的实现必须匹配。

例如下面这条声明：

```c
DEF_HELPER_4(xg233ai_dma, void, env, tl, tl, tl)
```

对应的实现就必须是：

```c
void helper_xg233ai_dma(CPURISCVState *env,
                        target_ulong a,
                        target_ulong b,
                        target_ulong c)
```

这不是普通 C 的隐式转换规则，而是 TCG 的 helper 调用约定。

## 9. 翻译侧常用函数

### 读写 GPR

```c
TCGv src = get_gpr(ctx, a->rs1, EXT_NONE);
TCGv dst = dest_gpr(ctx, a->rd);
gen_set_gpr(ctx, a->rd, dst);
```

### 常量

```c
TCGv zero = tcg_constant_tl(0);
```

### 临时变量

```c
TCGv tmp = tcg_temp_new();
```

### 算术和位运算

```c
tcg_gen_add_tl(dst, a, b);
tcg_gen_sub_tl(dst, a, b);
tcg_gen_and_tl(dst, a, b);
tcg_gen_or_tl(dst, a, b);
tcg_gen_xor_tl(dst, a, b);
tcg_gen_shli_tl(dst, a, 2);
tcg_gen_shri_tl(dst, a, 2);
```

### 在翻译阶段生成 guest 内存访问

```c
tcg_gen_qemu_ld_tl(val, addr, ctx->mem_idx, MO_TEUW);
tcg_gen_qemu_st_tl(val, addr, ctx->mem_idx, MO_TEUW);
```

这些函数生成的是访存 IR，不是在翻译阶段真的去读写内存。

## 10. 运行时常用访存函数

在 helper 里，不应该再用 `tcg_gen_qemu_*`，而是用运行时的 MMU 感知访存函数。

例如：

```c
uint32_t value = cpu_ldl_le_data_ra(env, addr, ra);
cpu_stl_le_data_ra(env, addr, value, ra);
```

这些函数在运行时真正访问 guest 内存，并且能够正确抛出异常。

## 11. 什么时候用纯 TCG，什么时候用 helper

适合用纯 TCG 的情况：

- 指令语义简单
- 固定规模逻辑用 `tcg_gen_*` 容易表达
- 控制流很小

适合用 helper 的情况：

- 循环次数依赖运行时值
- 访存流程比较复杂
- 用 C 写明显更直观
- 指令语义比较大、比较杂

### 为什么 `dma` 适合 helper

对于这种逻辑：

```text
N = {8, 16, 32}[gpr[rs2]]
for i in 0..N-1:
    for j in 0..N-1:
        dst[j * N + i] = src[i * N + j]
```

纯 TCG 写起来会很别扭，因为：

- `N` 依赖运行时寄存器值
- 循环次数依赖运行时寄存器值
- 中间包含大量 load/store

这就是非常典型的 helper 场景。

## 12. 这棵树里 `dma` 指令的语义

当前语义约定是：

- `rd` 保存目标基地址
- `rs1` 保存源基地址
- `rs2` 是选择子：
  - `0 -> N = 8`
  - `1 -> N = 16`
  - `2 -> N = 32`
- 每个元素按 FP32 处理，也就是 4 字节
- 执行的是转置拷贝：

```text
dst[j * N + i] = src[i * N + j]
```

因此 translator 可以保持很薄：

```c
static bool trans_dma(DisasContext *ctx, arg_reg *a)
{
    TCGv dst_addr = get_gpr(ctx, a->rd, EXT_NONE);
    TCGv src_addr = get_gpr(ctx, a->rs1, EXT_NONE);
    TCGv selector = get_gpr(ctx, a->rs2, EXT_NONE);

    if (!ctx->cfg_ptr->ext_Xg233ai) {
        return false;
    }

    gen_helper_xg233ai_dma(tcg_env, dst_addr, src_addr, selector);
    return true;
}
```

而真正的双重循环放到 helper 中执行。

## 13. 在 `trans_*()` 里不能做的事

下面这些都不应该出现在 translator 里：

- 把 `TCGv` 当作 C 数组下标
- 在 C 的 `if` 里直接判断 `TCGv`
- 在 C 的 `for` 里直接用 `TCGv` 做循环上界
- 把 `TCGv` 当作已知的具体整数值

错误示例：

```c
int n = selector;
if (selector == 0) { ... }
for (int i = 0; i < selector; i++) { ... }
```

如果 `selector` 是 `TCGv`，上面这些都是错的。

## 14. 一些实用小技巧

### 让 translator 保持薄

如果一条指令的语义比较大，就让 `trans_*()` 只负责：

- 取参数
- 检查扩展开关
- 调 helper

复杂逻辑尽量移到 helper。

### 复用已有参数集

如果你的指令格式是标准的 `rd/rs1/rs2`，优先复用现有参数集，例如 `&reg` 或 `&r`。

### 多参考已有 vendor 扩展

这棵树里比较适合参考的有：

- `target/riscv/XVentanaCondOps.decode`
- `target/riscv/insn_trans/trans_xventanacondops.c.inc`
- `target/riscv/xthead.decode`
- `target/riscv/insn_trans/trans_xthead.c.inc`

### Helper 里优先用 MMU-aware 访存

如果 helper 要访问 guest 内存，优先使用 guest 访存 helper，这样访存异常才能正确处理。

### 选择子语义要写清楚

如果某个寄存器不是直接表示值，而是表示模式，例如：

- `0 -> 8`
- `1 -> 16`
- `2 -> 32`

那就把这层映射写清楚，不要留成“魔法值”。

## 15. 新增一条指令的检查清单

1. 确认扩展开关位已经存在，或者先补上。
2. 在 `target/riscv/cpu.c` 注册扩展属性。
3. 在 `xg233ai.decode` 增加 pattern。
4. 在 `trans_xg233ai.c.inc` 增加或修改 translator。
5. 如果需要 helper，在 `helper.h` 里声明。
6. 在 `op_helper.c` 里实现 helper。
7. 在 `meson.build` 里接入 decoder 生成。
8. 在 `translate.c` 里 include decoder 和 translator。
9. 确认 CPU model 或命令行可以启用该扩展。
10. 用最小 guest 程序做编译和运行验证。

## 16. 总结

最重要的规则只有几条：

- `trans_*()` 负责生成 IR，不负责真正执行指令
- `helper_*()` 在运行时执行，并拿到真实值
- `TCGv` 不是普通 C 整数
- 复杂、运行时依赖强的语义通常更适合放进 helper
- `helper.h` 里的声明和 `op_helper.c` 里的实现必须严格匹配

