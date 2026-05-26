#include "tecnico.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>

using namespace std;

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoTecnico::think(Sensores sensores) {
  Action accion = IDLE;

  // Decisión del agente según el nivel
  switch (sensores.nivel) {
  case 0:
    accion = ComportamientoTecnicoNivel_0(sensores);
    break;
  case 1:
    accion = ComportamientoTecnicoNivel_1(sensores);
    break;
  case 2:
    accion = ComportamientoTecnicoNivel_2(sensores);
    break;
  case 3:
    accion = ComportamientoTecnicoNivel_3(sensores);
    break;
  case 4:
    accion = ComportamientoTecnicoNivel_4(sensores);
    break;
  case 5:
    accion = ComportamientoTecnicoNivel_5(sensores);
    break;
  case 6:
    accion = ComportamientoTecnicoNivel_6(sensores);
    break;
  }

  return accion;
}


// ─────────────────────────────────────────────────────────────────────────────
// NIVEL 0
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Comprueba si una celda es de tipo camino transitable.
 * @param c Carácter que representa el tipo de superficie.
 * @return true si es camino ('C'), zapatillas ('D') o meta ('U').
 */
bool ComportamientoTecnico::es_camino(unsigned char c) const {
  return (c == 'C' || c == 'D' || c == 'U');
}

/**
 * @brief Determina si casilla viable por altura.
 * @param casilla tipo de terreno
 * @param dif diferencia de altura entre casillas
 * @param zap No se pone puesto que el técnico no es capaz de subir 2 alturas con zapatillas y por tanto no es relevante
 * @return 'P' si no es accesible por altura y casilla en otro caso
 * @note Recibe el tipo de terreno, la dif entre casillas si tiene las
 * zapatillas y comprueba si es viable por altura.
 * DIFERENCIA: He usado esta función en vez de EsAccesiblePorAltura puesto que de esta manera,
 * en la función VeoCasillaPulgarcitoNivel0I puedo establecer prioridad dependiendo del tipo de casilla que devuelve.
 */
char ComportamientoTecnico::ViablePorAltura(char casilla, int dif) const {
  if (abs(dif) <= 1) {
    return casilla;
  } else {
    return 'P';
  }
}

/**
 * @brief Decide la mejor opción para el Nivel 0 usando el mapa de pulgarcito.
 * @param i terreno que hay en la posición 1 de superficie (45 izq)
 * @param c terreno que hay en la posición 2 de supeficie (justo delante)
 * @param d terreno que hay en la posición 3 de supeficie (45 dch)
 * @param t_i tiempo que hay en la posición 1
 * @param t_c tiempo que hay en la posición 2
 * @param t_d tiempo que hay en la posición 3
 * @return 2 si es mejor WALK, 1 para TURN_SL y 3 para TURN_SR. O no hay nada interesante.
 */
int ComportamientoTecnico::SeleccionaCasillaPulgarcito0(char i, char c, char d, int t_i, int t_c, int t_d) const {
  // Prioridad 1: Meta
  if (c == 'U') return 2; // WALK
  else if (i == 'U') return 1; // TURN_SL
  else if (d == 'U') return 3; // TURN_SR

  // Prioridad 3: Camino con menor tiempo de visita
  // Descarto todo lo que no es camino poniendo un tiempo muy alto
  if (i != 'C') t_i = INT_MAX;
  if (c != 'C') t_c = INT_MAX;
  if (d != 'C') t_d = INT_MAX;

  // Si no hay ningún camino viable delante, devolvemos 0 para que gire a buscar uno usando el default del switch
  if (t_i == INT_MAX && t_c == INT_MAX && t_d == INT_MAX)
    return 0;

  // Compruebo cual es el menor de los 3 tiempos y el que lo sea decidirá cual acción se deberá hacer
  int min_tiempo = min(t_i, min(t_c, t_d));

  if (min_tiempo == t_c) return 2;
  if (min_tiempo == t_i) return 1;
  if (min_tiempo == t_d) return 3;
  return 0;
}

// Niveles iniciales (Comportamientos reactivos simples)
Action ComportamientoTecnico::ComportamientoTecnicoNivel_0(Sensores sensores) {
  Action accion = IDLE;

  // Poner el valor de los sensores de visión sobre los mapas
  ActualizarMapa(sensores);

  // Primero comprobamos si la casilla en la que nos situamos es la de las Zapatillas ('D') actualizando su variable de estadoT
  if (sensores.superficie[0] == 'D')
    tiene_zapatillas = true;

  // Comprobamos si la casilla en la que nos situamos es la Planta de Residuos ('U'), en cuyo caso habríamos terminado
  if (sensores.superficie[0] == 'U')
    return IDLE;

  // Extraemos el valor de las casillas centro, 45º izquierda, 45º derecha
  // El valor del char será la superficie de la casilla si se puede acceder a ella y Precipicio'P' en caso contrario
  char i = ViablePorAltura(sensores.superficie[1], sensores.cota[1] - sensores.cota[0]);
  char c = ViablePorAltura(sensores.superficie[2], sensores.cota[2] - sensores.cota[0]);
  char d = ViablePorAltura(sensores.superficie[3], sensores.cota[3] - sensores.cota[0]);

  // Descartamos aquellos movimientos que hagan colisión con el técnico comprobando en la matriz de agentes si las casillas están desocupadas o no
  if (sensores.agentes[1] != '_')
    i = 'P';
  if (sensores.agentes[2] != '_')
    c = 'P';
  if (sensores.agentes[3] != '_')
    d = 'P';

  // Sacamos las coordenadas y rumbo de la posición actual
  ubicacion posActual = {sensores.posF, sensores.posC, sensores.rumbo};

  // Las coordenadas y rumbo de la posición de delante usando el método Delante
  ubicacion posC = Delante(posActual);

  // Las coordenadas y rumbo de la posición izquierda las saco modificando el rumbo hacia la izquierda incrementando el "enum" Orientacion, aplicando el operador % y dando un paso Delante
  ubicacion posI = posActual;
  posI.brujula = (Orientacion)((posI.brujula + 7) % 8);
  posI = Delante(posI);

  // Para la derecha hago lo mismo que hice en la izquierda
  ubicacion posD = posActual;
  posD.brujula = (Orientacion)((posD.brujula + 1) % 8);
  posD = Delante(posD);

  // Ya teniendo los 3 objetos ubicacion(posI,posC,posD), guardo en una variable el tiempo que hay almacenado en cada posición
  int t_i = (posI.f >= 0 && posI.f < mtiempo.size() && posI.c >= 0 && posI.c < mtiempo[0].size()) ? mtiempo[posI.f][posI.c] : INT_MAX;
  int t_c = (posC.f >= 0 && posC.f < mtiempo.size() && posC.c >= 0 && posC.c < mtiempo[0].size()) ? mtiempo[posC.f][posC.c] : INT_MAX;
  int t_d = (posD.f >= 0 && posD.f < mtiempo.size() && posD.c >= 0 && posD.c < mtiempo[0].size()) ? mtiempo[posD.f][posD.c] : INT_MAX;

  // Tras obtener las superficies i,c,d los tiempos almacenados en esas casillas t_i,t_c,t_d y comprobar que no ha habido choques, planteamos que hacer en la función VeoCasillaPulgarcitoNivel0I
  int pos = SeleccionaCasillaPulgarcito0(i, c, d, t_i, t_c, t_d);

  switch (pos) {
    case 2: accion = WALK; break;
    case 1: accion = TURN_SL; break;
    case 3: accion = TURN_SR; break;
    default: accion = TURN_SL; break;
  }

  // Incrementamos la variable instante que será la que vaya marcando el recorrido del personaje y sobreescribimos en la matriz de tiempos el nuevo tiempo de dicha posición
  instante++;
  mtiempo[sensores.posF][sensores.posC] = instante;

  // Devolvemos la acción decidida
  return accion;
}


// ─────────────────────────────────────────────────────────────────────────────
// NIVEL 1
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Determina si una casilla es transitable en Nivel 1.
 *
 * Añade respecto al Nivel 0 el bloqueo explícito de Muro ('M'),
 * Agua ('A') y Precipicio ('P'), independientemente
 * de la altura. Si el terreno supera ese filtro, comprueba si tiene zapatillas
 * para lo cual podría ir por la casilla Bosque ('B') y si no es así,  delega en
 * ViablePorAltura para comprobar el desnivel.
 * DIFERENCIA con Ingeniero: el Técnico nunca bloquea 'B' en el filtro base, sino que lo permite con zapatillas.
 * @param terreno   Tipo de superficie de la casilla destino.
 * @param dif_cota  Diferencia de altura entre casilla actual y destino.
 * @param zap       true si el agente posee las zapatillas.
 * @return El carácter de terreno si es transitable, 'P' si no lo es.
 */
char ComportamientoTecnico::TransitableNivel1(char terreno, int dif_cota, bool zap) const {
  // En el nivel 1, Muro y Agua son precipicios
  if (terreno == 'M' || terreno == 'A' || terreno == 'P')
    return 'P';

  if (terreno == 'B' && !zap)
    return 'P';

  return ViablePorAltura(terreno, dif_cota);
}

/**
 * @brief Decide la mejor opción para el Nivel 1 usando el mapa de pulgarcito.
 * @param i terreno que hay en la posición 1 de superficie (45 izq)
 * @param c terreno que hay en la posición 2 de supeficie (justo delante)
 * @param d terreno que hay en la posición 3 de supeficie (45 dch)
 * @param t_i tiempo que hay en la posición 1
 * @param t_c tiempo que hay en la posición 2
 * @param t_d tiempo que hay en la posición 3
 * @param zap indica si estoy en posesión de las zapatillas
 * @return 2 si es mejor WALK, 1 para TURN_SL y 3 para TURN_SR. O no hay nada interesante.
 */
int ComportamientoTecnico::SeleccionaCasillaPulgarcito1(char i, char c, char d, int t_i, int t_c, int t_d, bool zap) const {
  // Si lo que ve es un Precipicio lo descartamos y le damos un tiempo infinito
  if (c == 'P')
    t_c = INT_MAX;
  if (i == 'P')
    t_i = INT_MAX;
  if (d == 'P')
    t_d = INT_MAX;

  if (t_i == INT_MAX && t_c == INT_MAX && t_d == INT_MAX)
    return 0;

  // Vemos el camino con menor tiempo que será el más antiguo o nunca visitado
  int min_tiempo = min(t_i, min(t_c, t_d));

  // Prioridad a las zapatillas
  if (!zap) {
    if (c == 'D')
      return 2;
    if (i == 'D')
      return 1;
    if (d == 'D')
      return 3;
  }

  // Prioridad a Caminos o Senderos ('C' o 'S') que NO hayamos visitado
  if (min_tiempo == 0) {
    if ((c == 'C' || c == 'S') && t_c == 0)
      return 2;
    if ((i == 'C' || i == 'S') && t_i == 0)
      return 1;
    if ((d == 'C' || d == 'S') && t_d == 0)
      return 3;
  }

  if (min_tiempo == t_c)
    return 2; // WALK
  if (min_tiempo == t_i)
    return 1; // TURN_SL
  if (min_tiempo == t_d)
    return 3; // TURN_SR

  return 0;
}

/**
 * @brief Comportamiento reactivo del tecnico para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_1(Sensores sensores) // He aplicado la misma lógica que había en el nivel 0, por lo que he eliminado comentarios
{
  Action accion = IDLE;

  ActualizarMapa(sensores);

  if (sensores.superficie[0] == 'D')
    tiene_zapatillas = true;

  char i = TransitableNivel1(sensores.superficie[1], sensores.cota[1] - sensores.cota[0], tiene_zapatillas); // Si la altura es <= 1 o tiene zapatillas y es un bosque con altura <= 1 es transitable
  char c = TransitableNivel1(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tiene_zapatillas);
  char d = TransitableNivel1(sensores.superficie[3], sensores.cota[3] - sensores.cota[0], tiene_zapatillas);

  if (sensores.agentes[1] != '_')
    i = 'P';
  if (sensores.agentes[2] != '_')
    c = 'P';
  if (sensores.agentes[3] != '_')
    d = 'P';

  ubicacion posActual = {sensores.posF, sensores.posC, sensores.rumbo};
  ubicacion posC = Delante(posActual);
  ubicacion posI = posActual;
  posI.brujula = (Orientacion)((posI.brujula + 7) % 8);
  posI = Delante(posI);
  ubicacion posD = posActual;
  posD.brujula = (Orientacion)((posD.brujula + 1) % 8);
  posD = Delante(posD);

  int t_i = (posI.f >= 0 && posI.f < mtiempo.size() && posI.c >= 0 && posI.c < mtiempo[0].size()) ? mtiempo[posI.f][posI.c] : INT_MAX;
  int t_c = (posC.f >= 0 && posC.f < mtiempo.size() && posC.c >= 0 && posC.c < mtiempo[0].size()) ? mtiempo[posC.f][posC.c] : INT_MAX;
  int t_d = (posD.f >= 0 && posD.f < mtiempo.size() && posD.c >= 0 && posD.c < mtiempo[0].size()) ? mtiempo[posD.f][posD.c] : INT_MAX;

  int pos = SeleccionaCasillaPulgarcito1(i, c, d, t_i, t_c, t_d, tiene_zapatillas);

  switch (pos) {
    case 2: accion = WALK; break;
    case 1: accion = TURN_SL; break;
    case 3: accion = TURN_SR; break;
    default: accion = TURN_SL; break;
  }
  instante++;
  mtiempo[sensores.posF][sensores.posC] = instante;

  return accion;
}


// ─────────────────────────────────────────────────────────────────────────────
// PARTE 2 (DELIBERATIVA)
// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
// NIVEL 2
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Indica si el tipo de superficie es transitable para el Técnico en modo deliberativo (mapa conocido).
 *
 * Son intransitables: Muro 'M', Precipicio 'P', Bosque 'B' y casillas aún desconocidas '?'.
 *
 * @param superficie Carácter de terreno a evaluar.
 * @return true si el Ingeniero puede pisar esa casilla.
 */
bool ComportamientoTecnico::esSuperficieValida(unsigned char superficie, bool zap) const {
  // El técnico no pisa muros, precipicios ni otros agentes.
  // Evita chocar con otros agentes.
  if (superficie == 'M' || superficie == 'P' || superficie == '?') {
    return false;
  }
  // Restricción del bosque para el Técnico
  if (superficie == 'B') {
    return zap;
  }
  return true;
}

/**
 * @brief Detecta si ejecutar una acción provocaría una colisión con el Ingeniero.
 *
 * @param sensores Sensores del turno actual.
 * @param accion   Acción que el Técnico pretende ejecutar.
 * @return true si hay riesgo de colisión con el Ingeniero.
 */
bool ComportamientoTecnico::RiesgoChoqueIngeniero(const Sensores &sensores, Action accion) const {
  switch (accion) {
    case WALK: 
      return sensores.agentes[2] == 'i';
    default:   
      return false;
  }
}

/**
 * @brief Comportamiento del Técnico en Nivel 2.
 *
 * En este nivel el Técnico no tiene objetivo propio: su única tarea
 * es no obstaculizar al Ingeniero. Si puede avanzar lo hace para
 * despejarse; si no, gira en busca de espacio libre.
 *
 * @param sensores Datos sensoriales del turno actual.
 * @return WALK si la casilla de delante es transitable, TURN_SR en caso contrario.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_2(Sensores sensores) {
  int dif = sensores.cota[2] - sensores.cota[0];
  bool transitable = esSuperficieValida(sensores.superficie[2], tiene_zapatillas);
  bool hay_agente  = RiesgoChoqueIngeniero(sensores, WALK);

  // El Técnico no es capaz de subir 2 alturas ni con zapatillas
  if (transitable && !hay_agente && abs(dif) <= 1) {
    return WALK; // Evasión activa hacia adelante
  }

  return TURN_SR; // Gira para buscar una ruta alternativa, se ha encontrado con un obstáculo
}


// ─────────────────────────────────────────────────────────────────────────────
// NIVEL 3
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Comprueba si el Técnico puede ejecutar una acción de movimiento
 *        desde su estado actual.
 *
 * El Técnico no dispone de JUMP, por lo que solo WALK requiere validación.
 * Verifica que esté dentro del mapa, superficie transitable y desnivel admisible (±1 siempre,
 * ya que las zapatillas del Técnico no amplían el rango de altura).
 *
 * @param accion  Acción a validar.
 * @param st      Estado actual del Técnico.
 * @param terreno Matriz de tipos de superficie.
 * @param altura  Matriz de cotas.
 * @return true si la acción es ejecutable.
 */
bool ComportamientoTecnico::CasillaAccesibleTecnico(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) const {
  if (accion != WALK)
    return true; // Giros siempre son viables, el técnico no tiene JUMP

  ubicacion casilla_delante = Delante(st.site);

  if (casilla_delante.f < 0 || casilla_delante.f >= terreno.size() || casilla_delante.c < 0 || casilla_delante.c >= terreno[0].size())
    return false;
  if (!esSuperficieValida(terreno[casilla_delante.f][casilla_delante.c], st.zapatillas))
    return false;

  int dif = altura[casilla_delante.f][casilla_delante.c] - altura[st.site.f][st.site.c];
  return (abs(dif) <= 1); // El técnico no puede escalar desniveles de 2, ni con zapatillas
}

/**
 * @brief Aplica una acción al estado del Técnico y devuelve el estado resultante.
 *
 * Actualiza posición u orientación según la acción. Si la nueva casilla
 * contiene zapatillas ('D'), las activa en el estado resultante.
 *
 * @param accion  Acción a aplicar.
 * @param st      Estado actual del Técnico.
 * @param terreno Matriz de tipos de superficie.
 * @param altura  Matriz de cotas (mantiene firma uniforme con el resto).
 * @return Estado resultante.
 */
EstadoT ComportamientoTecnico::applyT(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) const {
  EstadoT nuevo_st = st;
  
  switch (accion) {
  case WALK:
    nuevo_st.site = Delante(st.site);
    break;
  case TURN_SR:
    nuevo_st.site.brujula = (Orientacion)((nuevo_st.site.brujula + 1) % 8);
    break;
  case TURN_SL:
    nuevo_st.site.brujula = (Orientacion)((nuevo_st.site.brujula + 7) % 8);
    break;
  }
  // Adquisición de zapatillas en el nuevo estadoT
  if (terreno[nuevo_st.site.f][nuevo_st.site.c] == 'D')
    nuevo_st.zapatillas = true;
  if (terreno[nuevo_st.site.f][nuevo_st.site.c] == 'X')
    nuevo_st.puesto_base = true;
  return nuevo_st;
}

/**
 * @brief Heurística admisible para A*: distancia al objetivo.
 *
 * La distancia (máximo de diferencias en fila y columna) es
 * admisible porque el Técnico puede moverse en 8 direcciones y el coste
 * mínimo de WALK es 1.
 *
 * @param actual   Estado actual del Técnico.
 * @param objetivo Estado destino.
 * @return Estimación del coste restante.
 */
int ComportamientoTecnico::Heuristica(const EstadoT &actual, const EstadoT &objetivo) const{
  return max(abs(actual.site.f - objetivo.site.f), abs(actual.site.c - objetivo.site.c));
}

/**
 * @brief Calcula el coste energético de ejecutar una acción desde el estado actual.
 *
 * El coste depende del terreno de la casilla origen y, para WALK,
 * del desnivel entre origen y destino (+5 si sube, -2 si baja).
 * Los giros no tienen ajuste por altura.
 *
 * @param accion  Acción a evaluar.
 * @param st      Estado actual del Técnico.
 * @param terreno Matriz de tipos de superficie.
 * @param altura  Matriz de cotas.
 * @return Coste energético de la acción.
 */
int ComportamientoTecnico::CostoEnergiaTecnico(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) const{
  unsigned char sup = terreno[st.site.f][st.site.c];
  int costo = 0;

  if (accion == WALK) {
    ubicacion destino = Delante(st.site);
    int dif_altura = altura[destino.f][destino.c] - altura[st.site.f][st.site.c];

    if (sup == 'A') {
      costo = 60;
      if (dif_altura > 0)
        costo += 5;
      else if (dif_altura < 0)
        costo -= 2;
    } else if (sup == 'H') {
      costo = 6;
      if (dif_altura > 0)
        costo += 5;
      else if (dif_altura < 0)
        costo -= 2;
    } else if (sup == 'S') {
      costo = 3;
      if (dif_altura > 0)
        costo += 5;
      else if (dif_altura < 0)
        costo -= 2;
    } else {
      costo = 1; // El resto de casillas tienen incremento +0/-0
    }
  } else if (accion == TURN_SL || accion == TURN_SR) {
    if (sup == 'A')
      costo = 5;
    else if (sup == 'H')
      costo = 2;
    else
      costo = 1;
  }
  return costo;
}

/**
 * @brief Encuentra el camino de mínimo coste energético mediante A*.
 *
 * Usa una cola de prioridad ordenada por f = g + h, donde g es el coste
 * acumulado real y h es la heurística de Chebyshev. Los estados se marcan
 * como visitados en la extracción para garantizar optimalidad.
 *
 * @param inicio  Estado de partida del Técnico.
 * @param final   Estado destino (solo se comparan fila y columna).
 * @param terreno Matriz de tipos de superficie.
 * @param altura  Matriz de cotas.
 * @return Secuencia de acciones de mínimo coste, o lista vacía si no existe.
 */
list<Action> ComportamientoTecnico::A_Estrella(const EstadoT &inicio, const EstadoT &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) {
  
  priority_queue<NodoT, vector<NodoT>, ComparaNodos> frontier;
  list<Action> camino_solucion;

  // Matriz 5D para visitados: [Fila][Columna][Orientacion][Zapatillas][PuestoBase]
  // Evito el uso de set y reduzco la complejidad de O(log N) a O(1)
  vector<vector<vector<vector<vector<bool>>>>> visitados(terreno.size(),
                                                 vector<vector<vector<vector<bool>>>>(terreno[0].size(),
                                                                              vector<vector<vector<bool>>>(8,
                                                                                                   vector<vector<bool>>(2,
                                                                                                         vector<bool>(2, false)))));

  NodoT n_inicial;
  n_inicial.estado = inicio;
  n_inicial.g = 0;
  n_inicial.h = Heuristica(inicio, final);
  frontier.push(n_inicial);

  while (!frontier.empty()) {
    NodoT nodo_actual = frontier.top();
    frontier.pop();

    int zapatillas = nodo_actual.estado.zapatillas ? 1 : 0;
    int pb = nodo_actual.estado.puesto_base ? 1 : 0;

    // Saltar estados que ya han sido explorados
    if (visitados[nodo_actual.estado.site.f][nodo_actual.estado.site.c][nodo_actual.estado.site.brujula][zapatillas][pb])
      continue;
    visitados[nodo_actual.estado.site.f][nodo_actual.estado.site.c][nodo_actual.estado.site.brujula][zapatillas][pb] = true;

    // Comprobamos si hemos llegado al destino habiendo pasado por 'X'
    if (nodo_actual.estado.site.f == final.site.f && nodo_actual.estado.site.c == final.site.c) {
      if (nodo_actual.estado.puesto_base) return nodo_actual.secuencia;
    }

    Action accionesPosibles[] = {WALK, TURN_SR, TURN_SL};
    for (Action accion : accionesPosibles) {
      if (CasillaAccesibleTecnico(accion, nodo_actual.estado, terreno, altura)) {
        NodoT hijo;
        hijo.estado = applyT(accion, nodo_actual.estado, terreno, altura);
        hijo.secuencia = nodo_actual.secuencia;
        hijo.secuencia.push_back(accion);
        hijo.g = nodo_actual.g + CostoEnergiaTecnico(accion, nodo_actual.estado, terreno, altura);
        hijo.h = Heuristica(hijo.estado, final);

        frontier.push(hijo);
      }
    }
  }
  return {};
}

/**
 * @brief Comportamiento deliberativo del Técnico para el Nivel 3.
 *
 * Calcula una vez el camino de mínimo coste energético hasta la
 * filtración de Belkanita usando A*, y lo ejecuta acción a acción.
 * Si detecta al Ingeniero en la trayectoria, espera con IDLE.
 *
 * @param sensores Datos sensoriales del turno actual.
 * @return Acción a ejecutar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_3(Sensores sensores) {
  Action accion = IDLE;

  // 1. Si no hay plan, lo calculamos invocando nuestro método de búsqueda
  if (!hayPlan) {
    // EstadoT inicial (nuestra posición actual)
    EstadoT inicio, destino;
    inicio.site.f = sensores.posF;
    inicio.site.c = sensores.posC;
    inicio.site.brujula = sensores.rumbo;
    inicio.zapatillas = tiene_zapatillas;

    // EstadoT final (la posición de destino proporcionada por los sensores)
    destino.site.f = sensores.BelPosF;
    destino.site.c = sensores.BelPosC;
    destino.site.brujula = norte; // La orientación de llegada y las zapatillas dan igual
    destino.zapatillas = false;

    // Llamamos a nuestro algoritmo BFS
    plan = A_Estrella(inicio, destino, mapaResultado, mapaCotas);
    hayPlan = plan.size() != 0;
    VisualizaPlan(inicio.site, plan);
  }

  // 2. Si hay plan, lo ejecutamos paso a paso
  if (hayPlan && plan.size() > 0) {

    accion = plan.front();

    if (RiesgoChoqueIngeniero(sensores, accion)) {
      accion = IDLE; // Pausar ejecución, esperamos que el técnico se aparte y no consumimos energía
    } else {
      plan.pop_front(); // No hay riesgo de choque, hacemos la accion y la eliminamos del plan
    }
  }

  // 3. Si el plan se queda a 0 es porque hemos terminado de ejecutarlo, habríamos llegado a la Belkanita
  if (plan.size() == 0) {
    hayPlan = false;
  }

  return accion;
}


// ─────────────────────────────────────────────────────────────────────────────
// NIVEL 4
// ─────────────────────────────────────────────────────────────────────────────


/**
 * @brief Comportamiento del técnico para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_4(Sensores sensores) {
  return IDLE;
}


// ─────────────────────────────────────────────────────────────────────────────
// NIVEL 5
// ─────────────────────────────────────────────────────────────────────────────


/**
 * @brief Genera una ruta A* desde la posición actual del Técnico hasta el destino.
 *
 * Almacena la ruta en 'plan' y activa 'hayPlan' si se encontró solución.
 *
 * @param dest_f   Fila del destino.
 * @param dest_c   Columna del destino.
 * @param sensores Sensores del turno actual.
 */
void ComportamientoTecnico::GenerarRuta(int dest_f, int dest_c, const Sensores &sensores) {
  EstadoT inicio;
  inicio.site.f = sensores.posF;
  inicio.site.c = sensores.posC;
  inicio.site.brujula = sensores.rumbo;
  inicio.zapatillas = tiene_zapatillas;
  inicio.puesto_base = puesto_base_visitado; // Si ya pasó por 'X', el constraint ya está cumplido

  EstadoT destino;
  destino.site.f = dest_f;
  destino.site.c = dest_c;
  // La orientación final no es restrictiva en la búsqueda

  plan = A_Estrella(inicio, destino, mapaResultado, mapaCotas);
  hayPlan = !plan.empty();
}

/**
 * @brief Extrae y devuelve la siguiente acción del plan de navegación.
 *
 * Si el Ingeniero bloquea el paso, marca su casilla como muro temporalmente,
 * recalcula la ruta por A* para rodearle y restaura el mapa.
 * Si no hay ruta alternativa, devuelve IDLE.
 *
 * @param sensores Sensores del turno actual.
 * @return Siguiente acción del plan, o IDLE si hay bloqueo sin alternativa.
 */
Action ComportamientoTecnico::AvanzarCasilla(const Sensores &sensores) {
  if (!hayPlan || plan.empty())
    return IDLE;

  Action accion = plan.front();

  // Evitamos colisiones ortogonales
  if ((accion == WALK) && (sensores.agentes[2] != '_')) {

      ubicacion pos_actual = {sensores.posF, sensores.posC, sensores.rumbo};
      ubicacion pos_delante = Delante(pos_actual);
      // 1. Determinar coordenadas de la casilla frontal ocupada
      int f_ing = pos_delante.f;
      int c_ing = pos_delante.c;

      // Si el Ingeniero está exactamente en nuestro destino, esperamos
      if (f_ing == destino_f && c_ing == destino_c) {
        return IDLE;
      }

      // Bloqueamos temporalmente la casilla del Ingeniero en el mapa
      unsigned char original = mapaResultado[f_ing][c_ing];
      mapaResultado[f_ing][c_ing] = 'M';

      // Recalculamos la ruta evitando esa casilla
      plan.clear();
      GenerarRuta(destino_f, destino_c, sensores);

      // Restauramos el mapa real
      mapaResultado[f_ing][c_ing] = original;

      if (!hayPlan || plan.empty())
        return IDLE; // Sin ruta alternativa

      // Ejecutamos la primera acción de la ruta alternativa
      Action accion_evasiva = plan.front();

      // Precaución: si la alternativa también está bloqueada, esperamos
      if (accion_evasiva == WALK && sensores.agentes[2] != '_')
        return IDLE;

      plan.pop_front();
      if (plan.empty())
        hayPlan = false;
      return accion_evasiva;
  }
  // Caso normal, avanzamos según el plan
  plan.pop_front();
  if (plan.empty())
    hayPlan = false;

  return accion;
}

/**
 * @brief Comportamiento del Técnico para el Nivel 5.
 *
 * Máquina de 3 estados:
 *   Fase 0: Esperar invocación del Ingeniero (sensor venpaca).
 *   Fase 1: Navegar hasta la casilla indicada por GotoF/GotoC.
 *   Fase 2: Sincronizarse con el Ingeniero y ejecutar INSTALL.
 *
 * @param sensores Datos sensoriales del turno actual.
 * @return Acción a ejecutar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_5(Sensores sensores) {
  Action accion = IDLE;

  // DEBUG
  // cout << "[TEC] Fase actual: " << fase
  //      << " | Pos: (" << sensores.posF << ", " << sensores.posC << ")"
  //      << " | Goto: (" << sensores.GotoF << ", " << sensores.GotoC << ")"
  //      << " | venpaca: " << sensores.venpaca
  //      << " | Destino: (" << destino_f << ", " << destino_c << ")" << " | HayPlan: " << hayPlan << endl;

  switch (fase) {
    case 0: {
      if (sensores.venpaca) {
        // El Ingeniero ha ejecutado COME: recibimos sus coordenadas
        destino_f = sensores.GotoF;
        destino_c = sensores.GotoC;

        GenerarRuta(sensores.GotoF, sensores.GotoC, sensores);
        fase = 1;
      }
      break;
    }

    case 1: {
      // Mira si ha llegado a la casilla del Goto
      if (sensores.posF == destino_f && sensores.posC == destino_c) {
        // No hago break para garantizar sincronización con el Ingeniero, que en este mismo turno está en su fase de INSTALL, es decir, si llego a la casilla y estoy bien orientado.
        fase = 2;
      } else {
        if (!hayPlan)
          GenerarRuta(destino_f, destino_c, sensores);

        accion = hayPlan ? AvanzarCasilla(sensores) : ExploracionDirigida(destino_f,destino_c,sensores);
        break;
      }
    }

    case 2: {
      // DEBUG
      // cout << "[TEC-Fase2] enfrente: " << sensores.enfrente
      //      << " | agentes[2]: '" << sensores.agentes[2] << "'" << endl;
      // Sincronización e Instalación
      if (sensores.enfrente) {
        // Condición óptima: el Ingeniero también ve al Técnico
        accion = INSTALL;
        puesto_base_visitado = true; // Ya cumplimos el requisito de 'X' en el primer viaje
        fase = 0; // Reiniciar el ciclo para esperar el siguiente tramo
      } else if (sensores.agentes[2] == 'i') {
        // El Técnico está mirando al Ingeniero, pero el Ingeniero
        // aún está realizando adecuaciones (DIG/RAISE) o girándose.
        accion = IDLE;
      } else {
        // El Técnico no está mirando al Ingeniero. Debe rotar sobre sí mismo
        // para encontrarlo en las casillas adyacentes.
        accion = TURN_SR;
      }
      break;
    }
  }

  return accion;
}


// ─────────────────────────────────────────────────────────────────────────────
// NIVEL 6
// ─────────────────────────────────────────────────────────────────────────────


/**
 * @brief Decide la acción reactiva que más acerca al Técnico a un destino dado, considerando solo las tres casillas de delante.
 *
 * A diferencia del Nivel 1, que prioriza casillas no visitadas para maximizar exploración, 
 * esta función prioriza la casilla que minimiza la distancia euclídea al destino.
 *
 * Algoritmo:
 *   1. Calcula las tres casillas adyacentes (45° izq, frente, 45° dch).
 *   2. Comprueba viabilidad real con TransitableNivel1 + sensor de agentes.
 *   3. Asigna distancia² al destino (INT_MAX si la casilla no es viable).
 *   4. Elige la acción que lleva a la casilla con menor distancia.
 *   5. Si las tres están bloqueadas, devuelve TURN_SR para reorientarse.
 *
 * @param dest_f   Fila del destino al que se quiere acercar.
 * @param dest_c   Columna del destino.
 * @param sensores Sensores del turno actual.
 * @return WALK si conviene avanzar, TURN_SL/TURN_SR para reorientarse.
 */
Action ComportamientoTecnico::ExploracionDirigida(int dest_f, int dest_c, const Sensores &sensores) {
  Action accion = IDLE;

  // Actualizamos los sensores (por si acaso no se ha hecho en este ciclo)
  ActualizarMapa(sensores);
  if (sensores.superficie[0] == 'D')
    tiene_zapatillas = true;

  // Calculamos las posiciones adyacentes relativas al agente
  ubicacion posActual = {sensores.posF, sensores.posC, sensores.rumbo};
  ubicacion posC = Delante(posActual);

  ubicacion posI = posActual;
  posI.brujula = (Orientacion)((posI.brujula + 7) % 8);
  posI = Delante(posI);

  ubicacion posD = posActual;
  posD.brujula = (Orientacion)((posD.brujula + 1) % 8);
  posD = Delante(posD);

  // Evaluamos la viabilidad topológica REAL de las casillas adyacentes (usamos el mismo criterio que Nivel 1)
  char supI = TransitableNivel1(sensores.superficie[1], sensores.cota[1] - sensores.cota[0], tiene_zapatillas);
  char supC = TransitableNivel1(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tiene_zapatillas);
  char supD = TransitableNivel1(sensores.superficie[3], sensores.cota[3] - sensores.cota[0], tiene_zapatillas);

  // Descartamos casillas ocupadas por agentes
  if (sensores.agentes[1] != '_')
    supI = 'P';
  if (sensores.agentes[2] != '_')
    supC = 'P';
  if (sensores.agentes[3] != '_')
    supD = 'P';

  // Calculamos la distancia (heurística euclídea al cuadrado) desde las 3 opciones hasta el destino
  // Si una casilla es un Precipicio ('P') o obstáculo, le asignamos una distancia infinita
  int distI = (supI != 'P') ? ((posI.f - dest_f) * (posI.f - dest_f) + (posI.c - dest_c) * (posI.c - dest_c)) : INT_MAX;
  int distC = (supC != 'P') ? ((posC.f - dest_f) * (posC.f - dest_f) + (posC.c - dest_c) * (posC.c - dest_c)) : INT_MAX;
  int distD = (supD != 'P') ? ((posD.f - dest_f) * (posD.f - dest_f) + (posD.c - dest_c) * (posD.c - dest_c)) : INT_MAX;

  // Si todas las opciones frontales están bloqueadas, forzamos un giro para mirar a otro lado
  if (distI == INT_MAX && distC == INT_MAX && distD == INT_MAX) {
    return TURN_SR;
  }

  // Buscamos la casilla viable que más nos acerque al destino
  int minDist = min(distC, min(distI, distD));

  if (minDist == distC)
    return WALK;
  if (minDist == distI)
    return TURN_SL;
  if (minDist == distD)
    return TURN_SR;

  return TURN_SR;
}

/**
 * @brief Comportamiento del Técnico para el Nivel 6.
 *
 * Estrategia en dos modos según el estado del Ingeniero:
 *   Modo autónomo (fase 0 y sin invocación):
 *     El Técnico explora reactivamente con la lógica del Nivel 1.
 *     Esto contribuye a descubrir mapa y reduce el tiempo total de planificación.
 *
 *   Modo subordinado (cuando llega venpaca o ya se ha avanzado de fase):
 *     Pasa a la lógica del Nivel 5.
 *
 * @param sensores Datos sensoriales del turno actual.
 * @return Acción a ejecutar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_6(Sensores sensores) {
  // Modo exploración autónoma: hasta la primera invocación del Ingeniero
  if (fase == 0 && !sensores.venpaca) {
    return ComportamientoTecnicoNivel_1(sensores);
  }

  // Modo construcción: lógica completa del Nivel 5 (espera, navega, INSTALL)
  return ComportamientoTecnicoNivel_5(sensores);
}

// =========================================================================
// FUNCIONES PROPORCIONADAS
// =========================================================================

/**
 * @brief Actualiza el mapaResultado y mapaCotas con la información de los sensores.
 * @param sensores Datos actuales de los sensores.
 */
void ComportamientoTecnico::ActualizarMapa(Sensores sensores) {
  mapaResultado[sensores.posF][sensores.posC] = sensores.superficie[0];
  mapaCotas[sensores.posF][sensores.posC] = sensores.cota[0];

  int pos = 1;
  switch (sensores.rumbo) {
  case norte:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++) {
        mapaResultado[sensores.posF - j][sensores.posC + i] = sensores.superficie[pos];
        mapaCotas[sensores.posF - j][sensores.posC + i] = sensores.cota[pos++];
      }
    break;
  case noreste:
    mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[1];
    mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[1];
    mapaResultado[sensores.posF - 1][sensores.posC + 1] = sensores.superficie[2];
    mapaCotas[sensores.posF - 1][sensores.posC + 1] = sensores.cota[2];
    mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[3];
    mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[3];
    mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[4];
    mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[4];
    mapaResultado[sensores.posF - 2][sensores.posC + 1] = sensores.superficie[5];
    mapaCotas[sensores.posF - 2][sensores.posC + 1] = sensores.cota[5];
    mapaResultado[sensores.posF - 2][sensores.posC + 2] = sensores.superficie[6];
    mapaCotas[sensores.posF - 2][sensores.posC + 2] = sensores.cota[6];
    mapaResultado[sensores.posF - 1][sensores.posC + 2] = sensores.superficie[7];
    mapaCotas[sensores.posF - 1][sensores.posC + 2] = sensores.cota[7];
    mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[8];
    mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[8];
    mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[9];
    mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[9];
    mapaResultado[sensores.posF - 3][sensores.posC + 1] = sensores.superficie[10];
    mapaCotas[sensores.posF - 3][sensores.posC + 1] = sensores.cota[10];
    mapaResultado[sensores.posF - 3][sensores.posC + 2] = sensores.superficie[11];
    mapaCotas[sensores.posF - 3][sensores.posC + 2] = sensores.cota[11];
    mapaResultado[sensores.posF - 3][sensores.posC + 3] = sensores.superficie[12];
    mapaCotas[sensores.posF - 3][sensores.posC + 3] = sensores.cota[12];
    mapaResultado[sensores.posF - 2][sensores.posC + 3] = sensores.superficie[13];
    mapaCotas[sensores.posF - 2][sensores.posC + 3] = sensores.cota[13];
    mapaResultado[sensores.posF - 1][sensores.posC + 3] = sensores.superficie[14];
    mapaCotas[sensores.posF - 1][sensores.posC + 3] = sensores.cota[14];
    mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[15];
    mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[15];
    break;
  case este:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++) {
        mapaResultado[sensores.posF + i][sensores.posC + j] = sensores.superficie[pos];
        mapaCotas[sensores.posF + i][sensores.posC + j] = sensores.cota[pos++];
      }
    break;
  case sureste:
    mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[1];
    mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[1];
    mapaResultado[sensores.posF + 1][sensores.posC + 1] = sensores.superficie[2];
    mapaCotas[sensores.posF + 1][sensores.posC + 1] = sensores.cota[2];
    mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[3];
    mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[3];
    mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[4];
    mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[4];
    mapaResultado[sensores.posF + 1][sensores.posC + 2] = sensores.superficie[5];
    mapaCotas[sensores.posF + 1][sensores.posC + 2] = sensores.cota[5];
    mapaResultado[sensores.posF + 2][sensores.posC + 2] = sensores.superficie[6];
    mapaCotas[sensores.posF + 2][sensores.posC + 2] = sensores.cota[6];
    mapaResultado[sensores.posF + 2][sensores.posC + 1] = sensores.superficie[7];
    mapaCotas[sensores.posF + 2][sensores.posC + 1] = sensores.cota[7];
    mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[8];
    mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[8];
    mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[9];
    mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[9];
    mapaResultado[sensores.posF + 1][sensores.posC + 3] = sensores.superficie[10];
    mapaCotas[sensores.posF + 1][sensores.posC + 3] = sensores.cota[10];
    mapaResultado[sensores.posF + 2][sensores.posC + 3] = sensores.superficie[11];
    mapaCotas[sensores.posF + 2][sensores.posC + 3] = sensores.cota[11];
    mapaResultado[sensores.posF + 3][sensores.posC + 3] = sensores.superficie[12];
    mapaCotas[sensores.posF + 3][sensores.posC + 3] = sensores.cota[12];
    mapaResultado[sensores.posF + 3][sensores.posC + 2] = sensores.superficie[13];
    mapaCotas[sensores.posF + 3][sensores.posC + 2] = sensores.cota[13];
    mapaResultado[sensores.posF + 3][sensores.posC + 1] = sensores.superficie[14];
    mapaCotas[sensores.posF + 3][sensores.posC + 1] = sensores.cota[14];
    mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[15];
    mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[15];
    break;
  case sur:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++) {
        mapaResultado[sensores.posF + j][sensores.posC - i] = sensores.superficie[pos];
        mapaCotas[sensores.posF + j][sensores.posC - i] = sensores.cota[pos++];
      }
    break;
  case suroeste:
    mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[1];
    mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[1];
    mapaResultado[sensores.posF + 1][sensores.posC - 1] = sensores.superficie[2];
    mapaCotas[sensores.posF + 1][sensores.posC - 1] = sensores.cota[2];
    mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[3];
    mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[3];
    mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[4];
    mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[4];
    mapaResultado[sensores.posF + 2][sensores.posC - 1] = sensores.superficie[5];
    mapaCotas[sensores.posF + 2][sensores.posC - 1] = sensores.cota[5];
    mapaResultado[sensores.posF + 2][sensores.posC - 2] = sensores.superficie[6];
    mapaCotas[sensores.posF + 2][sensores.posC - 2] = sensores.cota[6];
    mapaResultado[sensores.posF + 1][sensores.posC - 2] = sensores.superficie[7];
    mapaCotas[sensores.posF + 1][sensores.posC - 2] = sensores.cota[7];
    mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[8];
    mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[8];
    mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[9];
    mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[9];
    mapaResultado[sensores.posF + 3][sensores.posC - 1] = sensores.superficie[10];
    mapaCotas[sensores.posF + 3][sensores.posC - 1] = sensores.cota[10];
    mapaResultado[sensores.posF + 3][sensores.posC - 2] = sensores.superficie[11];
    mapaCotas[sensores.posF + 3][sensores.posC - 2] = sensores.cota[11];
    mapaResultado[sensores.posF + 3][sensores.posC - 3] = sensores.superficie[12];
    mapaCotas[sensores.posF + 3][sensores.posC - 3] = sensores.cota[12];
    mapaResultado[sensores.posF + 2][sensores.posC - 3] = sensores.superficie[13];
    mapaCotas[sensores.posF + 2][sensores.posC - 3] = sensores.cota[13];
    mapaResultado[sensores.posF + 1][sensores.posC - 3] = sensores.superficie[14];
    mapaCotas[sensores.posF + 1][sensores.posC - 3] = sensores.cota[14];
    mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[15];
    mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[15];
    break;
  case oeste:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++) {
        mapaResultado[sensores.posF - i][sensores.posC - j] = sensores.superficie[pos];
        mapaCotas[sensores.posF - i][sensores.posC - j] = sensores.cota[pos++];
      }
    break;
  case noroeste:
    mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[1];
    mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[1];
    mapaResultado[sensores.posF - 1][sensores.posC - 1] = sensores.superficie[2];
    mapaCotas[sensores.posF - 1][sensores.posC - 1] = sensores.cota[2];
    mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[3];
    mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[3];
    mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[4];
    mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[4];
    mapaResultado[sensores.posF - 1][sensores.posC - 2] = sensores.superficie[5];
    mapaCotas[sensores.posF - 1][sensores.posC - 2] = sensores.cota[5];
    mapaResultado[sensores.posF - 2][sensores.posC - 2] = sensores.superficie[6];
    mapaCotas[sensores.posF - 2][sensores.posC - 2] = sensores.cota[6];
    mapaResultado[sensores.posF - 2][sensores.posC - 1] = sensores.superficie[7];
    mapaCotas[sensores.posF - 2][sensores.posC - 1] = sensores.cota[7];
    mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[8];
    mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[8];
    mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[9];
    mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[9];
    mapaResultado[sensores.posF - 1][sensores.posC - 3] = sensores.superficie[10];
    mapaCotas[sensores.posF - 1][sensores.posC - 3] = sensores.cota[10];
    mapaResultado[sensores.posF - 2][sensores.posC - 3] = sensores.superficie[11];
    mapaCotas[sensores.posF - 2][sensores.posC - 3] = sensores.cota[11];
    mapaResultado[sensores.posF - 3][sensores.posC - 3] = sensores.superficie[12];
    mapaCotas[sensores.posF - 3][sensores.posC - 3] = sensores.cota[12];
    mapaResultado[sensores.posF - 3][sensores.posC - 2] = sensores.superficie[13];
    mapaCotas[sensores.posF - 3][sensores.posC - 2] = sensores.cota[13];
    mapaResultado[sensores.posF - 3][sensores.posC - 1] = sensores.superficie[14];
    mapaCotas[sensores.posF - 3][sensores.posC - 1] = sensores.cota[14];
    mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[15];
    mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[15];
    break;
  }
}

/**
 * @brief Determina si una casilla es transitable para el técnico.
 * En esta práctica, si el técnico tiene zapatillas, el bosque ('B') es transitable.
 * @param f Fila de la casilla.
 * @param c Columna de la casilla.
 * @param tieneZapatillas Indica si el agente posee las zapatillas.
 * @return true si la casilla es transitable.
 */
bool ComportamientoTecnico::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas) {
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size())
    return false;
  return es_camino(mapaResultado[f][c]); // Solo 'C', 'S', 'D', 'U' son transitables en Nivel 0
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el técnico: desnivel máximo siempre 1.
 * @param actual EstadoT actual del agente (fila, columna, orientacion).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoTecnico::EsAccesiblePorAltura(const ubicacion &actual) {
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size())
    return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (desnivel > 1)
    return false;
  return true;
}

/**
 * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal según la orientación actual (8 direcciones).
 * @param actual EstadoT actual del agente (fila, columna, orientacion).
 * @return EstadoT con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoTecnico::Delante(const ubicacion &actual) const {
  ubicacion delante = actual;
  switch (actual.brujula) {
  case 0:
    delante.f--;
    break; // norte
  case 1:
    delante.f--;
    delante.c++;
    break; // noreste
  case 2:
    delante.c++;
    break; // este
  case 3:
    delante.f++;
    delante.c++;
    break; // sureste
  case 4:
    delante.f++;
    break; // sur
  case 5:
    delante.f++;
    delante.c--;
    break; // suroeste
  case 6:
    delante.c--;
    break; // oeste
  case 7:
    delante.f--;
    delante.c--;
    break; // noroeste
  }
  return delante;
}

/**
 * @brief Imprime por consola la secuencia de acciones de un plan.
 *
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::PintaPlan(const list<Action> &plan) {
  auto it = plan.begin();
  while (it != plan.end()) {
    if (*it == WALK) {
      cout << "W ";
    } else if (*it == JUMP) {
      cout << "J ";
    } else if (*it == TURN_SR) {
      cout << "r ";
    } else if (*it == TURN_SL) {
      cout << "l ";
    } else if (*it == COME) {
      cout << "C ";
    } else if (*it == IDLE) {
      cout << "I ";
    } else {
      cout << "-_ ";
    }
    it++;
  }
  cout << "( longitud " << plan.size() << ")" << endl;
}

/**
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa 2D.
 *
 * @param st    EstadoT de partida.
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::VisualizaPlan(const ubicacion &st,
                                          const list<Action> &plan) {
  listaPlanCasillas.clear();
  ubicacion cst = st;

  listaPlanCasillas.push_back({cst.f, cst.c, WALK});
  auto it = plan.begin();
  while (it != plan.end()) {

    switch (*it) {
    case JUMP:
      switch (cst.brujula) {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, JUMP});
    case WALK:
      switch (cst.brujula) {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, WALK});
      break;
    case TURN_SR:
      cst.brujula = (Orientacion)(((int)cst.brujula + 1) % 8);
      break;
    case TURN_SL:
      cst.brujula = (Orientacion)(((int)cst.brujula + 7) % 8);
      break;
    }
    it++;
  }
}
