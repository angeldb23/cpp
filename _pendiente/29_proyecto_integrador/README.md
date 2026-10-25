# 🏰 Módulo 9: Proyecto Integrador Final - Mini Motor / Juego Modular

¡Felicitaciones por llegar al proyecto final!

Acá aplicamos **todo** lo aprendido a lo largo del curso:
- **Separación modular:** Archivos de cabecera (`.hpp`) y de implementación (`.cpp`).
- **Arquitectura de Bucle de Juego (Game Loop):** Procesar Input -> Actualizar Lógica -> Renderizar.
- **Herencia y Polimorfismo:** Clase base abstracta `Entidad` y clases derivadas (`Jugador`, `Monstruo`).
- **Gestión Moderna de Memoria:** Punteros inteligentes (`std::unique_ptr`) en un `std::vector`.
- **RAII y Encapsulamiento.**

---

## 📂 Archivos del Proyecto

- `Entidad.hpp`: Declaración de la jerarquía polimórfica de entidades.
- `Juego.hpp`: Declaración de la clase del motor y del bucle principal.
- `Juego.cpp`: Implementación de la lógica del juego.
- `main.cpp`: Punto de entrada que inicializa y corre el juego.
- `Makefile`: Script para compilar todo el proyecto de un solo comando.

---

## 🛠️ Cómo compilar y ejecutar

Podés compilarlo fácilmente desde la terminal:

```bash
# Opción 1: Con g++ directo
g++ -std=c++17 -Wall main.cpp Juego.cpp -o mini_juego
./mini_juego

# Opción 2: Con make
make
./mini_juego
```
