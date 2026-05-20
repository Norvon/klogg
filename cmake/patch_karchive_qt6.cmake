if(NOT DEFINED KARCHIVE_SOURCE_DIR)
  message(FATAL_ERROR "KARCHIVE_SOURCE_DIR is required")
endif()

set(karchive_cpp "${KARCHIVE_SOURCE_DIR}/karchive/src/karchive.cpp")
set(kar_cpp "${KARCHIVE_SOURCE_DIR}/karchive/src/kar.cpp")
set(krcc_cpp "${KARCHIVE_SOURCE_DIR}/karchive/src/krcc.cpp")

file(READ "${karchive_cpp}" karchive_contents)
string(REPLACE
  "tr(\"Could not set device mode to %1\").arg(mode)"
  "tr(\"Could not set device mode to %1\").arg(mode.toInt())"
  karchive_contents
  "${karchive_contents}"
)
string(REPLACE
  "tr(\"Unsupported mode %1\").arg(d->mode)"
  "tr(\"Unsupported mode %1\").arg(d->mode.toInt())"
  karchive_contents
  "${karchive_contents}"
)
file(WRITE "${karchive_cpp}" "${karchive_contents}")

file(READ "${kar_cpp}" kar_contents)
string(REPLACE
  "tr(\"Unsupported mode %1\").arg(mode)"
  "tr(\"Unsupported mode %1\").arg(mode.toInt())"
  kar_contents
  "${kar_contents}"
)
file(WRITE "${kar_cpp}" "${kar_contents}")

file(READ "${krcc_cpp}" krcc_contents)
string(REPLACE
  "tr(\"Unsupported mode %1\").arg(mode)"
  "tr(\"Unsupported mode %1\").arg(mode.toInt())"
  krcc_contents
  "${krcc_contents}"
)
file(WRITE "${krcc_cpp}" "${krcc_contents}")
