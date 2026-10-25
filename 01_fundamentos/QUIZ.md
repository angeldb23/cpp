# 🧠 Quiz de Repaso: Módulo 01 (Fundamentos y Control de Flujo)

Respondé estas preguntas conceptuales y de análisis de código para consolidar lo aprendido en el Módulo 01.

---

### Preguntas Conceptuales

1. ¿Por qué en C++ moderno se recomienda inicializar variables con llaves `{}` (uniform initialization) en lugar de `=`?
2. ¿Cuál es la diferencia técnica entre una variable `const` y una expresión `constexpr`?
3. ¿Qué ocurre exactamente si dividís `int a = 5; int b = 2; double c = a / b;` y cómo se soluciona de forma explícita?
4. ¿Qué ventaja ofrece compilar con `-fsanitize=address,undefined` durante el desarrollo?

---

### ¿Qué imprime este código?

5. Analizá el siguiente fragmento de código sin compilarlo:

```cpp
#include <iostream>

int main() {
    int x = 1;
    for (int i = 0; i < 3; ++i) {
        switch (x) {
            case 1:
                x += 2;
            case 2:
                x += 3;
                break;
            default:
                x += 1;
        }
    }
    std::cout << "x = " << x << std::endl;
    return 0;
}
```
¿Cuál es el valor final de `x` impreso por pantalla? Explicá por qué.

---

## 🔑 Respuestas y Explicaciones

1. **Uniform Initialization (`{}`):**  
   Previene conversiones estrechas silenciosas ("narrowing conversions"). Por ejemplo, `int x = 3.99;` en C++ clásico trunca el número a 3 sin avisar, mientras que `int x{3.99};` genera un error de compilación inmediato impidiendo la pérdida involuntaria de precisión.

2. **`const` vs `constexpr`:**  
   `const` es una promesa de inmutabilidad en tiempo de ejecución (la variable no puede cambiar después de ser inicializada, aunque su valor provenga de un cálculo dinámico o del teclado). `constexpr` es una garantía de que el valor PUEDE calcularse en tiempo de compilación por el compilador, incrustándose como un valor inmediato en el binario sin costo de CPU en ejecución.

3. **División entera:**  
   Como ambos operandos (`a` y `b`) son de tipo `int`, C++ realiza una división entera descartando el residuo (`5 / 2 = 2`). Luego convierte el `2` a `2.0` al asignarlo a `c`. Para obtener `2.5`, al menos uno de los operandos debe convertirse explícitamente a tipo flotante: `double c = static_cast<double>(a) / b;`.

4. **Address & Undefined Behavior Sanitizers:**  
   Instrumentan el binario con chequeos que detectan al instante accesos a memoria fuera de límites (buffer overflows), uso de variables no inicializadas, punteros colgantes (use-after-free) y desplazamientos de bits inválidos, deteniendo el programa con un reporte exacto del archivo y línea donde ocurrió la falla.

5. **Análisis del código con switch:**  
   - En la vuelta 0: `x = 1`. Coincide con `case 1:`. Se ejecuta `x += 2` (x pasa a valer 3). **¡Ojo! No hay `break` en `case 1`**, por lo que cae por cascada (*fallthrough*) al `case 2:`. Ejecuta `x += 3` (x pasa a valer 6) y encuentra el `break`.
   - En la vuelta 1: `x = 6`. No coincide ni con 1 ni con 2, va al `default:`. Ejecuta `x += 1` (x pasa a valer 7).
   - En la vuelta 2: `x = 7`. Va al `default:`. Ejecuta `x += 1` (x pasa a valer 8).
   - **Resultado final impreso:** `x = 8`.
