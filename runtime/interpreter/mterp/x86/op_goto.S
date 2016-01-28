/*
 * Unconditional branch, 8-bit offset.
 *
 * The branch distance is a signed code-unit offset, which we need to
 * double to get a byte offset.
 */
    /* goto +AA */
    movsbl  rINSTbl, %eax                   # eax <- ssssssAA
    addl    %eax, %eax                      # eax <- AA * 2
    leal    (rPC, %eax), rPC
    FETCH_INST
    jg      1f                              # AA * 2 > 0 => no suspend check
#if MTERP_SUSPEND
    REFRESH_IBASE
#else
    jmp     MterpCheckSuspendAndContinue
#endif
1:
    GOTO_NEXT