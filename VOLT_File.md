Description byte by byte of the binary format ".volt":
======================================================

4 bytes: int32_t - number of curves in file
==== REPEATING SEQUENCE * number of curves =====
4 bytes: int32_t - length of each curve data
x bytes (until NULL): char[] - Curve name
x bytes (until NULL): char[] - Curve comment
4 bytes: int32_t - number of Param values
==== REPEATING SEQUENCE * number of Param values =======
4 bytes: int32_t - param number
4 bytes: int32_t - param value
===== END OF SEQUENCE ==================================
==== REPEATING SEQUENCE * Param(PARAM::ptnr) ===========
8 bytes: double - measurement point time valie
8 bytes: double - measurement point potential value
8 bytes: double - measurement current value
===== END OF SQUENCE ====================================
=== IF PARAM(nonaveraged) != 0 ==========================
4 bytes: int32_t size of nonaveraged data
        === REPEATING SEQUENCE * size of nonaveraged ============
        4 bytes: float - nonaveraged measurement current
        ===== END OF SEQUENCE ===================================
==== END OF IF ==========================================
=== END OF SQUENCE =================================
=== END OF FILE ====================================
