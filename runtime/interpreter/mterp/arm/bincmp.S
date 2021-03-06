    /*
     * Generic two-operand compare-and-branch operation.  Provide a "revcmp"
     * fragment that specifies the *reverse* comparison to perform, e.g.
     * for "if-le" you would use "gt".
     *
     * For: if-eq, if-ne, if-lt, if-ge, if-gt, if-le
     */
    /* if-cmp vA, vB, +CCCC */
    mov     r1, rINST, lsr #12          @ r1<- B
    ubfx    r0, rINST, #8, #4           @ r0<- A
    GET_VREG r3, r1                     @ r3<- vB
    GET_VREG r2, r0                     @ r2<- vA
    FETCH_S rINST, 1                    @ rINST<- branch offset, in code units
    cmp     r2, r3                      @ compare (vA, vB)
    mov${revcmp} rINST, #2
#if MTERP_PROFILE_BRANCHES
    @ TUNING: once measurements are complete, remove #if and hand-schedule.
    EXPORT_PC
    mov     r0, rSELF
    add     r1, rFP, #OFF_FP_SHADOWFRAME
    mov     r2, rINST
    bl      MterpProfileBranch          @ (self, shadow_frame, offset)
    cmp     r0, #0
    bne     MterpOnStackReplacement     @ Note: offset must be in rINST
#endif
    adds    r2, rINST, rINST            @ convert to bytes, check sign
    ldr     lr, [rSELF, #THREAD_FLAGS_OFFSET]
    FETCH_ADVANCE_INST_RB r2            @ update rPC, load rINST
    bmi     MterpCheckSuspendAndContinue
    GET_INST_OPCODE ip                  @ extract opcode from rINST
    GOTO_OPCODE ip                      @ jump to next instruction
