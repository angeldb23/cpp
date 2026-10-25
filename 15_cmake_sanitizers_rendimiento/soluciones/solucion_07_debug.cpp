/* =============================================================================
   MÓDULO 15 - SOLUCIÓN 07: Depuración
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

std::string gdb_basic_commands() {
    return "run\n"
           "break main\n"
           "continue\n"
           "next\n"
           "step\n"
           "bt\n"
           "print variable\n"
           "info locals";
}

std::string gdb_watchpoint(const std::string& var_name,
                            const std::string& type,
                            const std::string& condition) {
    std::string cmd;
    if (type == "write") cmd = "watch ";
    else if (type == "read") cmd = "rwatch ";
    else if (type == "access") cmd = "awatch ";
    else cmd = "watch ";
    cmd += var_name;
    if (!condition.empty()) {
        cmd += " if " + condition;
    }
    return cmd;
}

std::string gdb_conditional_bp(const std::string& file,
                                int line,
                                const std::string& condition) {
    return "break " + file + ":" + std::to_string(line) + " if " + condition;
}

std::string core_dump_setup() {
    return "ulimit -c unlimited\n"
           "echo '/tmp/core.%e.%p' > /proc/sys/kernel/core_pattern\n"
           "# /etc/systemd/coredump.conf: ProcessSizeMax=2G";
}

std::string rr_record_replay(const std::string& exe) {
    return "rr record " + exe + "\n"
           "rr replay";
}

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