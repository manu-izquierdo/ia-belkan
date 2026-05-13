# Práctica 2 — Operación Contención Belkanita
### Inteligencia Artificial · Universidad de Granada · ETSIIT · Curso 2025-2026

---

## Autoría y propiedad del código

Este repositorio es una copia privada del repositorio oficial de la asignatura **Inteligencia Artificial** del Departamento de Ciencias de la Computación e Inteligencia Artificial (DECSAI) de la Universidad de Granada.

**El código del simulador, el motor del juego y toda la infraestructura del proyecto pertenecen a sus autores originales** del DECSAI/UGR y están disponibles en el repositorio oficial de la asignatura: [`ugr-ccia-IA/practica2`](https://github.com/ugr-ccia-IA/practica2).

La única parte de este repositorio cuya autoría me corresponde es el contenido de la carpeta:

```
Comportamientos_Agentes/
├── ingeniero.hpp
├── ingeniero.cpp
├── tecnico.hpp
└── tecnico.cpp
```

Estos cuatro ficheros contienen la implementación de los comportamientos reactivos y deliberativos de los agentes **Ingeniero** y **Técnico** para los Niveles 0 al 6 de la práctica.

**Estudiante:** Manuel Jesús Izquierdo Izquierdo  
**Grupo teoría:** 2ºC

---

## Descripción de la práctica

El objetivo es dotar de inteligencia a dos agentes autónomos que deben construir una red de tuberías para transportar la sustancia tóxica **Belkanita** desde su punto de filtración hasta una planta de tratamiento de residuos (`'U'`), navegando por un mapa con terrenos, alturas y restricciones de energía e impacto ecológico.

Los niveles implementados son:

| Nivel | Nombre | Tipo | Agente principal |
|-------|--------|------|-----------------|
| 0 | Rastreadores de Alcantarillas | Reactivo | Ingeniero + Técnico |
| 1 | De Reconocimiento por la Naturaleza | Reactivo | Ingeniero + Técnico |
| 2 | Corre, Ingeniero, Corre! | Deliberativo (BFS) | Ingeniero |
| 3 | El Técnico mide sus esfuerzos | Deliberativo (A*) | Técnico |
| 4 | Planifica, Ingeniero, Planifica | Deliberativo (Priority Queue) | Ingeniero |
| 5 | A Poner Tuberías | Multiagente coordinado | Ingeniero + Técnico |
| 6 | ¿Qué alguien ponga luz? | Multiagente + mapa desconocido | Ingeniero + Técnico |

---

## Prerrequisitos y configuración del repositorio

### 1. Añadir tu clave SSH a GitHub
Lo más cómodo es utilizar una clave SSH para conectarte desde el terminal. Consulta la guía oficial:
[Conectar a GitHub con SSH](https://docs.github.com/es/authentication/connecting-to-github-with-ssh)

### 2. Crear tu copia personal del repositorio
Para que tu copia sea privada, en lugar de un fork usa la opción de importación:
1. Ve a <https://github.com/new/import>
2. Introduce como URL de origen: `https://github.com/ugr-ccia-IA/practica2`
3. Marca el repositorio como **privado**

### 3. Clonar tu repositorio en local
```bash
git clone git@github.com:TU_USUARIO_GITHUB/practica2.git
```

### 4. Enlazar con el repositorio oficial de la asignatura
```bash
git remote add upstream git@github.com:ugr-ccia-IA/practica2.git
```

Para incorporar actualizaciones del repositorio oficial a tu copia:
```bash
git pull upstream main
git push origin main
```

---

## Instalación y compilación (Linux)

```bash
# Primera vez: instala dependencias y compila
./install.sh

# Recompilar tras cambios en el código
make clean && make -j$(nproc)
```

---

## Ejecución

**Con interfaz gráfica:**
```bash
./practica2
```

**Sin interfaz (modo batch), útil para depuración:**
```bash
./practica2SG -m ./mapas/MAPA.map -n NIVEL -i F_ING C_ING O_ING -t F_TEC C_TEC O_TEC -seed 0 -Tiempo 3000 -Ambiental 1000 -Energia 3000 -O F_BELK C_BELK
```

Ejemplo completo:
```bash
./practica2SG -m ./mapas/mapa30.map -n 5 -i 15 17 2 -t 17 20 6 -seed 0 -Tiempo 3000 -Ambiental 1000 -Energia 3000 -O 19 15
```
---

## Flujo de trabajo con Git

```bash
git add .
git commit -m "Descripción del cambio"
git push origin main
```

---

## Más información

- [Repositorio oficial de la asignatura](https://github.com/ugr-ccia-IA/practica2)
- Guión completo disponible en [PRADO](https://pradogrado2526.ugr.es/)
