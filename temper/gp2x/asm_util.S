.global invalidate_cache_region

invalidate_cache_region:
  mov r2, #0x0
  swi 0x9f0002

  bx lr

