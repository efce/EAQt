Description byte by byte of the binary format ".volt":
=============================================================

=== START OF FILE ===========================================
4 bytes: int32_t - number of curves in file
==== REPEATING SEQUENCE * number of curves ==================
    4 bytes: uint32_t - length of each curve data
    x bytes (until NULL): char[] UTF8 encoding - Curve name
    x bytes (until NULL): char[] UTF8 encoding - Curve comment
    4 bytes: int32_t - number of Parameters
    ==== REPEATING SEQUENCE * number of Parameters ==========
        4 bytes: int32_t - parameter value
    ===== END OF SEQUENCE ===================================
    ==== REPEATING SEQUENCE * Paramter #16 (PARAM::ptnr) ====
    ==== param number 16 number of measured points ==========
        8 bytes: double - measurement point time valie
        8 bytes: double - measurement point potential value
        8 bytes: double - measurement current value
    ===== END OF SQUENCE ====================================
    === IF Parameter #60 != 0 ===============================
    === param#60 is a frequency in kHz of sampling (0 means =
    === that no nonaveraged samples are recorded ============
        4 bytes: int32_t size of nonaveraged data
        === REPEATING SEQUENCE * size of nonaveraged ========
            4 bytes: float - nonaveraged measurement current
        ===== END OF SEQUENCE ===============================
    ==== END OF IF ==========================================
=== END OF SQUENCE ==========================================
=== END OF FILE =============================================
