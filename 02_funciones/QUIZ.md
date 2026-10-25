# 🧠 Quiz de Repaso: Módulo 02 - Funciones

## Preguntas Conceptuales
1. ¿Cuál es la diferencia entre pasar un argumento por valor y por referencia? ¿En qué casos es preferible usar referencia?
2. Explica qué son los *argumentos con valor por defecto* y cuáles son sus restricciones en la declaración de una función.
3. ¿Qué es una *referencia universal* y cómo se relaciona con el *perfect forwarding*?
4. Describe qué ocurre internamente cuando se llama a `std::move` sobre un objeto.
5. ¿Cómo afecta la regla de los *default arguments* a la ABI de una librería compartida?

## ¿Qué imprime este código?
```cpp
#include <iostream>
#include <utility>

void imprimir(int& x) { std::cout << "lvalue" << std::endl; }
void imprimir(int&& x) { std::cout << "rvalue" << std::endl; }

template <typename T>
void forwardPrint(T&& arg) {
    imprimir(std::forward<T>(arg));
}

int main() {
    int a = 10;
    forwardPrint(a);
    forwardPrint(20);
    forwardPrint(std::move(a));
}
```

## Respuestas y Explicaciones
*(No leas antes de intentar responder)*
1. ...
2. ...
3. ...
4. ...
5. ...
