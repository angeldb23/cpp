/* =============================================================================
   MÓDULO 15 - LECCIÓN 07: Depuración — gdb/lldb, breakpoints, watchpoints,
   core dumps
   =============================================================================
   TEMA 7 de TEMAS.md (2 lecciones): Herramientas de depuración.

   GDB (GNU Debugger) — Estándar en Linux:
   - gdb ./mi_programa
   - run [args]              # Iniciar
   - break file.cpp:123      # Breakpoint en línea
   - break func_name         # Breakpoint en función
   - break *0x401000         # Breakpoint en dirección
   - watch expr              # Watchpoint (data breakpoint)
   - rwatch / awatch         # Read / access watchpoint
   - continue / c            # Continuar
   - next / n                # Paso siguiente (no entra funciones)
   - step / s                # Paso siguiente (entra funciones)
   - finish                  # Terminar función actual
   - backtrace / bt          # Stack trace
   - frame N                 # Cambiar frame
   - print var / p var       # Imprimir variable
   - p/x var                 # Hex
   - display var             # Auto-print cada parada
   - info locals / info args # Variables locales / args
   - info breakpoints        # Listar breakpoints
   - delete N                # Borrar breakpoint N
   - condition N expr        # Breakpoint condicional
   - commands N              # Comandos al parar en breakpoint N

   LLDB (LLVM Debugger) — macOS / Clang:
   - Sintaxis similar a GDB pero comandos diferentes:
   - breakpoint set --name func
   - watchpoint set variable var
   - thread backtrace / bt
   - frame variable var
   - expression expr / p expr
   - process launch --stop-at-entry

   BREAKPOINTS CONDICIONALES:
   - gdb: break file.cpp:123 if x > 10
   - lldb: breakpoint set -f file.cpp -l 123 -c "x > 10"

   WATCHPOINTS (data breakpoints):
   - gdb: watch var == 42
   - lldb: watchpoint set variable var
   - Hardware watchpoints: limitados (4 en x86), usan debug registers.
   - Software watchpoints: lentos (single-step).

   CORE DUMPS:
   - ulimit -c unlimited     # Habilitar core dumps
   - /proc/sys/kernel/core_pattern: |/usr/lib/systemd/systemd-coredump %P %u %g %s %t %c %e
   - gdb ./exe core          # Analizar core dump
   - bt                      # Stack trace del crash
   - info registers          # Registros en momento del crash

   POST-MORTEM DEBUGGING:
   - coredumpctl list        # Listar cores (systemd)
   - coredumpctl gdb PID     # Abrir en GDB
   - gdb -ex "set pagination off" -ex "bt" -ex "quit" ./exe core  # Batch mode

   PYTHON SCRIPTING EN GDB:
   - Python API para pretty-printers, comandos custom.
   - ~/.gdbinit: define hooks, pretty printers STL.
   - gdb -ex "source script.py"

   RR (Record and Replay) — Time-travel debugging:
   - rr record ./exe          # Grabar ejecución
   - rr replay                # Replay determinístico
   - Reverse continue (rc), reverse next (rn)
   - Permite ir HACIA ATRÁS en el tiempo.

   VS CODE / CLION / QT CREATOR:
   - GUI sobre GDB/LLDB
   - Debug adapters: cppdbg (GDB), lldb-vscode
   - Launch.json: configurar args, env, cwd, miDebuggerPath

   CORE DUMPS EN CONTAINERS:
   - docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined
   - /proc/sys/kernel/core_pattern: core.%e.%p
   - docker commit para preservar estado.

   POST-MORTEM EN PRODUCCIÓN:
   - Habilitar core dumps en prod (ulimit -c unlimited)
   - Recoger core + binario + debuginfo
   - gdb -ex "set sysroot /path/to/sysroot" -ex "bt" ./exe core

   DEBUGGING EN PRODUCCIÓN (sin gdb):
   - Logging estructurado (spdlog, fmt)
   - Métricas (Prometheus, statsd)
   - Distributed tracing (OpenTelemetry, Jaeger)
   - eBPF (bpftrace) para observabilidad kernel-level. */

#include <iostream>
#include <string>
#include <vector>

void demo_gdb_basics() {
    std::cout << "--- GDB básicos ---\n";
    std::cout << R"(
# Iniciar
gdb ./mi_programa
gdb --args ./mi_programa arg1 arg2

# Ejecución
run                    # Iniciar
run arg1 arg2          # Con argumentos
continue / c           # Continuar tras breakpoint
next / n               # Paso siguiente (no entra funciones)
step / s               # Paso siguiente (entra funciones)
finish                 # Terminar función actual
until 123              # Correr hasta línea 123

# Breakpoints
break file.cpp:123           # Línea
break mi_func                # Función
break *0x401000              # Dirección
break file.cpp:123 if x > 10 # Condicional

# Watchpoints (data breakpoints)
watch var                  # Write watchpoint
rwatch var                 # Read watchpoint
awatch var                 # Read/Write watchpoint

# Inspección
print var / p var          # Imprimir
p/x var                    # Hex
p/t var                    # Binario
p var@5                    # Array de 5 elementos
display var                # Auto-print cada parada
info locals                # Variables locales
info args                  # Argumentos
info breakpoints           # Listar breakpoints
info registers             # Registros

# Stack
backtrace / bt             # Stack trace
bt full                    # Con variables locales
frame N                    # Cambiar a frame N
up / down                  # Subir/bajar frames

# Control
delete N                   # Borrar breakpoint N
disable N                  # Deshabilitar
enable N                   # Habilitar
condition N expr           # Breakpoint condicional
commands N                 # Comandos al parar
  > print x
  > continue
  > end

# Ayuda
help breakpoints
help running
)" << "\n\n";
}

void demo_lldb() {
    std::cout << "--- LLDB (macOS / Clang) ---\n";
    std::cout << R"(
# Iniciar
lldb ./mi_programa
lldb -- ./mi_programa arg1 arg2

# Breakpoints
breakpoint set --name func
breakpoint set --file file.cpp --line 123
breakpoint set --file file.cpp --line 123 --condition "x > 10"

# Watchpoints
watchpoint set variable var
watchpoint set expression -- *(int*)0x7fff...

# Inspección
frame variable var
expression var / p var
frame variable -x var  # Hex

# Stack
thread backtrace / bt
thread backtrace all   # Todos los threads
frame select N

# Ejecución
process launch
process launch --stop-at-entry
thread continue / c
thread step-over / n
thread step-in / s
thread step-out / f

# Ayuda
help breakpoint
help watchpoint
)" << "\n\n";
}

void demo_core_dumps() {
    std::cout << "--- Core dumps ---\n";
    std::cout << R"(
# Habilitar core dumps
ulimit -c unlimited                    # Bash
limit coredumpsize unlimited           # zsh/fish
# Persistente: /etc/security/limits.conf
# * soft core unlimited
# * hard core unlimited

# Core pattern (systemd)
# /proc/sys/kernel/core_pattern:
# |/usr/lib/systemd/systemd-coredump %P %u %g %s %t %c %e

# Analizar core dump
gdb ./mi_programa core
gdb ./mi_programa core.12345

# En GDB:
bt                    # Stack trace
bt full               # Con locales
info registers        # Registros
info frame            # Frame actual

# systemd-coredump
coredumpctl list
coredumpctl info 1234
coredumpctl gdb 1234
coredumpctl debug 1234  # Abre en gdb

# Batch mode (CI/automatizado)
gdb -ex "set pagination off" -ex "bt" -ex "quit" ./exe core
)" << "\n\n";
}

void demo_rr() {
    std::cout << "--- rr (Record and Replay) ---\n";
    std::cout << R"(
# Time-travel debugging
# Instalación: apt install rr / brew install rr

# Grabar ejecución
rr record ./mi_programa arg1 arg2

# Replay (determinístico, siempre igual)
rr replay

# En rr replay (GDB interface):
reverse-continue / rc    # Continuar hacia ATRÁS
reverse-next / rn        # Paso hacia atrás
reverse-step / rs
reverse-finish

# Breakpoints normales funcionan en replay
# Watchpoints funcionan en replay

# Usos:
# - Bugs intermitentes (heisenbugs)
# - Race conditions (ir hacia atrás al race)
# - Memory corruption (ver quién escribió)
# - Flaky tests
)" << "\n\n";
}

void demo_vscode() {
    std::cout << "--- VS Code / CLion / Qt Creator ---\n";
    std::cout << R"(
# VS Code: .vscode/launch.json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/build/debug/mi_programa",
      "args": ["arg1", "arg2"],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb",
      "miDebuggerPath": "/usr/bin/gdb",
      "setupCommands": [
        { "description": "Enable pretty-printing",
          "text": "-enable-pretty-printing",
          "ignoreFailed": true }
      ]
    }
  ]
}

# CLion: Run > Edit Configurations > CMake Application
# Qt Creator: Debug > Start Debugging > Start Debugging External Application

# Extensiones útiles VS Code:
# - C/C++ (Microsoft)
# - C/C++ Advanced (IntelliSense)
# - DAP extensions para otros debuggers
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
#   spdlog::info("user={} action={}", user_id, action);
# - Métricas (Prometheus)
#   counter.increment("requests_total");
# - Distributed tracing (OpenTelemetry)
#   trace.span("db.query", "SELECT * FROM users");
# - eBPF (bpftrace) para kernel-level
#   bpftrace -e 'tracepoint:syscalls:sys_enter_openat { printf("%s %s\n", comm, str(args->filename)); }'

# Post-mortem sin core dump:
# - Logging exhaustivo + correlation IDs
# - Replay de requests (grabar request/response)
# - Feature flags para activar debug logging on-demand
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 07: Depuración — gdb/lldb, breakpoints, watchpoints, core dumps ===\n\n";

    demo_gdb_basics();
    demo_lldb();
    demo_core_dumps();
    demo_rr();
    demo_vscode();
    demo_production();

    std::cout << "=== FIN LECCIÓN 07 ===\n";
    return 0;
}