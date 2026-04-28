#include "tecnico.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>

using namespace std;

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoTecnico::think(Sensores sensores)
{
  Action accion = IDLE;

  // Decisión del agente según el nivel
  switch (sensores.nivel) {
    case 0: accion = ComportamientoTecnicoNivel_0(sensores); break;
    case 1: accion = ComportamientoTecnicoNivel_1(sensores); break;
    case 2: accion = ComportamientoTecnicoNivel_E(sensores); break;
   // case 3: accion = ComportamientoTecnicoNivel_3(sensores); break;
    case 3: accion = ComportamientoTecnicoNivel_E(sensores); break;
    case 4: accion = ComportamientoTecnicoNivel_4(sensores); break;
    case 5: accion = ComportamientoTecnicoNivel_5(sensores); break;
    case 6: accion = ComportamientoTecnicoNivel_6(sensores); break;
  }

  return accion;
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
char ViablePorAltura0T (char casilla, int dif) {
  if (abs(dif) <= 1){
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
int VeoCasillaPulgarcitoNivel0T (char i, char c, char d, int t_i, int t_c, int t_d) {
    // Prioridad a la Meta
    if (c == 'U') return 2;
    else if (i == 'U') return 1;
    else if (d == 'U') return 3;

    // Descarto todo lo que no es camino poniendo un tiempo muy alto
    if (i != 'C') t_i = 999999;
    if (c != 'C') t_c = 999999;
    if (d != 'C') t_d = 999999;

    // Si no hay ningún camino viable delante, devolvemos 0 para que gire a buscar uno usando el default del switch
    if (t_i == 999999 && t_c == 999999 && t_d == 999999) return 0;

    // Compruebo cual es el menor de los 3 tiempos y el que lo sea decidirá cual acción se deberá hacer
    int min_tiempo = min(t_i, min(t_c, t_d));

    if (min_tiempo == t_c) return 2; // WALK
    if (min_tiempo == t_i) return 1; // TURN_SL
    if (min_tiempo == t_d) return 3; // TURN_SR

    return 0;
}

// Niveles iniciales (Comportamientos reactivos simples)
Action ComportamientoTecnico::ComportamientoTecnicoNivel_0(Sensores sensores)
{
  Action accion = IDLE;
  
    // Poner el valor de los sensores de visión sobre los mapas
  ActualizarMapa(sensores);
  
  // Primero comprobamos si la casilla en la que nos situamos es la de las Zapatillas ('D') actualizando su variable de estado
  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  // Comprobamos si la casilla en la que nos situamos es la Planta de Residuos ('U'), en cuyo caso habríamos terminado
  if(sensores.superficie[0]=='U')  return IDLE;

  // Extraemos el valor de las casillas centro, 45º izquierda, 45º derecha
  // El valor del char será la superficie de la casilla si se puede acceder a ella y Precipicio'P' en caso contrario
  char i = ViablePorAltura0T (sensores.superficie[1], sensores.cota[1]-sensores.cota[0]); 
  char c = ViablePorAltura0T (sensores.superficie[2], sensores.cota[2]-sensores.cota[0]);
  char d = ViablePorAltura0T (sensores.superficie[3], sensores.cota[3]-sensores.cota[0]);
  
  // Descartamos aquellos movimientos que hagan colisión con el técnico comprobando en la matriz de agentes si las casillas están desocupadas o no
  if (sensores.agentes[1] != '_') i = 'P';
  if (sensores.agentes[2] != '_') c = 'P';
  if (sensores.agentes[3] != '_') d = 'P';

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
  int t_i = (posI.f >= 0 && posI.f < mtiempo.size() && posI.c >= 0 && posI.c < mtiempo[0].size()) ? mtiempo[posI.f][posI.c] : 999999;
  int t_c = (posC.f >= 0 && posC.f < mtiempo.size() && posC.c >= 0 && posC.c < mtiempo[0].size()) ? mtiempo[posC.f][posC.c] : 999999;
  int t_d = (posD.f >= 0 && posD.f < mtiempo.size() && posD.c >= 0 && posD.c < mtiempo[0].size()) ? mtiempo[posD.f][posD.c] : 999999;

  // Tras obtener las superficies i,c,d los tiempos almacenados en esas casillas t_i,t_c,t_d y comprobar que no ha habido choques, planteamos que hacer en la función VeoCasillaPulgarcitoNivel0I
  int pos = VeoCasillaPulgarcitoNivel0T(i, c, d, t_i, t_c, t_d);

  switch (pos)
  {
  case 2:
  accion = WALK;
  break;

  case 1:
  accion = TURN_SL;
  break;
  
  case 3:
  accion = TURN_SR;
  break;
  
  default:
  accion = TURN_SL;
  break;
  }

  // Incrementamos la variable instante que será la que vaya marcando el recorrido del personaje y sobreescribimos en la matriz de tiempos el nuevo tiempo de dicha posición
  instante++;
  mtiempo[sensores.posF][sensores.posC]=instante;
  
  // Devolvemos la acción decidida
  last_action=accion;
  return accion;
}

/**
 * @brief Comprueba si una celda es de tipo camino transitable.
 * @param c Carácter que representa el tipo de superficie.
 * @return true si es camino ('C'), zapatillas ('D') o meta ('U').
 */
bool ComportamientoTecnico::es_camino(unsigned char c) const {
  return (c == 'C' || c == 'D' || c == 'U');
}

/**
 * @brief Viabilidad para Nivel 1. Muros y Agua bloquean. Resto pasa si la altura lo permite.
 */
char ViablePorAltura1T (char casilla, int dif, bool zap) {
  // En el nivel 1, Muro y Agua son precipicios
  if (casilla == 'M' || casilla == 'A' || casilla == 'P') return 'P'; 
  
  if (casilla == 'B' && !zap) return 'P';

  if (abs(dif) <= 1){
    return casilla;
  } else {
    return 'P';
  }
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
int VeoCasillaPulgarcitoNivel1T (char i, char c, char d, int t_i, int t_c, int t_d, bool zap) {
  // Si lo que ve es un Precipicio lo descartamos y le damos un tiempo infinito
  if (c == 'P') t_c = 999999;
  if (i == 'P') t_i = 999999;
  if (d == 'P') t_d = 999999;

  if (t_i == 999999 && t_c == 999999 && t_d == 999999) return 0;

  // Vemos el camino con menor tiempo que será el más antiguo o nunca visitado
  int min_tiempo = min(t_i, min(t_c, t_d));

  // Prioridad a las zapatillas
  if (!zap) {
      if (c == 'D') return 2;
      if (i == 'D') return 1;
      if (d == 'D') return 3;
  }

  // Prioridad a Caminos o Senderos ('C' o 'S') que NO hayamos visitado
  if (min_tiempo == 0) {
      if ((c == 'C' || c == 'S') && t_c == 0) return 2;
      if ((i == 'C' || i == 'S') && t_i == 0) return 1;
      if ((d == 'C' || d == 'S') && t_d == 0) return 3;
  }

  if (min_tiempo == t_c) return 2; // WALK
  if (min_tiempo == t_i) return 1; // TURN_SL
  if (min_tiempo == t_d) return 3; // TURN_SR

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

  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  char i = ViablePorAltura1T (sensores.superficie[1], sensores.cota[1]-sensores.cota[0], tiene_zapatillas); // Si la altura es <= 1 o tiene zapatillas y es un bosque con altura <= 1 es transitable
  char c = ViablePorAltura1T (sensores.superficie[2], sensores.cota[2]-sensores.cota[0], tiene_zapatillas);
  char d = ViablePorAltura1T (sensores.superficie[3], sensores.cota[3]-sensores.cota[0], tiene_zapatillas);

  if (sensores.agentes[1] != '_') i = 'P';
  if (sensores.agentes[2] != '_') c = 'P'; 
  if (sensores.agentes[3] != '_') d = 'P';
  
  ubicacion posActual = {sensores.posF, sensores.posC, sensores.rumbo};
  ubicacion posC = Delante(posActual);
  ubicacion posI = posActual;
  posI.brujula = (Orientacion)((posI.brujula + 7) % 8);
  posI = Delante(posI);  
  ubicacion posD = posActual; 
  posD.brujula = (Orientacion)((posD.brujula + 1) % 8);
  posD = Delante(posD);

  int t_i = (posI.f >= 0 && posI.f < mtiempo.size() && posI.c >= 0 && posI.c < mtiempo[0].size()) ? mtiempo[posI.f][posI.c] : 999999;
  int t_c = (posC.f >= 0 && posC.f < mtiempo.size() && posC.c >= 0 && posC.c < mtiempo[0].size()) ? mtiempo[posC.f][posC.c] : 999999;
  int t_d = (posD.f >= 0 && posD.f < mtiempo.size() && posD.c >= 0 && posD.c < mtiempo[0].size()) ? mtiempo[posD.f][posD.c] : 999999;

  int pos = VeoCasillaPulgarcitoNivel1T(i, c, d, t_i, t_c, t_d, tiene_zapatillas);

  switch (pos)
  {
  case 2:
  accion = WALK;
  break;

  case 1:
  accion = TURN_SL;
  break;
  
  case 3:
  accion = TURN_SR;
  break;
  
  default:
  accion = TURN_SL;
  break;
  }

  instante++;
  mtiempo[sensores.posF][sensores.posC]=instante;
  
  last_action=accion;
  return accion;
}



// Parte 2

/**
 * @brief Toma de entrada un estado y devuelve el estado en el que quedaría el agente tras hacer la acción
 */
EstadoT ComportamientoTecnico::NextCasillaTecnico(const EstadoT &st){
  EstadoT siguiente = st;
  switch (st.site.brujula)
  {
  case norte:
    siguiente.site.f = st.site.f - 1;
    break;
  case noreste:
    siguiente.site.f = st.site.f - 1;
    siguiente.site.c = st.site.c + 1;
    break;
  case este:
    siguiente.site.c = st.site.c + 1;
    break;
  case sureste:
    siguiente.site.f = st.site.f + 1;
    siguiente.site.c = st.site.c + 1;
    break;
  case sur:
    siguiente.site.f = st.site.f + 1;
    break;
  case suroeste:
    siguiente.site.f = st.site.f + 1;
    siguiente.site.c = st.site.c - 1;
    break;
  case oeste:
    siguiente.site.c = st.site.c - 1;
  break;
    case noroeste:
    siguiente.site.f = st.site.f - 1;
    siguiente.site.c = st.site.c - 1;
  }
  return siguiente;
}

/**
 * @brief Toma el EstadoT st, la matriz de terreno y la de altura y devuelve si sería posible que el Técnico hiciese un WALK a esa posición.
 */
bool ComportamientoTecnico::CasillaAccesibleTécnico(const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  EstadoT next = NextCasillaTecnico(st);
  bool check1 = false, check2 = false, check3 = false;
  check1 = terreno[next.site.f][next.site.c] != 'P' and terreno[next.site.f][next.site.c] != 'M';
  check2 = terreno[next.site.f][next.site.c] != 'B' or (terreno[next.site.f][next.site.c] == 'B' and st.zapatillas);
  check3 = abs(altura[next.site.f][next.site.c] - altura[st.site.f][st.site.c]) <= 1;
  return check1 and check2 and check3;
}

/**
 * @brief Recibe una serie de parámetros y devuelve el estado resultante tras aplicarla
 * 
 * @param accion acción que quiere realizar
 * @param st estado actual del agente
 * @param terreno 
 * @param altura 
 * @return EstadoT resultante tras la accion
 */
EstadoT ComportamientoTecnico::applyT(Action accion, const EstadoT & st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  EstadoT next = st;
  switch(accion){
    case WALK:
      if (CasillaAccesibleTécnico(st,terreno,altura)){
        next = NextCasillaTecnico(st);
      }
    break;
    case TURN_SR:
      next.site.brujula = (Orientacion) ((next.site.brujula+1)%8);
    break;
      case TURN_SL:
      next.site.brujula = (Orientacion) ((next.site.brujula+7)%8);
    break;
  }
  return next;
}

/**
 * @brief Obtiene un estado y una lista de estados y dice si este está en la lista o no
 * 
 * @param st 
 * @param lista 
 * @return true 
 * @return false 
 */
bool Find (const NodoT & st, const list<NodoT> &lista){
  auto it = lista.begin();
  while (it != lista.end() and !((*it) == st)){
    it++;
  }
  return (it != lista.end());
}

/**
* @param inicio Estado Inicial de la búsqueda.
* @param final Estado Final de la búsqueda.
* @param terreno Matriz que contiene la información del terreno.
* @param altura Matriz que contiene la altura del mapa.
* @return La secuencia de acciones para llegar al estado final.
* @note Devuelve un plan vacío si no es posible encontrar un plan válido.
*/
list<Action> ComportamientoTecnico::B_Anchura ( const EstadoT &inicio, const EstadoT &final,
                                                 const vector<vector<unsigned char>> &terreno,
                                                 const vector<vector<unsigned char>> &altura) {
  NodoT current_node;
  list<NodoT> frontier;
  set<EstadoT> explored; // Corrección: Se almacena el Estado, no el Nodo, para evitar sobrecarga de CPU y RAM
  list<Action> path;
  
  current_node.estado = inicio;
  frontier.push_back(current_node);

  bool SolutionFound = (current_node.estado.site.f == final.site.f and current_node.estado.site.c == final.site.c);

  while (!SolutionFound and !frontier.empty()){
    // 1. Extracción segura al inicio del bucle
    current_node = frontier.front();
    frontier.pop_front();

    // 2. Control de nodos visitados optimizado (O(log N))
    if (explored.find(current_node.estado) != explored.end()) {
        continue; // Si el estado ya está explorado, saltamos al siguiente nodo de la frontera
    }
    explored.insert(current_node.estado); // Marcamos el estado como explorado

    // Compruebo si estoy en una casilla que da las zapatillas
    if (terreno[current_node.estado.site.f][current_node.estado.site.c] == 'D'){
      current_node.estado.zapatillas = true;
    }

    // --- Expansión de WALK ---
    NodoT child_Walk = current_node;
    child_Walk.estado = applyT(WALK, current_node.estado, terreno, altura);
    
    // Corrección: Solo evaluamos si el movimiento WALK ha provocado un cambio real (no es un muro)
    if (!(child_Walk.estado == current_node.estado)) {
        if (child_Walk.estado.site.f == final.site.f and child_Walk.estado.site.c == final.site.c){
          child_Walk.secuencia.push_back(WALK);
          current_node = child_Walk;
          SolutionFound = true;
        }
        else {
          child_Walk.secuencia.push_back(WALK);
          frontier.push_back(child_Walk);
        }
    }

    // --- Expansión de Giros ---
    if (!SolutionFound) {
      // TURN_SR
      NodoT child_TurnSR = current_node;
      child_TurnSR.estado = applyT(TURN_SR, current_node.estado, terreno, altura);
      child_TurnSR.secuencia.push_back(TURN_SR);
      frontier.push_back(child_TurnSR); // Se añade incondicionalmente a la frontera; el control de repetidos se hace al extraer (arriba)
    
      // TURN_SL
      NodoT child_TurnSL = current_node;
      child_TurnSL.estado = applyT(TURN_SL, current_node.estado, terreno, altura);
      child_TurnSL.secuencia.push_back(TURN_SL);
      frontier.push_back(child_TurnSL);
    }
  }

  // Asignación final a la variable path solicitada
  if (SolutionFound) {
      path = current_node.secuencia;
  }
  
  return path;
}

Action ComportamientoTecnico::ComportamientoTecnicoNivel_E(Sensores sensores){
  Action accion = IDLE;

  // 1. Si no hay plan, lo calculamos invocando nuestro método de búsqueda
  if (!hayPlan) {
      // Estado inicial (nuestra posición actual)
      EstadoT inicio, destino;
      inicio.site.f = sensores.posF;
      inicio.site.c = sensores.posC;
      inicio.site.brujula = sensores.rumbo;
      inicio.zapatillas = tiene_zapatillas;

      // Estado final (la posición de destino proporcionada por los sensores)
      destino.site.f = sensores.BelPosF;
      destino.site.c = sensores.BelPosC;
      destino.site.brujula = norte; // La orientación de llegada da igual
      destino.zapatillas = false;   // Da igual con qué zapatillas lleguemos

      // Llamamos a nuestro algoritmo BFS
      plan = B_Anchura(inicio, destino, mapaResultado, mapaCotas);
      VisualizaPlan(inicio.site,plan);
      hayPlan = plan.size()!=0;
  }
  // 2. Si hay plan, lo ejecutamos paso a paso
  if (hayPlan && plan.size() > 0) {
      accion = plan.front(); // Cogemos la primera acción
      plan.pop_front();      // La borramos de la lista
  }

  // Si nos quedamos sin plan (hemos llegado al final), reseteamos para futuros viajes
  if (plan.size() == 0) {
      hayPlan = false;
  }

  return accion;
}

/**
 * @brief Comportamiento del técnico para el Nivel 2.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_2(Sensores sensores) {
  return IDLE;
}

/**
 * @brief Comportamiento del técnico para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_3(Sensores sensores) {
  return IDLE;
}

/**
 * @brief Comportamiento del técnico para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_4(Sensores sensores) {
  return IDLE;
}

/**
 * @brief Comportamiento del técnico para el Nivel 5.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_5(Sensores sensores) {
  return IDLE;
}

/**
 * @brief Comportamiento del técnico para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_6(Sensores sensores) {
  return IDLE;
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
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size()) return false;
  return es_camino(mapaResultado[f][c]);  // Solo 'C', 'S', 'D', 'U' son transitables en Nivel 0
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el técnico: desnivel máximo siempre 1.
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoTecnico::EsAccesiblePorAltura(const ubicacion &actual) {
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size()) return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (desnivel > 1) return false;
  return true;
}

/**
 * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal según la orientación actual (8 direcciones).
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return Estado con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoTecnico::Delante(const ubicacion &actual) const {
  ubicacion delante = actual;
  switch (actual.brujula) {
    case 0: delante.f--; break;                        // norte
    case 1: delante.f--; delante.c++; break;     // noreste
    case 2: delante.c++; break;                     // este
    case 3: delante.f++; delante.c++; break;     // sureste
    case 4: delante.f++; break;                        // sur
    case 5: delante.f++; delante.c--; break;     // suroeste
    case 6: delante.c--; break;                     // oeste
    case 7: delante.f--; delante.c--; break;     // noroeste
  }
  return delante;
}


/**
 * @brief Imprime por consola la secuencia de acciones de un plan.
 *
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::PintaPlan(const list<Action> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    if (*it == WALK)
    {
      cout << "W ";
    }
    else if (*it == JUMP)
    {
      cout << "J ";
    }
    else if (*it == TURN_SR)
    {
      cout << "r ";
    }
    else if (*it == TURN_SL)
    {
      cout << "l ";
    }
    else if (*it == COME)
    {
      cout << "C ";
    }
    else if (*it == IDLE)
    {
      cout << "I ";
    }
    else
    {
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
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::VisualizaPlan(const ubicacion &st,
                                            const list<Action> &plan)
{
   listaPlanCasillas.clear();
  ubicacion cst = st;

  listaPlanCasillas.push_back({cst.f, cst.c, WALK});
  auto it = plan.begin();
  while (it != plan.end())
  {

    switch (*it)
    {
    case JUMP:
      switch (cst.brujula)
      {
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
      switch (cst.brujula)
      {
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
      cst.brujula = (Orientacion) (( (int) cst.brujula + 1) % 8);
      break;
    case TURN_SL:
      cst.brujula = (Orientacion) (( (int) cst.brujula + 7) % 8);
      break;
    }
    it++;
  }
}


