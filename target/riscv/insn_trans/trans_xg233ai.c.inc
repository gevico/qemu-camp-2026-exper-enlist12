static bool trans_dma(DisasContext *ctx, arg_reg *a) {
    TCGv dst_addr = get_gpr(ctx, a->rd, EXT_NONE);
    TCGv src_addr = get_gpr(ctx, a->rs1, EXT_NONE);
    TCGv selector = get_gpr(ctx, a->rs2, EXT_NONE);

    if (!ctx->cfg_ptr->ext_Xg233ai) {
        return false;
    }

    gen_helper_xg233ai_dma(tcg_env, dst_addr, src_addr, selector);
    return true;
}

static bool trans_sort(DisasContext *ctx, arg_reg *a) {
    TCGv num = get_gpr(ctx, a->rd, EXT_NONE);
    TCGv addr = get_gpr(ctx, a->rs1, EXT_NONE);

    if (!ctx->cfg_ptr->ext_Xg233ai) {
        return false;
    }

    gen_helper_xg233ai_sort(tcg_env, addr, num);
    return true;
}

static bool trans_crush(DisasContext *ctx, arg_reg *a) {
    TCGv dst_addr = get_gpr(ctx, a->rd, EXT_NONE);
    TCGv src_addr = get_gpr(ctx, a->rs1, EXT_NONE);
    TCGv num = get_gpr(ctx, a->rs2, EXT_NONE);

    if (!ctx->cfg_ptr->ext_Xg233ai) {
        return false;
    }

    gen_helper_xg233ai_crush(tcg_env, dst_addr, src_addr, num);
    return true;
}

static bool trans_expand(DisasContext *ctx, arg_reg *a) {
    TCGv dst_addr = get_gpr(ctx, a->rd, EXT_NONE);
    TCGv src_addr = get_gpr(ctx, a->rs1, EXT_NONE);
    TCGv num = get_gpr(ctx, a->rs2, EXT_NONE);

    if (!ctx->cfg_ptr->ext_Xg233ai) {
        return false;
    }

    gen_helper_xg233ai_expand(tcg_env, dst_addr, src_addr, num);
    return true;
}

static bool trans_vdot(DisasContext *ctx, arg_reg *a) {
    TCGv array1 = get_gpr(ctx, a->rs1, EXT_NONE);
    TCGv array2 = get_gpr(ctx, a->rs2, EXT_NONE);
    TCGv res = tcg_temp_new();

    if (!ctx->cfg_ptr->ext_Xg233ai) {
        return false;
    }

    gen_helper_xg233ai_vdot(res, tcg_env, array1, array2);
    gen_set_gpr(ctx, a->rd, res);
    return true;
}

static bool trans_vrelu(DisasContext *ctx, arg_reg *a) {
    TCGv dst_addr = get_gpr(ctx, a->rd, EXT_NONE);
    TCGv src_addr = get_gpr(ctx, a->rs1, EXT_NONE);
    TCGv num = get_gpr(ctx, a->rs2, EXT_NONE);

    if (!ctx->cfg_ptr->ext_Xg233ai) {
        return false;
    }

    gen_helper_xg233ai_vrelu(tcg_env, dst_addr, src_addr, num);
    return true;
}

static bool trans_vscale(DisasContext *ctx, arg_reg *a) {
    TCGv dst_addr = get_gpr(ctx, a->rd, EXT_NONE);
    TCGv src_addr = get_gpr(ctx, a->rs1, EXT_NONE);
    TCGv num = get_gpr(ctx, a->rs2, EXT_NONE);

    if (!ctx->cfg_ptr->ext_Xg233ai) {
        return false;
    }

    gen_helper_xg233ai_vscale(tcg_env, dst_addr, src_addr, num);
    return true;
}

static bool trans_vmax(DisasContext *ctx, arg_reg *a) {
    TCGv array1 = get_gpr(ctx, a->rs1, EXT_NONE);
    TCGv num = get_gpr(ctx, a->rs2, EXT_NONE);
    TCGv res = tcg_temp_new();

    if (!ctx->cfg_ptr->ext_Xg233ai) {
        return false;
    }

    gen_helper_xg233ai_vmax(res, tcg_env, array1, num);
    gen_set_gpr(ctx, a->rd, res);
    return true;
}

static bool trans_vadd(DisasContext *ctx, arg_reg *a) {
    TCGv dst_addr = get_gpr(ctx, a->rd, EXT_NONE);
    TCGv src1_addr = get_gpr(ctx, a->rs1, EXT_NONE);
    TCGv src2_addr = get_gpr(ctx, a->rs2, EXT_NONE);

    if (!ctx->cfg_ptr->ext_Xg233ai) {
        return false;
    }

    gen_helper_xg233ai_vadd(tcg_env, src1_addr, src2_addr, dst_addr);
    return true;
}

static bool trans_gemm(DisasContext *ctx, arg_reg *a) {
    TCGv dst_addr = get_gpr(ctx, a->rd, EXT_NONE);
    TCGv src1_addr = get_gpr(ctx, a->rs1, EXT_NONE);
    TCGv src2_addr = get_gpr(ctx, a->rs2, EXT_NONE);

    if (!ctx->cfg_ptr->ext_Xg233ai) {
        return false;
    }

    gen_helper_xg233ai_gemm(tcg_env, src1_addr, src2_addr, dst_addr);
    return true;
}