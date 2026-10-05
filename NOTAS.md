# Notas de estudio

Explicaciones detalladas de cada módulo del proyecto. El código tiene comentarios
cortos; aquí está el "por qué" y el paso a paso de cada parte.

---

## Conceptos de C++ usados

- **`static` en una función:** la función solo existe dentro de ese archivo `.cpp`;
  ningún otro archivo la puede llamar. Se usa en `validateRotation` (Encryption.cpp).
- **`size_t`:** tipo de dato que solo admite números positivos. Es ideal para tamaños
  de objetos e índices (por ejemplo, lo que devuelve `string::size()`).
- **`static_cast<char>(x)`:** convierte un número (0 a 255) en un `char` para poder
  meterlo dentro de un `string`.
- **`static_cast<long>(x)`:** se usa para convertir `size()` (que devuelve `size_type`)
  a `long`, que es el tipo que usan nuestras funciones para longitudes.
- **`reinterpret_cast<const char*>(p)`:** ve el mismo bloque de memoria como otro tipo
  de puntero (por ejemplo, `unsigned char*` como `char*`) sin copiar nada.
- **`unsigned char`:** ocupa exactamente 1 byte (8 bits) y va de 0 a 255. Por eso los
  datos binarios se manejan como arreglos de `unsigned char`.
- **`0xFF` y `0xFFFF`:** `0xFF` = 255 = un byte lleno de unos. `0xFFFF` = 65 535 = el
  mayor número que cabe en 2 bytes.
- **Pasar punteros por referencia (`unsigned char*& data`):** permite que la función
  reserve memoria con `new[]` y el llamador reciba el puntero. Quien llama es el
  responsable de liberarlo con `delete[]`.
- **Excepciones propias:** cada módulo tiene su clase de excepción que hereda de
  `std::runtime_error` o `std::invalid_argument`. Así se pueden capturar con un
  `try-catch` normal o con `catch (const std::exception& e)` en `main`.

## Librerías incluidas en main.cpp

- `<iostream>`: `cout` y `cin` (consola).
- `<fstream>`: `ifstream` y `ofstream` (leer y escribir archivos).
- `<stdexcept>`: `runtime_error`, `invalid_argument`, las excepciones estándar.
- `<string>`: `string` y `to_string` (RLE y mensajes de error).

---

## Flujo general (main.cpp)

**Opción 1: comprimir y encriptar**
1. Leer el archivo de texto original (en modo binario, para no alterar ningún byte).
2. Comprimir con el método elegido (RLE o LZ78).
3. Serializar: convertir la salida del compresor en un arreglo plano de bytes.
4. Encriptar ese arreglo (rotación + XOR) y guardarlo en `encrypted.txt`.

**Opción 2: desencriptar y descomprimir**
1. Leer el archivo encriptado.
2. Desencriptar (XOR + rotación a la derecha), con la misma `n` y `K`.
3. Deserializar y descomprimir con el mismo método.
4. Guardar el resultado en `decrypted.txt`.

### Detalles de main.cpp
- `cin.getline(inputPath, PATH_SIZE)`: lee una línea y la guarda en `inputPath`,
  sin pasarse de `PATH_SIZE` caracteres. Así la ruta puede tener espacios.
- Después de `cin >> numero` queda un Enter en el buffer. `cin.ignore(1000, '\n')`
  lo descarta para que el siguiente `getline` no lea una línea vacía.
- Si el usuario escribe letras donde va un número, `cin.fail()` es verdadero:
  se hace `cin.clear()` para limpiar el error y se descarta la línea.
- `n` y `K` se validan **antes** de reservar memoria.
- **Manejo de memoria con excepciones:** todos los arreglos dinámicos empiezan en
  `nullptr`. Si algo falla a mitad del proceso, el `catch (...)` los libera
  (`delete[]` sobre `nullptr` no hace nada) y relanza la excepción con `throw;`
  para que `main` muestre el mensaje de error.

---

## RLE (Run-Length Encoding)

Cada secuencia de caracteres consecutivos iguales (una "racha") se guarda como un
registro binario de **3 bytes**:

```
[byte alto][byte bajo][símbolo]
```

- **Byte alto:** primera parte del número de repeticiones.
- **Byte bajo:** segunda parte del número de repeticiones.
- **Tercer byte:** la letra (el símbolo).

Ejemplo: `AAAA` se guarda como `[0][4][A]`: el alto y el bajo juntos forman el
número 4, y luego va la A.

`"AAAABBBCC"` → `{0,4,'A'} {0,3,'B'} {0,2,'C'}`

### ¿Por qué 2 bytes para el conteo?
Un solo byte llega hasta 255. Con dos se pueden contar rachas de hasta 65 535
(`RLE_MAX_RUN_LENGTH = 0xFFFF`). Una racha más larga se parte en varios registros.

### ¿Por qué un conteo binario y no texto como "4A3B2C"?
Para que el texto original pueda contener dígitos sin volver ambigua la
descompresión. Si el texto fuera "4A3B", no se sabría qué es conteo y qué es
contenido. El `string` resultante contiene bytes arbitrarios, no texto legible.

### Compresión, paso a paso
- Se toma un carácter (`symbol = text[i]`) y se empieza a contar desde 1.
- El ciclo interno dice: *"mientras todavía no me haya salido del texto, y el
  siguiente carácter sea igual al símbolo que estoy contando, y no haya llegado al
  máximo permitido, sigue sumando uno al conteo"*.
- `(count >> 8) & 0xFF`: mueve los bits 8 posiciones a la derecha para obtener el
  byte alto; `& 0xFF` se queda solo con los últimos 8 bits por seguridad.
- `count & 0xFF`: toma el byte bajo (los 8 bits de la derecha).
- Se agrega el símbolo y se avanza `i += count`.

### Descompresión, paso a paso
- Cada racha ocupa 3 bytes, así que el texto comprimido tiene que medir un múltiplo
  de 3 (3, 6, 9...). Por eso se revisa `size % 3`; si no da 0, los datos están
  corruptos.
- Se leen el byte alto y el bajo, y `(alto << 8) | bajo` reconstruye el conteo
  original (al revés de como se partió).
- Un conteo de 0 es imposible en datos válidos, así que también lanza excepción.
- `result.append(runLength, simbolo)` agrega el símbolo repetido.

### ¿Por qué RLE a veces hace el archivo MÁS grande?
La encriptación **no** cambia el tamaño (1 byte entra, 1 byte sale). Lo que crece
es la compresión RLE:

| Texto        | Original | RLE                    | Resultado      |
|--------------|----------|------------------------|----------------|
| `AAAAAAAAAA` | 10 bytes | 1 racha × 3 = 3 bytes  | comprime       |
| `AAA`        | 3 bytes  | 3 bytes                | queda igual    |
| `hola`       | 4 bytes  | 4 rachas × 3 = 12 bytes| crece 3 veces  |

En un texto normal casi no hay letras iguales seguidas, así que cada letra se
vuelve un registro de 3 bytes. RLE solo es eficiente cuando hay rachas de **más
de 3** caracteres iguales seguidos.

---

## LZ78

### Ideas básicas
- La salida es una lista de pares `(índice, símbolo)`:
  - `índice`: posición de una frase ya vista en el diccionario.
  - `símbolo`: el carácter nuevo que sigue a esa frase.
- `index == 0` significa "no hay coincidencia previa" (frase nueva de 1 carácter).
- Los índices del diccionario empiezan en **1** (1-based), porque el 0 está reservado.

### El diccionario (sin STL)
- Es un arreglo dinámico de punteros a `DictionaryEntry`, hecho a mano con
  `new`/`delete`, sin `std::vector` ni ningún contenedor STL.
- Cada `DictionaryEntry` guarda una frase como arreglo de `char` reservado con
  `new[]` (no necesariamente termina en `'\0'`) y su longitud.
- Empieza con capacidad 8. Cuando se llena, `grow()` **duplica** la capacidad:
  reserva un arreglo nuevo, copia los punteros y borra solo el arreglo viejo de
  punteros (no las entradas, que siguen en uso).
- El destructor libera cada entrada, su `data` y el arreglo de punteros.
- `findMatch(prefix, length)` busca una entrada cuyo contenido coincida
  **exactamente** con esos caracteres; devuelve su índice o 0.

### Compresión, paso a paso
1. Desde la posición `i`, se busca la coincidencia más larga en el diccionario:
   se va alargando el prefijo de a un carácter mientras siga existiendo.
2. El carácter que rompió la coincidencia es el `símbolo` del par.
3. Se emite el par `(índice de la coincidencia, símbolo)`.
4. Se agrega al diccionario la frase nueva (coincidencia + símbolo).
5. Se avanza `i` la longitud de esa frase.

El arreglo de salida de pares también crece duplicando su capacidad.

### Caso borde: fin del texto
Si la coincidencia llega justo hasta el final del texto, no queda carácter
siguiente que emitir. Se usa `symbol == '\0'` como marca de "esta frase del
diccionario, tal cual, es todo lo que hay que copiar aquí". Se asume que el texto
de entrada no contiene bytes `'\0'` reales, lo cual es válido para archivos de
texto plano (no para imágenes o PDF).

### Descompresión
Se reconstruye el diccionario **exactamente igual** que en la compresión,
agregando la misma frase en cada paso, para que los índices coincidan. Si un par
apunta a un índice que todavía no existe, los datos están corruptos (por ejemplo,
se usó una clave o un método equivocado al desencriptar) y se lanza
`LZ78InvalidFormatException`.

### ¿Por qué LZ78 comprime mejor que RLE en texto normal?
También usa 3 bytes por par, pero cada par puede representar una frase completa
ya vista, no solo una letra. Cuanto más se repiten palabras, más ahorra.

---

## Serialización

Es el puente entre la compresión y la encriptación. Cada algoritmo entrega el
resultado en un formato distinto (RLE en `string`, LZ78 como arreglo de `Pair`),
pero la encriptación solo recibe un arreglo plano de bytes. Este módulo empaca la
salida de cada compresor en una fila de bytes para encriptarla y guardarla, y la
desempaca después en el formato que espera cada descompresor.

Formato (ambos de 3 bytes por registro):
```
RLE:  [conteo alto][conteo bajo][carácter]
LZ78: [índice alto][índice bajo][carácter]
```

- Un índice de LZ78 debe caber en 2 bytes (máximo `0xFFFF` = 65 535). Si un texto
  es tan grande que el diccionario supera eso, se lanza `SerializationException`.
- Al leer, la longitud debe ser múltiplo de 3.

---

## Encriptación (rotación + XOR)

`data` es un arreglo de `unsigned char`, y cada uno ocupa exactamente 1 byte.
Entonces `data[i]` es un carácter entero (o un byte del texto comprimido), y en
cada vuelta del ciclo:
- Se toman sus 8 bits.
- Se rotan todos juntos.
- Se les hace XOR con la clave, también los 8 juntos.
- El resultado (otro byte de 8 bits) se guarda en la misma posición `data[i]`,
  reemplazando el original.

### Rotar bits
Se corre todo `n` posiciones y los bits que se salen por un extremo entran por el
otro.

Ejemplo: `rotateLeft('A', 2)` con `'A' = 01000001`:
- `byte << 2` corre todo 2 posiciones a la izquierda: `00000100`. Los 2 bits de la
  izquierda (`01`) se "caen".
- `byte >> 6` (es decir, `8 - n`) rescata justo esos bits que se cayeron y los deja
  a la derecha: `00000001`.
- `|` junta ambas partes: `00000101`.

`rotateRight` hace lo mismo hacia el otro lado.

### Encriptar y desencriptar
- **Encriptar:** primero rotar a la izquierda `n` bits, luego XOR con la clave `K`.
- **Desencriptar:** el proceso inverso en orden inverso: primero XOR con la misma
  clave (XOR aplicado dos veces se cancela), luego rotar a la derecha `n` bits.

### Validación
`validateRotation` revisa que `0 < n < 8`. Rotar 0 u 8 bits deja el byte igual
(no encripta nada), y con valores mayores los desplazamientos no tienen sentido
para un byte.
