/* =============================================================================
   MÓDULO 15 - EJERCICIO 07: Depuración — GDB/LLDB, core dumps, watchpoints
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 7 de TEMAS.md: gdb/lldb, breakpoints, watchpoints, core dumps):
     1. gdb_basic_commands(): string con comandos GDB básicos
        (run, break, continue, next, step, bt, print, info locals)
     2. gdb_watchpoint(cmd, condition): string con comando watchpoint
        (watch/rwatch/awatch) con condición opcional
     3. gdb_conditional_bp(file, line, condition): string con breakpoint
        condicional en file:line si condition es true
     4. core_dump_setup(): string con configuración para core dumps
        (ulimit, /proc/sys/kernel/core_pattern, systemd-coredump)
     5. rr_record_replay(exe): string con comandos rr record y replay

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_07_debug.cpp -o ej07 && ./ej07
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

// TODO 1: Comandos GDB básicos
std::string gdb_basic_commands() {
    // TU CÓDIGO: lista de comandos básicos separados por \n
    return "";
}

// TODO 2: Watchpoint con condición
std::string gdb_watchpoint([[maybe_unused]] const std::string& var_name,
                            [[maybe_unused]] const std::string& type,
                            [[maybe_unused]] const std::string& condition) {
    // TU CÓDIGO: watch/rwatch/awatch var if condition
    return "";
}

// TODO 3: Breakpoint condicional
std::string gdb_conditional_bp([[maybe_unused]] const std::string& file,
                                [[maybe_unused]] int line,
                                [[maybe_unused]] const std::string& condition) {
    // TU CÓDIGO: break file:line if condition
    return "";
}

// TODO 4: Core dump setup
std::string core_dump_setup() {
    // TU CÓDIGO: ulimit -c unlimited, core_pattern, systemd config
    return "";
}

// TODO 5: RR record y replay
std::string rr_record_replay([[maybe_unused]] const std::string& exe) {
    // TU CÓDIGO: rr record ./exe && rr replay
    return "";
}

// ============= TESTS =============
TEST_CASE(GDBBasicCommandsContieneEsenciales) {
    std::string cmds = gdb_basic_commands();
    ASSERT_TRUE(cmds.find("run") != std::string::npos);
    ASSERT_TRUE(cmds.find("break") != std::string::npos);
    ASSERT_TRUE(cmds.find("continue") != std::string::npos);
    ASSERT_TRUE(cmds.find("bt") != std::string::npos);
    ASSERT_TRUE(cmds.find("print") != std::string::npos);
    ASSERT_TRUE(cmds.find("info locals") != std::string::npos);
}

TEST_CASE(GDBWatchpointConCondicion) {
    std::string wp = gdb_watchpoint("mi_var", "write", "mi_var > 10");
    ASSERT_TRUE(wp.find("watch") != std::string::npos);
    ASSERT_TRUE(wp.find("mi_var") != std::string::npos);
    ASSERT_TRUE(wp.find("if") != std::string::npos);
    ASSERT_TRUE(wp.find("mi_var > 10") != std::string::npos);
}

TEST_CASE(GDBWatchpointRead) {
    std::string wp = gdb_watchpoint("ptr", "read", "");
    ASSERT_TRUE(wp.find("rwatch") != std::string::npos || wp.find("awatch") != std::string::npos);
    ASSERT_TRUE(wp.find("ptr") != std::string::npos);
}

TEST_CASE(GDBConditionalBreakpoint) {
    std::string bp = gdb_conditional_bp("main.cpp", 42, "x > 10");
    ASSERT_TRUE(bp.find("break") != std::string::npos);
    ASSERT_TRUE(bp.find("main.cpp") != std::string::npos);
    ASSERT_TRUE(bp.find("42") != std::string::npos);
    ASSERT_TRUE(bp.find("if") != std::string::npos);
    ASSERT_TRUE(bp.find("x > 10") != std::string::npos);
}

TEST_CASE(CoreDumpSetupContieneConfig) {
    std::string setup = core_dump_setup();
    ASSERT_TRUE(setup.find("ulimit -c unlimited") != std::string::npos);
    ASSERT_TRUE(setup.find("core_pattern") != std::string::npos);
    ASSERT_TRUE(setup.find("systemd-coredump") != std::string::npos || setup.find("coredump.conf") != std::string::npos);
}

TEST_CASE(RRRecordReplayContieneComandos) {
    std::string rr = rr_record_replay("./mi_programa");
    ASSERT_TRUE(rr.find("rr record") != std::string::npos);
    ASSERT_TRUE(rr.find("rr replay") != std::string::npos);
    ASSERT_TRUE(rr.find("./mi_programa") != std::string::npos);
}

int main() {
    return RUN_ALL_TESTS();
}