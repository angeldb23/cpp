/* =============================================================================
   MÓDULO 15 - LECCIÓN 08: Depuración avanzada — RR, scripting GDB/LLDB,
   debugging en producción, contenedores, eBPF
   =============================================================================
   TEMA 7 de TEMAS.md (2da lección): Depuración avanzada.

   RR (RECORD AND REPLAY) — TIME-TRAVEL DEBUGGING:
   - rr record ./exe          # Graba ejecución determinística
   - rr replay                # Replay con GDB interface
   - Reverse: rc (reverse-continue), rn (reverse-next), rs (reverse-step)
   - Breakpoints/watchpoints funcionan en replay
   - Permite: ir HACIA ATRÁS desde crash hasta causa raíz
   - Usos: heisenbugs, race conditions, memory corruption

   GDB PYTHON SCRIPTING:
   - Python API para pretty-printers, comandos custom
   - ~/.gdbinit: define hooks, printers STL
   - define hook-stop: código que corre en cada parada
   - gdb -ex "source script.py"

   PRETTY PRINTERS:
   - libstdc++ pretty printers (built-in GDB 7+)
   - boost, qt, eigen, fmt, spdlog printers
   - set print pretty on
   - set print elements 0 (unlimited array elements)

   CONDITIONAL BREAKPOINTS AVANZADOS:
   - break file.cpp:123 if x > 10 && y == nullptr
   - break func if $_streq($arg0, "error")
   - break *0x401000 if $rax == 0

   WATCHPOINTS HARDWARE vs SOFTWARE:
   - Hardware: 4 en x86 (debug registers), rápidos
   - Software: single-step emulation, lentos
   - awatch (access), rwatch (read), watch (write)

   CORE DUMPS EN CONTAINERS:
   - docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined
   - /proc/sys/kernel/core_pattern: core.%e.%p
   - coredumpctl list / info / gdb / debug

   POST-MORTEM EN PRODUCCIÓN:
   - ulimit -c unlimited en systemd/service
   - /etc/systemd/coredump.conf: ProcessSizeMax=2G
   - coredumpctl gdb PID
   - gdb -ex "set sysroot /path" -ex "bt" ./exe core

   DEBUGGING SIN GDB EN PROD:
   - Logging estructurado (spdlog + fmt + correlation IDs)
   - Métricas: Prometheus, statsd, OpenTelemetry
   - Distributed tracing: Jaeger, Zipkin, Tempo
   - eBPF / bpftrace: observabilidad kernel-level

   DEBUGGING EN CONTAINERS/K8S:
   - kubectl debug -it pod --image=busybox --target=container
   - kubectl cp pod:/path/core /local/core
   - kubectl exec -it pod -- gdb /proc/PID/exe PID

   POST-MORTEM SIN CORE DUMP:
   - Logging exhaustivo + correlation IDs
   - Replay de requests (grabar request/response)
   - Feature flags para activar debug logging on-demand

   DEBUGGING EN K8S:
   - kubectl debug node/name -it --image=ubuntu -- bash
   - Ephemeral containers: kubectl debug pod -it --image=gdb --target=app
   - kopia/velero para backup/restore estado

   REMOTE DEBUGGING:
   - gdbserver :1234 ./exe
   - gdb -ex "target remote host:1234"
   - VS Code: "request": "attach", "connect": { "host": "...", "port": 1234 } */

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>

// Demos de técnicas avanzadas

void demo_rr_concept() {
    std::cout << "--- RR (Record and Replay) ---\n";
    std::cout << R"(
# Time-travel debugging
# Instalación: apt install rr / brew install rr

# 1. Grabar ejecución (determinística)
rr record ./mi_programa arg1 arg2

# 2. Replay (siempre idéntico)
rr replay

# En rr replay (GDB interface):
# reverse-continue (rc)    # Continuar hacia ATRÁS
# reverse-next (rn)        # Paso hacia atrás
# reverse-step (rs)
# reverse-finish

# Breakpoints normales funcionan en replay
# Watchpoints funcionan en replay

# Usos:
# - Heisenbugs (bugs que desaparecen al debuggear)
# - Race conditions (ir hacia atrás al race)
# - Memory corruption (ver quién escribió)
# - Flaky tests (reproducir fallo exacto)

# Limitaciones:
# - Solo Linux (usa ptrace + perf)
# - Overhead ~1.2x en record
# - No soporta multi-process bien (fork)
)" << "\n\n";
}

void demo_gdb_python() {
    std::cout << "--- GDB Python Scripting ---\n";
    std::cout << R"PYTHON(
# ~/.gdbinit - Pretty printers y hooks
python
import gdb

class MiClasePrinter:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        return "MiClase(x=" + str(self.val['x']) + ", y=" + str(self.val['y']) + ")"
    def children(self):
        return [('x', self.val['x']), ('y', self.val['y'])]

def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("mis_tipos")
    pp.add_printer("MiClase", "^MiClase$", MiClasePrinter)
    return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer())

# Hook en cada parada (comentado para evitar error de compilacion)
# define hook-stop
#     python
#         # Auto-print variables importantes
#         gdb.execute('info locals')
#     end
# end
)PYTHON" << "\n\n";
}

void demo_pretty_printers() {
    std::cout << "--- Pretty Printers ---\n";
    std::cout << R"PP(
# Built-in (GDB 7+):
# libstdc++ pretty printers: vector, map, string, shared_ptr, unique_ptr
set print pretty on
set print elements 0    # Unlimited array elements

# Boost pretty printers
python
import sys
sys.path.insert(0, '/usr/share/gdb/boost')
from boost import printers
printers.register_printers(gdb.current_objfile())
end

# fmt/SPDLog pretty printers
# https://github.com/fmtlib/fmt/tree/master/support/gdb

# En GDB:
# (gdb) info pretty-printer
# (gdb) print mi_vector
# $1 = std::vector of length 3, capacity 4 = {1, 2, 3}
)PP" << "\n\n";
}

void demo_conditional_bp() {
    std::cout << "--- Conditional Breakpoints avanzados ---\n";
    std::cout << R"(
# GDB
break file.cpp:123 if x > 10 && y == nullptr
break mi_func if $_streq($arg0, "error")
break *0x401000 if $rax == 0

# Comandos en breakpoint
break mi_func
commands
  > silent
  > printf "mi_func called with x=%d\n", x
  > continue
end

# LLDB
breakpoint set --name func --condition "x > 10"
breakpoint set --file file.cpp --line 123 --condition "x > 10"
breakpoint command add 1
  > frame variable
  > continue
DONE
)" << "\n\n";
}

void demo_watchpoints() {
    std::cout << "--- Watchpoints (Hardware vs Software) ---\n";
    std::cout << R"(
# Hardware watchpoints (x86: 4 debug registers)
# - Rapidos, usan debug registers (DR0-DR3)
# - watch var        # write
# - rwatch var       # read
# - awatch var       # read/write

# Software watchpoints (fallback)
# - Lentos: single-step emulation
# - Usados cuando hardware agotado

# LLDB:
# watchpoint set variable var
# watchpoint set expression -- *(int*)0x7fff...

# GDB:
# watch var           # write
# rwatch var          # read
# awatch var          # read/write
# watch -l var        # hardware (explicito)

# Condicionales en watchpoints (GDB 7+):
# watch var if x > 10
)" << "\n\n";
}

void demo_core_containers() {
    std::cout << "--- Core dumps en contenedores ---\n";
    std::cout << R"(
# Docker
docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined ...
# --cap-add=SYS_PTRACE: permite ptrace (gdb, strace)
# --security-opt seccomp=unconfined: desactiva seccomp

# Core pattern en container
# /proc/sys/kernel/core_pattern: core.%e.%p

# systemd-coredump en container
# coredumpctl list
# coredumpctl gdb <PID>

# kubectl debug
kubectl debug pod/mi-pod -it --image=ubuntu --target=app
kubectl debug node/mi-nodo -it --image=ubuntu -- bash
)" << "\n\n";
}

void demo_production() {
    std::cout << "--- Debugging en producción ---\n";
    std::cout << R"(
# Core dumps en prod
ulimit -c unlimited
# /etc/systemd/coredump.conf: ProcessSizeMax=2G

# Containers
docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined ...

# Sin gdb en prod: observabilidad
# - Logging estructurado (spdlog + fmt)
spdlog::info("user={} action={} correlation_id={}", user_id, action, corr_id);

# Métricas (Prometheus)
counter.increment("requests_total");
histogram.observe("request_duration_ms", duration);

# Distributed tracing (OpenTelemetry)
auto span = tracer->StartSpan("db.query");
span->SetAttribute("db.statement", query);
span->End();

# eBPF / bpftrace (kernel-level)
bpftrace -e 'tracepoint:syscalls:sys_enter_openat { printf("%s %s\n", comm, str(args->filename)); }'

# Sin core dump: Logging exhaustivo + correlation IDs
# Replay de requests (grabar request/response)
# Feature flags para activar debug logging on-demand
)" << "\n\n";
}

void demo_k8s_debug() {
    std::cout << "--- Debugging en Kubernetes ---\n";
    std::cout << R"(
# Ephemeral containers (K8s 1.23+)
kubectl debug pod/mi-pod -it --image=gdb --target=app
# Crea container efímero con gdb, attach a proceso 'app'

# Debug node
kubectl debug node/mi-nodo -it --image=ubuntu -- bash

# Copiar core dump
kubectl cp pod/mi-pod:/tmp/core /local/core

# Remote debugging
kubectl port-forward pod/mi-pod 1234:1234
# gdb -ex "target remote localhost:1234"

# Ephemeral container con gdb
kubectl debug pod/mi-pod -it --image=ubuntu --target=app -- bash -c "apt update && apt install -y gdb && gdb -p 1"
)" << "\n\n";
}

void demo_remote() {
    std::cout << "--- Remote debugging ---\n";
    std::cout << R"(
# gdbserver (en target)
gdbserver :1234 ./mi_programa arg1 arg2
# o attach a PID:
gdbserver :1234 --attach 1234

# gdb (en host)
gdb ./mi_programa
(gdb) target remote 192.168.1.100:1234

# VS Code launch.json (attach)
{
  "name": "Attach Remote",
  "type": "cppdbg",
  "request": "attach",
  "program": "${workspaceFolder}/build/mi_programa",
  "miDebuggerServerAddress": "192.168.1.100:1234",
  "MIMode": "gdb"
}

# SSH tunnel para gdbserver remoto
ssh -L 1234:localhost:1234 user@remote
# luego gdb target remote localhost:1234
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 08: Depuración avanzada (parte 2) ===\n\n";

    demo_rr_concept();
    demo_gdb_python();
    demo_pretty_printers();
    demo_conditional_bp();
    demo_watchpoints();
    demo_core_containers();
    demo_production();
    demo_k8s_debug();
    demo_remote();

    std::cout << "=== FIN LECCIÓN 08 (Depuración parte 2) ===\n";
    return 0;
}
