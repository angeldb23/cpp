#pragma once
/* =============================================================================
   MÓDULO 25 - Cabecera compartida del módulo (Audio y Empaquetado de Assets)
   =============================================================================
   Solo constantes y helpers NEUTROS que no resuelven ningún TODO del curso:
   las lecciones y ejercicios implementan la lógica de audio (mezcla, delay,
   freeverb, pack, CRC32) a mano, porque eso es lo que se estudia.

   Si tomás un valor constante desde acá, quedó verificado en la sesión
   de construcción (DECISIONES.md dice contra qué se verificó cada uno).

   miniaudio: un solo header C99, tag 0.11.25 (verificado con git ls-remote).
   ============================================================================= */

#include <cmath>
#include <cstdint>

namespace f25 {

/* El sample rate "estándar" profesional moderno. 44.1kHz sobrevive por el CD;
   48000 Hz es el default de miniaudio (y de casi todos los backends). */
inline constexpr std::uint32_t kSampleRate = 48000;

/* Dos canales es el caso de 99% del audio de un juego chico; el spatializer de
   miniaudio mezcla a la cantidad de canales del engine (2 si no se cambia). */
inline constexpr std::uint32_t kCanales = 2;

/* Pool máximo de voces simultáneas del "cierre" del módulo (eldueño elige
   agrandarlo o achicarlo según su presupuesto de tiempo de CPU). */
inline constexpr int kMaxVoces = 32;

/* Pasos del motor de audio en un bus del grafo (master/SFX/music/ambience). */
inline constexpr int kBusesSistema = 4;

inline constexpr double kPi = 3.14159265358979323846;

inline double fAbs(double v) {
    return v < 0.0 ? -v : v;
}

inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

/* Milisegundos -> muestras a un sample rate dado (usalo para tiempos de
   delay/attack/release configurables). 48.000 Hz: 1 ms son 48 muestras. */
inline double msAmuestras(double ms, std::uint32_t sampleRate) {
    return ms * (static_cast<double>(sampleRate) / 1000.0);
}

} // namespace f25
