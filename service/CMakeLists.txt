set(GATE_SRC service_gate.c)
include_directories(../)
add_library(gate SHARED  ${GATE_SRC})
