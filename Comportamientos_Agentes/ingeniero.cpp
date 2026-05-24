#include "ingeniero.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>

using namespace std;

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoIngeniero::think(Sensores sensores) {
  Action accion = IDLE;

  // Decisión del agente según el nivel
  switch (sensores.nivel) {
  case 0:
    accion = ComportamientoIngenieroNivel_0(sensores);
    break;
  case 1:
    accion = ComportamientoIngenieroNivel_1(sensores);
    break;
  case 2:
    accion = ComportamientoIngenieroNivel_2(sensores);
    break;
  case 3:
    accion = ComportamientoIngenieroNivel_3(sensores);
    break;
  case 4:
    accion = ComportamientoIngenieroNivel_4(sensores);
    break;
  case 5:
    accion = ComportamientoIngenieroNivel_5(sensores);
    break;
  case 6:
    accion = ComportamientoIngenieroNivel_6(sensores);
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
bool ComportamientoIngeniero::es_camino(unsigned char c) const {
  return (c == 'C' || c == 'D' || c == 'U');
}

/**
 * @brief Determina si una casilla es accesible según la diferencia de altura.
 *
 * Si la diferencia de cota es <= 1 (o <= 2 con zapatillas), la casilla
 * es accesible y se devuelve su tipo de terreno. En caso contrario, se
 * considera un precipicio y se devuelve 'P'.
 *
 * @param terreno  Tipo de superficie de la casilla destino.
 * @param dif_cota Diferencia de altura entre la casilla actual y la destino.
 * @param zap  true si el agente dispone de zapatillas (permite saltar 2).
 * @return El carácter de terreno si es accesible, 'P' si no lo es.
 */
char ComportamientoIngeniero::ViablePorAltura(char terreno, int dif_cota, bool zap) const {
  if (abs(dif_cota) <= 1 || (zap && abs(dif_cota) <= 2))
    return terreno;
  else
    return 'P';
}

/**
 * @brief Decide la mejor opción para el Nivel 0 usando el mapa de pulgarcito.
 * @param i terreno que hay en la posición 1 de superficie (45 izq).
 * @param c terreno que hay en la posición 2 de supeficie (justo delante).
 * @param d terreno que hay en la posición 3 de supeficie (45 dch).
 * @param t_i tiempo de última visita a la casilla izquierda.
 * @param t_c tiempo de última visita a la casilla central.
 * @param t_d tiempo de última visita a la casilla derecha.
 * @param zap indica si estoy en posesión de las zapatillas.
 * @return 2 → WALK | 1 → TURN_SL | 3 → TURN_SR | 0 → sin opción (gira por defecto).
 */
int ComportamientoIngeniero::SeleccionaCasillaPulgarcito0(char i, char c, char d, int t_i, int t_c, int t_d, bool zap) const {
  // Prioridad 1: Meta
  if (c == 'U') return 2; // WALK
  else if (i == 'U') return 1; // TURN_SL
  else if (d == 'U') return 3; // TURN_SR

  // Prioridad 2: Zapatillas (si no las tenemos)
  if (!zap) {
    if (c == 'D') return 2;
    else if (i == 'D') return 1;
    else if (d == 'D') return 3;
  }

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

/**
 * @brief Comportamiento reactivo de nivel 0 basado en el "Mapa de Pulgarcito".
 *
 * Cada turno consulta las tres casillas accesibles (izquierda, frente, derecha)
 * y elige moverse hacia aquella que hace más tiempo que no se ha visitado,
 * priorizando siempre la meta y las zapatillas si aún no se poseen.
 *
 * @param sensores Datos sensoriales del agente en el turno actual.
 * @return Acción a ejecutar: WALK, TURN_SL, TURN_SR o IDLE.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_0(Sensores sensores) {
  Action accion = IDLE;

  // Poner el valor de los sensores de visión sobre los mapas
  ActualizarMapa(sensores);

  // Primero comprobamos si la casilla en la que nos situamos es la de las Zapatillas ('D') actualizando su variable de estado
  if (sensores.superficie[0] == 'D')
    tiene_zapatillas = true;

  // Comprobamos si la casilla en la que nos situamos es la Planta de Residuos ('U'), en cuyo caso habríamos terminado
  if (sensores.superficie[0] == 'U')
    return IDLE;

  // Extraemos el valor de las casillas centro, 45º izquierda, 45º derecha
  // El valor del char será la superficie de la casilla si se puede acceder a ella y Precipicio'P' en caso contrario
  char i = ViablePorAltura(sensores.superficie[1], sensores.cota[1] - sensores.cota[0], tiene_zapatillas);
  char c = ViablePorAltura(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tiene_zapatillas);
  char d = ViablePorAltura(sensores.superficie[3], sensores.cota[3] - sensores.cota[0], tiene_zapatillas);

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

  // Tras obtener las superficies i,c,d los tiempos almacenados en esas casillas t_i,t_c,t_d y comprobar que no ha habido choques, planteamos que hacer en la función SeleccionaCasillaPulgarcito
  int pos = SeleccionaCasillaPulgarcito0(i, c, d, t_i, t_c, t_d, tiene_zapatillas);

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
 * Agua ('A'), Bosque ('B') y Precipicio ('P'), independientemente
 * de la altura. Si el terreno supera ese filtro, delega en
 * ViablePorAltura para comprobar el desnivel.
 *
 * @param terreno   Tipo de superficie de la casilla destino.
 * @param dif_cota  Diferencia de altura entre casilla actual y destino.
 * @param zap       true si el agente posee las zapatillas.
 * @return El carácter de terreno si es transitable, 'P' si no lo es.
 */
char ComportamientoIngeniero::TransitableNivel1(char terreno, int dif_cota, bool zap) const{
  if (terreno == 'M' || terreno == 'A' || terreno == 'P' || terreno == 'B')
    return 'P';

  return ViablePorAltura(terreno, dif_cota, zap);
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
int ComportamientoIngeniero::SeleccionaCasillaPulgarcito1(char i, char c, char d, int t_i, int t_c, int t_d, bool zap) const {
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

  if (min_tiempo == t_c) return 2;
  if (min_tiempo == t_i) return 1;
  if (min_tiempo == t_d) return 3;

  return 0;
}

/**
 * @brief Comportamiento reactivo del ingeniero para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_1(Sensores sensores) // He aplicado la misma lógica que había en el nivel 0, por lo que he eliminado comentarios
{
  Action accion = IDLE;

  ActualizarMapa(sensores);

  if (sensores.superficie[0] == 'D')
    tiene_zapatillas = true;

  char i = TransitableNivel1(sensores.superficie[1], sensores.cota[1] - sensores.cota[0], tiene_zapatillas); // Si la altura es <= 1 o tiene zapatillas y es <= 2 es transitable
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
// NIVEL 2 - Búsqueda en Anchura (BFS)
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Indica si el tipo de superficie es transitable para el Ingeniero en modo deliberativo (mapa conocido).
 *
 * Son intransitables: Muro 'M', Precipicio 'P', Bosque 'B' y casillas aún desconocidas '?'.
 *
 * @param superficie Carácter de terreno a evaluar.
 * @return true si el Ingeniero puede pisar esa casilla.
 */
bool ComportamientoIngeniero::esSuperficieValida(unsigned char superficie) const {
  return (superficie != 'M' && superficie != 'P' && superficie != 'B' && superficie != '?');
}

/**
 * @brief Comprueba si el Ingeniero puede ejecutar una acción de movimiento desde su estado actual.
 *
 * Para WALK: verifica que la casilla de delante esté dentro del mapa, sea transitable y el desnivel sea admisible (±1 sin zapatillas, ±2 con ellas).
 * Para JUMP: aplica las mismas comprobaciones que WALK a la casilla intermedia y a la casilla destino. El resto de acciones siempre son válidas.
 *
 * @param accion  Acción que se quiere validar.
 * @param st      Estado actual del Ingeniero (posición, orientación, zapatillas).
 * @param terreno Matriz de tipos de superficie del mapa.
 * @param altura  Matriz de cotas del mapa.
 * @return true si la acción es ejecutable desde el estado dado.
 */
bool ComportamientoIngeniero::CasillaAccesibleIngeniero(Action accion, const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) const {
  // Si no avanzamos, la acción es válida
  if (accion != WALK && accion != JUMP)
    return true;

  ubicacion casilla_delante = Delante(st.site);

  if (casilla_delante.f < 0 || casilla_delante.f >= terreno.size() || casilla_delante.c < 0 || casilla_delante.c >= terreno[0].size())
    return false;

  if (!esSuperficieValida(terreno[casilla_delante.f][casilla_delante.c]))
    return false;
  
  // Ingeniero tiene como casillas intransitables Muro 'M', Precipicio 'P' y Bosque 'B'
  if (accion == WALK) {
    int dif_altura = altura[casilla_delante.f][casilla_delante.c] - altura[st.site.f][st.site.c];
    return (abs(dif_altura) <= 1 || (st.zapatillas && abs(dif_altura) <= 2));
  }
  else if (accion == JUMP) {
    ubicacion casilla_jump = Delante(casilla_delante);
    // Comprobación casilla destino
    if (casilla_jump.f < 0 || casilla_jump.f >= terreno.size() || casilla_jump.c < 0 || casilla_jump.c >= terreno[0].size())
      return false;
    if (!esSuperficieValida(terreno[casilla_jump.f][casilla_jump.c]))
      return false;

    int dif_altura = altura[casilla_jump.f][casilla_jump.c] - altura[st.site.f][st.site.c];
    return (abs(dif_altura) <= 1 || (st.zapatillas && abs(dif_altura) <= 2));
  }
  return false;
}

/**
 * @brief Aplica una acción al estado actual y devuelve el estado resultante.
 *
 * Calcula la nueva posición u orientación según la acción. Si la nueva
 * casilla contiene zapatillas ('D'), las activa en el estado resultante.
 *
 * @param accion  Acción a aplicar.
 * @param st      Estado actual del Ingeniero.
 * @param terreno Matriz de tipos de superficie (para detectar zapatillas).
 * @param altura  Matriz de cotas.
 * @return Estado resultante tras ejecutar la acción.
 */
EstadoI ComportamientoIngeniero::applyI(Action accion, const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) const {
  EstadoI nuevo_st = st;
  
  switch (accion) {
  case WALK:
    nuevo_st.site = Delante(st.site);
    nuevo_st.walk = true;
    break;
  case JUMP:
    nuevo_st.site = Delante(Delante(st.site));
    break;
  case TURN_SR:
    nuevo_st.site.brujula = (Orientacion)((nuevo_st.site.brujula + 1) % 8);
    nuevo_st.right = true;
    break;
  case TURN_SL:
    nuevo_st.site.brujula = (Orientacion)((nuevo_st.site.brujula + 7) % 8);
    nuevo_st.left = true;
    break;
  }
  // Adquisición de zapatillas en el nuevo estado
  if (terreno[nuevo_st.site.f][nuevo_st.site.c] == 'D')
    nuevo_st.zapatillas = true;
    
  return nuevo_st;
}

/**
 * @brief Busca el camino de menor número de pasos desde inicio hasta final
 *        usando Búsqueda en Anchura (BFS).
 *
 * Explora los estados en orden creciente de distancia. Usa una matriz 4D
 * [fila][columna][orientacion][zapatillas] para marcar estados visitados
 * en O(1), evitando el coste O(log N) de un set.
 *
 * @param inicio  Estado de partida del Ingeniero.
 * @param final   Estado destino (solo se comparan fila y columna).
 * @param terreno Matriz de tipos de superficie.
 * @param altura  Matriz de cotas.
 * @return Secuencia de acciones que lleva al destino, o lista vacía si no existe.
 */
list<Action> ComportamientoIngeniero::B_Anchura(const EstadoI &inicio, const EstadoI &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) {
  NodoI nodo_actual;
  list<NodoI> frontera; // Actuará como si fuese una cola FIFO, nodos abiertos pero no explorados

  // Matriz 7D para visitados: [Fila][Columna][Orientacion][Zapatillas][Walk][Right][Left]
  // Es necesario incluir los 3 flags porque (f,c,brujula,zap) con walk=false es un estado
  // DISTINTO a (f,c,brujula,zap) con walk=true. Sin esta distinción, el BFS descartaría
  // caminos que aún pueden alcanzar la meta con todos los flags activos.
  vector<vector<vector<vector<vector<vector<vector<bool>>>>>>> visitados(
      terreno.size(),
      vector<vector<vector<vector<vector<vector<bool>>>>>>(terreno[0].size(),
          vector<vector<vector<vector<vector<bool>>>>>(8,
              vector<vector<vector<vector<bool>>>>(2,
                  vector<vector<vector<bool>>>(2,
                      vector<vector<bool>>(2,
                          vector<bool>(2, false)))))));

  nodo_actual.estado = inicio;

  frontera.push_back(nodo_actual); // Insertamos el Nodo Raíz en la frontera (distancia 0)

  // Si la fila y columna del nodo inicio y destino coinciden ya se ha llegado a la Belkanita
  if (inicio.site.f == final.site.f && inicio.site.c == final.site.c)
    return {};

  while (!frontera.empty()) {
    // 1. Extraemos los nodos usando FIFO para que haga primero los de distancias menores y luego mayores
    nodo_actual = frontera.front();
    frontera.pop_front();

    // 2. Control de nodos visitados: el estado completo incluye los 3 flags de restricción
    int zap = nodo_actual.estado.zapatillas ? 1 : 0;
    int iw  = nodo_actual.estado.walk  ? 1 : 0;
    int ir  = nodo_actual.estado.right ? 1 : 0;
    int il  = nodo_actual.estado.left  ? 1 : 0;
    // vector<bool> usa bits empaquetados y no admite bool&: accedemos dos veces
    if (visitados[nodo_actual.estado.site.f][nodo_actual.estado.site.c]
                 [nodo_actual.estado.site.brujula][zap][iw][ir][il])
      continue; // Si ya se ha visitado este estado exacto, no se vuelve a explorar
    visitados[nodo_actual.estado.site.f][nodo_actual.estado.site.c]
             [nodo_actual.estado.site.brujula][zap][iw][ir][il] = true; // Marcamos el estado como explorado

    // Si llegmaos a la solucion sale del while y devuelve camino_solucion
    // En B_Anchura:
    if (nodo_actual.estado.site.f == final.site.f && nodo_actual.estado.site.c == final.site.c) {
        // Solo si el camino que ha llegado a la meta cumple los requisitos, lo aceptamos
        if (nodo_actual.estado.walk && nodo_actual.estado.left && nodo_actual.estado.right) {
            return nodo_actual.secuencia;
        }
        // Si no cumple, el algoritmo sigue buscando. ¡No hagas return aquí!
    }

    // Añadimos a la cola una copia de lo que ya llevaba mas lo que pasaría si hace cada una de sus acciones
    Action accionesPosibles[] = {WALK, JUMP, TURN_SR, TURN_SL};
    for (Action accion : accionesPosibles) {
      if (CasillaAccesibleIngeniero(accion, nodo_actual.estado, terreno, altura)) {
        NodoI hijo = nodo_actual;
        hijo.estado = applyI(accion, nodo_actual.estado, terreno, altura);
        hijo.secuencia.push_back(accion);
        frontera.push_back(hijo);
      }
    }
  }
  return {};
}

/**
 * @brief Detecta si ejecutar una acción provocaría una colisión con el Técnico.
 *
 * Consulta el sensor de agentes para comprobar si el Técnico ocupa la casilla destino del movimiento.
 *
 * @param sensores Sensores del turno actual.
 * @param accion   Acción que el Ingeniero pretende ejecutar.
 * @return true si hay riesgo de colisión, false en caso contrario.
 */
bool ComportamientoIngeniero::RiesgoChoqueTecnico(const Sensores &sensores, Action accion) const {
  switch (accion) {
  case WALK:
    return (sensores.agentes[2] == 't');
  case JUMP:
    // True si el técnico se encuentra en la casilla intermedia o en la de salto
    return (sensores.agentes[2] == 't' || sensores.agentes[6] == 't');
  default:
    return false;
  }
}

/**
 * @brief Comportamiento deliberativo del Ingeniero para el Nivel 2.
 *
 * Calcula una vez el camino óptimo (menor número de pasos) hasta la
 * filtración de Belkanita usando BFS, y lo ejecuta acción a acción.
 * Si detecta al Técnico en la trayectoria, espera con IDLE hasta que
 * se despeje.
 *
 * @param sensores Datos sensoriales del turno actual.
 * @return Acción a ejecutar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_2(Sensores sensores) {
  Action accion = IDLE;

  // 1. Si no hay plan, lo calculamos invocando nuestro método de búsqueda
  if (!hayPlan) {
    // Estado inicial (nuestra posición actual)
    EstadoI inicio, destino;
    inicio.site.f = sensores.posF;
    inicio.site.c = sensores.posC;
    inicio.site.brujula = sensores.rumbo;
    inicio.zapatillas = tiene_zapatillas;

    // Estado final (la posición de destino proporcionada por los sensores)
    destino.site.f = sensores.BelPosF;
    destino.site.c = sensores.BelPosC;
    destino.site.brujula = norte; // La orientación de llegada y las zapatillas dan igual
    destino.zapatillas = false;

    // Llamamos a nuestro algoritmo BFS
    plan = B_Anchura(inicio, destino, mapaResultado, mapaCotas);
    hayPlan = plan.size() != 0;
    VisualizaPlan(inicio.site, plan);
    // DEBUG: imprime la secuencia completa para verificar que contiene W, l y r
    cout << "[N2] Plan encontrado: "; PintaPlan(plan);
  }

  // 2. Si hay plan, lo ejecutamos paso a paso
  if (hayPlan && plan.size() > 0) {

    accion = plan.front();

    if (RiesgoChoqueTecnico(sensores, accion)) {
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
// NIVEL 3
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Comportamiento del ingeniero para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_3(Sensores sensores) {
  return IDLE;
}


// ─────────────────────────────────────────────────────────────────────────────
// NIVEL 4
// ─────────────────────────────────────────────────────────────────────────────


/**
 * @brief Comprueba si aplicar un delta de altura es válido en una casilla.
 *
 * Agua nunca admite DIG ni RAISE. RAISE requiere h < 9, DIG requiere h > 1.
 *
 * @param f      Fila de la casilla.
 * @param c      Columna de la casilla.
 * @param delta  -1 = DIG, 0 = sin cambio, 1 = RAISE.
 * @param terreno Matriz de tipos de superficie.
 * @param altura  Matriz de cotas.
 * @return true si el delta es aplicable.
 */
bool ComportamientoIngeniero::DeltaValido(int f, int c, int delta, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) const {
  unsigned char terr = terreno[f][c];
  int h = altura[f][c];

  if (terr == 'A' && delta != 0)
    return false; // Agua el agua no puede tener DIG ni RAISE
  if (delta == 1 && h >= 9)
    return false; // RAISE: solo si h < 9
  if (delta == -1 && h <= 1)
    return false; // DIG: solo si h > 1
  return true;
}

/**
 * @brief Indica si una casilla puede formar parte de la red de tuberías.
 *
 * Son inválidas: Muro 'M', Precipicio 'P', Bosque 'B' y desconocidas '?'.
 *
 * @param f      Fila de la casilla.
 * @param c      Columna de la casilla.
 * @param terreno Matriz de tipos de superficie.
 * @return true si se puede instalar tubería en esa casilla.
 */
bool ComportamientoIngeniero::CasillaValidaTuberia(int f, int c, const vector<vector<unsigned char>> &terreno) const {
  unsigned char t = terreno[f][c];

  if (f < 0 || f >= (int)terreno.size() || c < 0 || c >= (int)terreno[0].size())
    return false;
    
  if (t == 'M' || t == 'P' || t == 'B' || t == '?')
    return false;
  return true; // A, H, S, C, D, U son válidas
}

/**
 * @brief Devuelve el impacto ecológico de una acción sobre un tipo de terreno.
 *
 * Centraliza los tres costes (INSTALL, RAISE, DIG) en una sola función.
 * Valores según tabla del guión.
 *
 * @param terr   Tipo de superficie de la casilla.
 * @param accion INSTALL, RAISE o DIG.
 * @return Impacto ecológico de la acción.
 */
int ComportamientoIngeniero::getCosteEco(unsigned char sup, Action accion) const {
  switch (accion) {
    case INSTALL:
      if (sup == 'A')           return 50;
      if (sup == 'H')           return 45;
      if (sup == 'S')           return 25;
      if (sup == 'C' || sup == 'U') return 15;
      return 30;
    case RAISE:
      if (sup == 'H')           return 55;
      if (sup == 'S')           return 30;
      if (sup == 'C' || sup == 'U') return 10;
      return 40;
    case DIG:
      if (sup == 'H')           return 65;
      if (sup == 'S')           return 40;
      if (sup == 'C' || sup == 'U') return 25;
      return 50;
    default: return 0;
  }
}

/**
 * @brief Planifica la red de tuberías de mínima longitud con impacto
 *        ecológico dentro del límite dado, usando búsqueda de coste uniforme.
 *
 * Parte desde la filtración de Belkanita (fInicio, cInicio) y expande
 * hacia los 4 vecinos ortogonales. Cada nodo representa una casilla con
 * un delta de altura aplicado (-1=DIG, 0=nada, 1=RAISE). La frontera
 * ordena por longitud primero e impacto como desempate (ComparaTuberia).
 * Usa min_imp[f][c][delta] en vez de visitados bool para permitir que
 * un mismo nodo sea revisitado si llega con menor impacto.
 *
 * @param fInicio          Fila de la casilla origen (filtración Belkanita).
 * @param cInicio          Columna de la casilla origen.
 * @param terreno          Matriz de tipos de superficie.
 * @param altura           Matriz de cotas.
 * @param limiteAmbiental  Impacto ecológico máximo permitido.
 * @return Lista de Pasos que forman la red, o lista vacía si no existe solución.
 */
list<Paso> ComportamientoIngeniero::PlanificarRedTuberias(int fInicio, int cInicio, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura, int limiteAmbiental) {
  
  priority_queue<NodoTuberia, vector<NodoTuberia>, ComparaTuberia> frontera;
  
  // Matriz 3D: [f][c][delta]
  // Se usa int en vez de bool para permitir revisitar si llega con menor impacto
  vector<vector<vector<int>>> min_imp(
      terreno.size(),
      vector<vector<int>>(terreno[0].size(),
                          vector<int>(3, INT_MAX)));

  // Inicializar con los 3 deltas posibles en la Belkanita (-1,0,1), el primer impacto es solo DIG o RAISE, no suma INSTALL
  for (int delta = -1; delta <= 1; delta++) {
    if (DeltaValido(fInicio, cInicio, delta, terreno, altura)) {
      NodoTuberia inicio;
      inicio.f = fInicio;
      inicio.c = cInicio;
      inicio.delta = delta;
      inicio.longitud = 1;
      // Solo DIG/RAISE del primer punto, sin INSTALL
      int imp = 0;
      if (delta == -1)
        imp = getCosteEco(terreno[fInicio][cInicio], DIG);
      if (delta == 1)
        imp = getCosteEco(terreno[fInicio][cInicio], RAISE);
      inicio.impacto = imp;
      Paso p0{fInicio, cInicio, delta};
      inicio.camino.push_back(p0);
      frontera.push(inicio);
    }
  }

  // Bucle similar a A*
  while (!frontera.empty()) {
    NodoTuberia actual = frontera.top();
    frontera.pop(); // Extraemos el nodo más prometedor según ComparaTuberia, extrae siempre el de menor longitud y en caso de empate, el de menor impacto

    // Visitados AL EXTRAER
    int &mi = min_imp[actual.f][actual.c][actual.delta + 1]; // mi será una referencia a min_imp
    if (mi <= actual.impacto)
      continue; //ya llegamos a este nodo con menor impacto

    // Test objetivo
    if (terreno[actual.f][actual.c] == 'U') {
      if (actual.impacto <= limiteAmbiental) {
        return actual.camino; // Primera 'U' válida = óptima en longitud y dentro del límite
      }
      continue; // Supera el límite, no actualizamos min_imp, seguimos buscando
    }

    // Actualizamos mínimo solo para nodos que NO son 'U'
    mi = actual.impacto;

    int h_actual = altura[actual.f][actual.c] + actual.delta;

    // Generamos los 4 vecinos ortogonales (N,S,E,O)
    const int df[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};

    for (int dir = 0; dir < 4; dir++) {
      int nf = actual.f + df[dir];
      int nc = actual.c + dc[dir];

      if (!CasillaValidaTuberia(nf, nc, terreno))
        continue;

      // Por cada vecino probamos los 3 deltas posibles
      for (int dv = -1; dv <= 1; dv++) {
        if (!DeltaValido(nf, nc, dv, terreno, altura))
          continue;

        int h_v = altura[nf][nc] + dv;
        int diff = h_actual - h_v; // La diferencia de altura entre el nodo y su hijo no puede ser mayor de 1 ni menor a 0
        if (diff != 0 && diff != 1)
          continue;
          
        unsigned char terr_actual = terreno[actual.f][actual.c];
        unsigned char terr_vecino = terreno[nf][nc];
        
        // Calculamos la fórmula para cada conexión entre dos casillas:
        // INSTALL(casilla_anterior) + INSTALL(casilla_nueva) + DIG/RAISE(casilla_nueva si aplica)
        int imp_conexion = 0;
        imp_conexion += getCosteEco(terr_actual, INSTALL) + getCosteEco(terr_vecino, INSTALL);
        if (dv == -1)
        imp_conexion += getCosteEco(terr_vecino, DIG);
        if (dv == 1)
        imp_conexion += getCosteEco(terr_vecino, RAISE);

        NodoTuberia hijo = actual;
        hijo.f = nf;
        hijo.c = nc;
        hijo.delta = dv;
        hijo.longitud++;
        hijo.impacto += imp_conexion;
        Paso p{nf, nc, dv};
        hijo.camino.push_back(p);
        frontera.push(hijo);
      }
    }
  }

  return {};
}

/**
 * @brief Comportamiento del Ingeniero para el Nivel 4.
 *
 * Calcula la red de tuberías de mínima longitud con impacto dentro del
 * límite ambiental y la visualiza. El agente permanece en IDLE ya que
 * en este nivel no hay ejecución física, solo planificación.
 *
 * @param sensores Datos sensoriales del turno actual.
 * @return IDLE siempre.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_4(Sensores sensores) {
  if (!hayPlan) {
    plan_tuberias = PlanificarRedTuberias(
        sensores.BelPosF, sensores.BelPosC,
        mapaResultado, mapaCotas, sensores.max_ecologico);

    if (!plan_tuberias.empty()) {
      VisualizaRedTuberias(plan_tuberias);
      // cout << "Nivel 4: red de " << plan_tuberias.size() << " tramos planificada" << endl;
    }
    hayPlan = true;
  }
  return IDLE;
}


// ─────────────────────────────────────────────────────────────────────────────
// NIVEL 5
// ─────────────────────────────────────────────────────────────────────────────


/**
 * @brief Genera una ruta BFS desde la posición actual hasta un destino y la almacena en plan, actualizando hayPlan.
 *
 * @param dest_f   Fila del destino.
 * @param dest_c   Columna del destino.
 * @param sensores Sensores del turno actual.
 */
void ComportamientoIngeniero::GenerarRuta(int dest_f, int dest_c, const Sensores &sensores) {
  EstadoI inicio;
  inicio.site.f = sensores.posF;
  inicio.site.c = sensores.posC;
  inicio.site.brujula = sensores.rumbo;
  inicio.zapatillas = tiene_zapatillas;

  EstadoI destino;
  destino.site.f = dest_f;
  destino.site.c = dest_c;
  destino.site.brujula = norte; // Irrelevante para la búsqueda
  destino.zapatillas = false;

  plan = B_Anchura(inicio, destino, mapaResultado, mapaCotas);
  hayPlan = !plan.empty();
}

/**
 * @brief Extrae y devuelve la siguiente acción del plan.
 *
 * Si el Técnico está en la trayectoria, devuelve IDLE para esperar
 * sin consumir la acción del plan.
 *
 * @param sensores Sensores del turno actual.
 * @return Siguiente acción del plan, o IDLE si hay colisión o plan vacío.
 */
Action ComportamientoIngeniero::AvanzarCasilla(const Sensores &sensores) {
  if (!hayPlan || plan.empty())
    return IDLE;

  Action accion = plan.front();
  if (RiesgoChoqueTecnico(sensores, accion)) {
    return IDLE; // Pausa sin consumir acción del plan
  }

  plan.pop_front();
  if (plan.empty())
    hayPlan = false;

  return accion;
}

/**
 * @brief Aplica la operación de adecuación de terreno del tramo actual.
 *
 * Ejecuta RAISE si delta == 1, DIG si delta == -1, y marca el tramo
 * como procesado (delta = 0) para no repetir la acción.
 *
 * @param tramo_actual Referencia al paso actual del plan de tuberías.
 * @return RAISE, DIG, o IDLE si ya está adecuado.
 */
Action ComportamientoIngeniero::AdecuarTerreno(Paso &tramo_actual) {
  if (tramo_actual.op == 1) {
    tramo_actual.op = 0; // Marcar como procesado
    return RAISE;
  } else if (tramo_actual.op == -1) {
    tramo_actual.op = 0; // Marcar como procesado
    return DIG;
  }
  return IDLE;
}

/**
 * @brief Calcula el giro mínimo para orientarse hacia una casilla adyacente.
 *
 * Determina la orientación cardinal requerida y gira por el camino
 * más corto (≤4 pasos a la derecha → TURN_SR, si no → TURN_SL).
 *
 * @param dest_f   Fila de la casilla objetivo.
 * @param dest_c   Columna de la casilla objetivo.
 * @param sensores Sensores del turno actual.
 * @return TURN_SR, TURN_SL, o IDLE si ya está encarado.
 */
Action ComportamientoIngeniero::OrientarHacia(int dest_f, int dest_c, const Sensores &sensores) {
  Orientacion deseada;

  // Calcular orientación ortogonal (asumiendo adyacencia)
  if (dest_f < sensores.posF && dest_c == sensores.posC)
    deseada = norte;
  else if (dest_f > sensores.posF && dest_c == sensores.posC)
    deseada = sur;
  else if (dest_f == sensores.posF && dest_c > sensores.posC)
    deseada = este;
  else if (dest_f == sensores.posF && dest_c < sensores.posC)
    deseada = oeste;
  else
    return IDLE;

  if (sensores.rumbo == deseada)
    return IDLE;

  // Seleccionar el giro más corto
  int diff = (deseada - sensores.rumbo + 8) % 8;
  if (diff <= 4)
    return TURN_SR;
  else
    return TURN_SL;
}

/**
 * @brief Comportamiento del Ingeniero para el Nivel 5.
 *
 * Máquina de estados que coordina la construcción física de la red:
 *   Fase 0: Planificar la red de tuberías.
 *   Fase 1: Navegar hasta el nodo inicial (Belkanita).
 *   Fase 2: Adecuar el terreno del nodo actual (DIG/RAISE).
 *   Fase 3: Comprobar fin de red o invocar al Técnico (COME).
 *   Fase 4: Avanzar puntero de la lista al siguiente nodo.
 *   Fase 5: Navegar hasta el siguiente nodo.
 *   Fase 6: Adecuar el terreno del nuevo nodo.
 *   Fase 7: Orientarse hacia el nodo anterior e instalar tubería (INSTALL).
 *
 * @param sensores Datos sensoriales del turno actual.
 * @return Acción a ejecutar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores) {
  Action accion = IDLE;
  // DEBUG
  // cout << "[ING] Fase actual: " << fase
  //      << " | Pos: (" << sensores.posF << ", " << sensores.posC << ")" << endl;
  switch (fase) {
    case 0: {
      // Planificamos el camino que no lleva hacia la Belkanita
      // En caso de encontrar camino lo visualizamos
      plan_tuberias = PlanificarRedTuberias(sensores.BelPosF, sensores.BelPosC, mapaResultado, mapaCotas, sensores.max_ecologico);
      if (!plan_tuberias.empty()) {
        VisualizaRedTuberias(plan_tuberias);
        fase = 1;
      }
      break;
    }

    case 1: {
      // Navegación hacia el nodo actual (inicialmente la Belkanita)
      int dest_f = plan_tuberias.front().fil;
      int dest_c = plan_tuberias.front().col;
      
      // Si llegamos pasamos a la siguiente fase
      if (sensores.posF == dest_f && sensores.posC == dest_c) {
        fase = 2;
        return IDLE;
      }

      // Si no hay plan recalculamos
      if (!hayPlan)
        GenerarRuta(dest_f, dest_c, sensores);
      
      // Avanzamos a la siguiente acción a ejecutar
      accion = AvanzarCasilla(sensores);
      break;
    }

    case 2: {
      // Adecuación topológica del nodo actual (aplica al inicial y sucesivos)
      accion = AdecuarTerreno(plan_tuberias.front());
      if (accion == IDLE) {
        fase = 3;
      }
      break;
    }

    case 3: {
      // Condición de parada: Si no quedan más nodos, la red está finalizada
      if (plan_tuberias.size() <= 1) {
        return IDLE;
      }

      // Guardamos DONDE estamos ahora porque cuando lleguemos al siguiente nodo tendremos que orientarnos hacia AQUI para hacer el INSTALL
      prev_f = sensores.posF;
      prev_c = sensores.posC;

      fase = 4;
      return COME; // Transmite GotoF y GotoC al Técnico
    }

    case 4: {
      // Eliminamos el nodo que acabamos de procesar. Ahora front() apunta al SIGUIENTE nodo
      plan_tuberias.pop_front();
      fase = 5;
      return IDLE; // Consumo de instante para salto de fase lógico
    }

    case 5: {
      // Ahora front() es el nuevo nodo destino
      int dest_f = plan_tuberias.front().fil;
      int dest_c = plan_tuberias.front().col;

      if (sensores.posF == dest_f && sensores.posC == dest_c) {
        fase = 6; 
        return IDLE;
      }

      if (!hayPlan)
        GenerarRuta(dest_f, dest_c, sensores);
      accion = AvanzarCasilla(sensores);
      break;
    }

    case 6: {
      // Adecuación topológica del nuevo nodo
      accion = AdecuarTerreno(plan_tuberias.front());
      if (accion == IDLE) {
        fase = 7;
      }
      break;
    }

    case 7: {
      // Sincronización estricta e Instalación
      accion = OrientarHacia(prev_f, prev_c, sensores);

      if (accion == IDLE) { // Ya estamos orientados
        if (sensores.enfrente) {
          accion = INSTALL; // Ejecución simultánea
          fase = 3;         // Retorno a la fase 3 para invocar al Técnico a este nuevo nodo
        }
        // Si enfrente=false esperamos: el Técnico aún no está listo
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
 * @brief Comportamiento del Ingeniero para el Nivel 6.
 *
 * Estrategia en dos etapas:
 *   1. Exploración reactiva (delegada a Nivel_1) hasta que el mapa permita
 *      planificar una red válida desde la Belkanita hasta una 'U'.
 *   2. Una vez planificada la red, ejecuta la lógica del Nivel_5 (navegar,
 *      adecuar terreno, sincronizar con el Técnico, INSTALL).
 *
 * Optimización: PlanificarRedTuberias es costosa, así que aplicamos un
 * cooldown tras cada intento fallido para no repetirlo cada turno.
 *
 * @param sensores Datos sensoriales del turno actual.
 * @return Acción a ejecutar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_6(Sensores sensores) {
  // Fase de exploración: aún no hay red planificada
  if (!exploracion_terminada) {
    if (cooldown > 0) {
      cooldown--;
    }

    // Solo intentamos planificar si el cooldown lo permite y la Belkanita está mapeada
    if (cooldown <= 0 && mapaResultado[sensores.BelPosF][sensores.BelPosC] != '?') {
      plan_tuberias = PlanificarRedTuberias(sensores.BelPosF, sensores.BelPosC, mapaResultado, mapaCotas, sensores.max_ecologico);

      if (!plan_tuberias.empty()) {
        VisualizaRedTuberias(plan_tuberias);
        exploracion_terminada = true;
        fase = 1;    // Iniciar máquina de estados de construcción (Nivel 5)
        return IDLE; // Saltamos la fase 0 de planificación del Nivel 5
      } else {
        // Penalización por fallo: El mapa no está lo suficientemente descubierto.
        // Evita repetir el cálculo pesado en los próximos 30 instantes.
        cooldown = 30;
      }
    }
    return ComportamientoIngenieroNivel_1(sensores);
  }
  return ComportamientoIngenieroNivel_5(sensores);
}

// =========================================================================
// FUNCIONES PROPORCIONADAS
// =========================================================================

/**
 * @brief Actualiza el mapaResultado y mapaCotas con la información de los sensores.
 * @param sensores Datos actuales de los sensores.
 */
void ComportamientoIngeniero::ActualizarMapa(Sensores sensores) {
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
 * @brief Determina si una casilla es transitable para el ingeniero.
 * @param f Fila de la casilla.
 * @param c Columna de la casilla.
 * @param tieneZapatillas Indica si el agente posee las zapatillas.
 * @return true si la casilla es transitable (no es muro ni precipicio).
 */
bool ComportamientoIngeniero::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas) {
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size())
    return false;
  return es_camino(mapaResultado[f][c]); // Solo 'C', 'D', 'U' son transitables en Nivel 0
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el ingeniero: desnivel máximo 1 sin zapatillas, 2 con zapatillas.
 * @param actual Estado actual del agente (fila, columna, orientacion, zap).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoIngeniero::EsAccesiblePorAltura(const ubicacion &actual, bool zap) {
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size())
    return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (zap && desnivel > 2)
    return false;
  if (!zap && desnivel > 1)
    return false;
  return true;
}

/**
 * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal según la orientación actual (8 direcciones).
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return Estado con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoIngeniero::Delante(const ubicacion &actual) const {
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
void ComportamientoIngeniero::PintaPlan(const list<Action> &plan) {
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
 * @brief Imprime las coordenadas y operaciones de un plan de tubería.
 *
 * @param plan  Lista de pasos (fila, columna, operación),
 *              donde operacion = -1 (DIG), operación = 1 (RAISE).
 */
void ComportamientoIngeniero::PintaPlan(const list<Paso> &plan) {
  auto it = plan.begin();
  while (it != plan.end()) {
    cout << it->fil << ", " << it->col << " (" << it->op << ")\n";
    it++;
  }
  cout << "( longitud " << plan.size() << ")" << endl;
}

/**
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa 2D.
 *
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoIngeniero::VisualizaPlan(const ubicacion &st,
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

/**
 * @brief Convierte un plan de tubería en la lista de casillas usada
 *        por el sistema de visualización.
 *
 * @param st    Estado de partida (no utilizado directamente).
 * @param plan  Lista de pasos del plan de tubería.
 */
void ComportamientoIngeniero::VisualizaRedTuberias(const list<Paso> &plan) {
  listaCanalizacionTuberias.clear();
  auto it = plan.begin();
  while (it != plan.end()) {
    listaCanalizacionTuberias.push_back({it->fil, it->col, it->op});
    it++;
  }
}
