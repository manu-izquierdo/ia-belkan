#include "ingeniero.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>

using namespace std;

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoIngeniero::think(Sensores sensores)
{
  Action accion = IDLE;

  // Decisión del agente según el nivel
  switch (sensores.nivel) {
    case 0: accion = ComportamientoIngenieroNivel_0(sensores); break;
    case 1: accion = ComportamientoIngenieroNivel_1(sensores); break;
    case 2: accion = ComportamientoIngenieroNivel_2(sensores); break;
    case 3: accion = ComportamientoIngenieroNivel_3(sensores); break;
    case 4: accion = ComportamientoIngenieroNivel_4(sensores); break;
    case 5: accion = ComportamientoIngenieroNivel_5(sensores); break;
    case 6: accion = ComportamientoIngenieroNivel_6(sensores); break;
  }

  return accion;
}


/**
* @brief Determina si casilla viable por altura.
* @param casilla tipo de terreno
* @param dif diferencia de altura entre casillas
* @param zap indica si estoy en posesión de las zapatillas
* @return 'P' si no es accesible por altura y casilla en otro caso
* @note Recibe el tipo de terreno, la dif entre casillas si tiene las 
* zapatillas y comprueba si es viable por altura.
* DIFERENCIA: He usado esta función en vez de EsAccesiblePorAltura puesto que de esta manera,
* en la función VeoCasillaPulgarcitoNivel0I puedo establecer prioridad dependiendo del tipo de casilla que devuelve.
*/
char ViablePorAltura0I (char casilla, int dif, bool zap) {
  if (abs(dif) <= 1 or (zap and abs(dif) <=2)){
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
 * @param zap indica si estoy en posesión de las zapatillas
 * @return 2 si es mejor WALK, 1 para TURN_SL y 3 para TURN_SR. O no hay nada interesante.
 */
int VeoCasillaPulgarcitoNivel0I (char i, char c, char d, int t_i, int t_c, int t_d, bool zap) {
    // Prioridad a la Meta y Zapatillas
    if (c == 'U') return 2;
    else if (i == 'U') return 1;
    else if (d == 'U') return 3;

    if (!zap) {
        if (c == 'D') return 2;
        else if (i == 'D') return 1;
        else if (d == 'D') return 3;
    }

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
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_0(Sensores sensores)
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
  char i = ViablePorAltura0I (sensores.superficie[1], sensores.cota[1]-sensores.cota[0], tiene_zapatillas); 
  char c = ViablePorAltura0I (sensores.superficie[2], sensores.cota[2]-sensores.cota[0], tiene_zapatillas);
  char d = ViablePorAltura0I (sensores.superficie[3], sensores.cota[3]-sensores.cota[0], tiene_zapatillas);
  
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
  int pos = VeoCasillaPulgarcitoNivel0I(i, c, d, t_i, t_c, t_d, tiene_zapatillas);

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
bool ComportamientoIngeniero::es_camino(unsigned char c) const
{
  return (c == 'C' || c == 'D' || c == 'U');
}

/**
 * @brief Viabilidad para Nivel 1. Muros y Agua bloquean. Resto pasa si la altura lo permite.
 */
char ViablePorAltura1I (char casilla, int dif, bool zap) {
  // En el nivel 1, Muro y Agua son precipicios
  if (casilla == 'M' || casilla == 'A' || casilla == 'P' || casilla == 'B') return 'P'; 
  
  if (abs(dif) <= 1 || (zap && abs(dif) <= 2)){
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
int VeoCasillaPulgarcitoNivel1I (char i, char c, char d, int t_i, int t_c, int t_d, bool zap) {
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
 * @brief Comportamiento reactivo del ingeniero para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_1(Sensores sensores) // He aplicado la misma lógica que había en el nivel 0, por lo que he eliminado comentarios
{
  Action accion = IDLE;

  ActualizarMapa(sensores);

  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  char i = ViablePorAltura1I (sensores.superficie[1], sensores.cota[1]-sensores.cota[0], tiene_zapatillas); // Si la altura es <= 1 o tiene zapatillas y es <= 2 es transitable
  char c = ViablePorAltura1I (sensores.superficie[2], sensores.cota[2]-sensores.cota[0], tiene_zapatillas);
  char d = ViablePorAltura1I (sensores.superficie[3], sensores.cota[3]-sensores.cota[0], tiene_zapatillas);

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

  int pos = VeoCasillaPulgarcitoNivel1I(i, c, d, t_i, t_c, t_d, tiene_zapatillas);

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

bool ComportamientoIngeniero::esSuperficieValida(unsigned char superficie) const {
  if (superficie == 'M' || superficie == 'P' || superficie == 'B') {
    return false;
  } else {
    return true;
  }
}


/**
 * @brief Toma el EstadoI st, la matriz de terreno y la de altura y devuelve si sería posible que el Ingeniero hiciese un WALK o JUMP a esa posición.
 */
bool ComportamientoIngeniero::CasillaAccesibleIngeniero(Action accion, const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  // Si no avanzamos, la acción es válida
  if (accion!=WALK && accion!=JUMP) return true;

  ubicacion casilla_delante = Delante(st.site);

  // Ingeniero tiene como casillas intransitables Muro 'M', Precipicio 'P' y Bosque 'B'
  if (accion == WALK) {
    if (casilla_delante.f < 0 || casilla_delante.f >= terreno.size() || casilla_delante.c < 0 || casilla_delante.c >= terreno[0].size()) return false;
    if (!esSuperficieValida(terreno[casilla_delante.f][casilla_delante.c])) return false;

    int dif_altura = altura[casilla_delante.f][casilla_delante.c] - altura[st.site.f][st.site.c];
    return (abs(dif_altura) <= 1 || (st.zapatillas && abs(dif_altura) <= 2));
  }

  else if (accion == JUMP) {
    ubicacion casilla_jump = Delante(casilla_delante);

    // Comprobación casilla intermedia
    if (casilla_delante.f < 0 || casilla_delante.f >= terreno.size() || casilla_delante.c < 0 || casilla_delante.c >= terreno[0].size()) return false;
    if (!esSuperficieValida(terreno[casilla_delante.f][casilla_delante.c])) return false;

    // Comprobación casilla destino
    if (casilla_jump.f < 0 || casilla_jump.f >= terreno.size() || casilla_jump.c < 0 || casilla_jump.c >= terreno[0].size()) return false;
    if (!esSuperficieValida(terreno[casilla_jump.f][casilla_jump.c])) return false;
    
    int dif_altura = altura[casilla_jump.f][casilla_jump.c] - altura[st.site.f][st.site.c];
    return (abs(dif_altura) <= 1 || (st.zapatillas && abs(dif_altura) <= 2));
  }
  return false;
}


/**
 * @brief Recibe una serie de parámetros y devuelve el estado resultante tras aplicarlos
 * 
 * @param accion acción que quiere realizar
 * @param st estado actual del agente
 * @param terreno 
 * @param altura 
 * @return EstadoI resultante tras la accion
 */
EstadoI ComportamientoIngeniero::applyI(Action accion, const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  EstadoI nuevo_st = st;
  switch(accion){
    case WALK:
      nuevo_st.site = Delante(st.site);
    break;
    case JUMP:
      nuevo_st.site = Delante(Delante(st.site));
    break;
    case TURN_SR:
      nuevo_st.site.brujula = (Orientacion) ((nuevo_st.site.brujula+1)%8);
    break;
      case TURN_SL:
      nuevo_st.site.brujula = (Orientacion) ((nuevo_st.site.brujula+7)%8);
    break;
  }
  // Adquisición de zapatillas en el nuevo estado
  if (terreno[nuevo_st.site.f][nuevo_st.site.c] == 'D') nuevo_st.zapatillas = true;
  return nuevo_st;
}


/**
* @brief Va almacenando los nodos de menor a mayor distancia y va buscando la solucion, cada nodo generará tantos nodos como acciones tenga válidas desde su posición, comprobando que son transitables y se pueden ejecutar
* @param inicio Estado Inicial de la búsqueda.
* @param final Estado Final de la búsqueda.
* @param terreno Matriz que contiene la información del terreno.
* @param altura Matriz que contiene la altura del mapa.
* @return La secuencia de acciones para llegar al estado final.
* @note Devuelve un plan vacío si no es posible encontrar un plan válido.
*/
list<Action> ComportamientoIngeniero::B_Anchura(const EstadoI &inicio, const EstadoI &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) {
  NodoI nodo_actual;
  list<NodoI> frontier; // Actuará como si fuese una cola FIFO, nodos abiertos pero no explorados
  list<Action> camino_solucion;

  // Matriz 4D para visitados: [Fila][Columna][Orientacion][Zapatillas]
  // Evito el uso de set y reduzco la complejidad de O(log N) a O(1)
  vector<vector<vector<vector<bool>>>> visitados(terreno.size(),
      vector<vector<vector<bool>>>(terreno[0].size(),
      vector<vector<bool>>(8,
      vector<bool>(2, false))));
  
  nodo_actual.estado = inicio;
  frontier.push_back(nodo_actual); //Insertamos el Nodo Raíz en la frontera (distancia 0)

  // Si la fila y columna del nodo inicio y destino coinciden ya se ha llegado a la Belkanita
  bool solucion_encontrada = (nodo_actual.estado.site.f == final.site.f and nodo_actual.estado.site.c == final.site.c);

  while (!solucion_encontrada and !frontier.empty()){
    // 1. Extraemos los nodos usando FIFO para que haga primero los de distancias menores y luego mayores
    nodo_actual = frontier.front();
    frontier.pop_front();

    // 2. Control de nodos visitados optimizado (O(log N))
    int zapatillas = nodo_actual.estado.zapatillas ? 1 : 0;
    if (visitados[nodo_actual.estado.site.f][nodo_actual.estado.site.c][nodo_actual.estado.site.brujula][zapatillas]) continue; //Si ya ha sido visitado, se continua
    visitados[nodo_actual.estado.site.f][nodo_actual.estado.site.c][nodo_actual.estado.site.brujula][zapatillas] = true; // Marcamos el estado como explorado

    // Si llegmaos a la solucion sale del while y devuelve camino_solucion
    if (nodo_actual.estado.site.f == final.site.f && nodo_actual.estado.site.c == final.site.c) {
      solucion_encontrada = true;
      camino_solucion = nodo_actual.secuencia;
    break;
    }

    //Añadimos a la cola una copia de lo que ya llevaba mas lo que pasaría si hace cada una de sus acciones
    Action accionesPosibles[] = {WALK, JUMP, TURN_SR, TURN_SL};
    for (Action accion : accionesPosibles) {
      if (CasillaAccesibleIngeniero(accion, nodo_actual.estado, terreno, altura)) {
        NodoI hijo = nodo_actual;
        hijo.estado = applyI(accion, nodo_actual.estado, terreno, altura);
        hijo.secuencia.push_back(accion);
        frontier.push_back(hijo);
      }
    }
  }
  return camino_solucion;
}


/**
 * @brief Determina si la ejecución de una acción conlleva un riesgo de colisión con el técnico
 * 
 * @param sensores Referencia constante al objeto de sensores actual.
 * @param accion Acción que el agente pretende realizar.
 * @return true si hay riesgo de choque, false en caso contrario.
 */
bool ComportamientoIngeniero::RiesgoChoqueTecnico(const Sensores &sensores, Action accion) {
    switch (accion) {
        case WALK:
            // True si el técnico se encuentra justo en la casilla de delante
            return (sensores.agentes[2] == 't');
        case JUMP:
            // True si el técnico se encuentra en la casilla intermedia o en la de salto
            return (sensores.agentes[2] == 't' || sensores.agentes[6] == 't');
        default:
            // Para el resto de acciones no hay riesgo de choque
            return false;
    }
}


// Niveles avanzados (Uso de búsqueda)
/**
 * @brief Comportamiento del ingeniero para el Nivel 2 (búsqueda).
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_2(Sensores sensores)
{
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
      VisualizaPlan(inicio.site,plan);
      hayPlan = plan.size()!=0;
  }

  // 2. Si hay plan, lo ejecutamos paso a paso
  if (hayPlan && plan.size() > 0) {
    
    accion = plan.front();

    if(RiesgoChoqueTecnico(sensores, accion)){
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

/**
 * @brief Comportamiento del ingeniero para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_3(Sensores sensores)
{
  return IDLE;
}

/**
 * @brief Comprueba si el cambio de altura es o no es válido en esa casilla
 * 
 * @param f fila
 * @param c columna
 * @param delta movimiento a hacer (-1 = DIG, 1 = RAISE)
 */
bool ComportamientoIngeniero::DeltaValido(int f, int c, int delta, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  unsigned char terr = terreno[f][c];
  int h = altura[f][c];
  
  if (terr == 'A' && delta != 0) return false;  // Agua el agua no puede tener DIG ni RAISE
  if (delta == 1 && h >= 9)     return false;   // RAISE: solo si h < 9
  if (delta == -1 && h <= 1)    return false;   // DIG: solo si h > 1
  return true;
}

/**
 * @brief Comprueba si la casilla permite instalar un tramo de tubería
 */
bool ComportamientoIngeniero::CasillaValidaTuberia(int f, int c, const vector<vector<unsigned char>> &terreno) {
  unsigned char t = terreno[f][c];
  if (t == 'M' || t == 'P' || t == 'B' || t == '?') return false;
  return true;  // A, H, S, C, D, U son válidas
}

/**
 * @brief Impacto Ecológico de instalar una tubería en ese tipo de terreno
 */
int ComportamientoIngeniero::getCosteEcoINSTALL(unsigned char terr) {
  if (terr == 'A') return 50;
  if (terr == 'H') return 45;
  if (terr == 'S') return 25;
  if (terr == 'C' || terr == 'U') return 15;
  return 30;
}

/**
 * @brief Impacto Ecológico de subir una casilla en ese tipo de terreno
 */
int ComportamientoIngeniero::getCosteEcoRAISE(unsigned char terr) {
  if (terr == 'H') return 55;
  if (terr == 'S') return 30;
  if (terr == 'C' || terr == 'U') return 10;
  return 40;
}

/**
 * @brief Impacto Ecológico de bajar una casilla en ese tipo de terreno
 */
int ComportamientoIngeniero::getCosteEcoDIG(unsigned char terr) {
  if (terr == 'H') return 65;
  if (terr == 'S') return 40;
  if (terr == 'C' || terr == 'U') return 25;
  return 50;
}

/**
 * @brief 
 * 
 * @param fInicio 
 * @param cInicio 
 * @param terreno 
 * @param altura 
 * @param limiteAmbiental 
 * @return list<Paso> 
 */
list<Paso> ComportamientoIngeniero::PlanificarRedTuberias(int fInicio, int cInicio, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura, int limiteAmbiental){
priority_queue<NodoTuberia, vector<NodoTuberia>, ComparaTuberia> frontera;
/* Matriz 3D: [f][c][delta] 
Con bool, marcabas un estado como visitado la primera vez que lo extraías. 
Esto daba error porque usamos dos criterios de coste: longitud e impacto, esto quería decir que 
una ruta más corta puede sobrepasar el impacto, y marcar x nodo como visitado 
bloqueando una ruta más larga pero más barata en impacto que pasa por ese mismo nodo x.
*/
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
    if (delta == -1) imp = getCosteEcoDIG(terreno[fInicio][cInicio]);
    if (delta ==  1) imp = getCosteEcoRAISE(terreno[fInicio][cInicio]);
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
  if (mi <= actual.impacto) continue;

  // Test objetivo
  if (terreno[actual.f][actual.c] == 'U') {
    if (actual.impacto <= limiteAmbiental) {
      return actual.camino;  // Primera 'U' válida = óptima en longitud y dentro del límite
    }
    continue;  // Supera el límite, no actualizamos min_imp, seguimos buscando
  }

  // Actualizamos mínimo solo para nodos que NO son 'U'
  mi = actual.impacto;

  int h_actual = altura[actual.f][actual.c] + actual.delta;

  // Generamos los 4 vecinos ortogonales (N,S,E,O)
  const int df[] = {-1, 1, 0, 0};
  const int dc[] = { 0, 0,-1, 1};

  for (int dir = 0; dir < 4; dir++) {
    int nf = actual.f + df[dir];
    int nc = actual.c + dc[dir];

    if (!CasillaValidaTuberia(nf, nc, terreno)) continue;

    // Por cada vecino probamos los 3 deltas posibles
    for (int dv = -1; dv <= 1; dv++) {
      if (!DeltaValido(nf, nc, dv, terreno, altura)) continue;

      int h_v = altura[nf][nc] + dv;
      int diff = h_actual - h_v; // La diferencia de altura entre el nodo y su hijo no puede ser mayor de 1 ni menor a 0
      if (diff != 0 && diff != 1) continue;

      NodoTuberia hijo = actual;
      hijo.f = nf; hijo.c = nc; hijo.delta = dv;
      hijo.longitud++;

      unsigned char terr_actual = terreno[actual.f][actual.c];
      unsigned char terr_vecino = terreno[nf][nc];

      // Calculamos la fórmula para cada conexión entre dos casillas:
      // INSTALL(casilla_anterior) + INSTALL(casilla_nueva) + DIG/RAISE(casilla_nueva si aplica)
      int imp_conexion = 0;
      imp_conexion += getCosteEcoINSTALL(terr_actual);
      imp_conexion += getCosteEcoINSTALL(terr_vecino);
      if (dv == -1) imp_conexion += getCosteEcoDIG(terr_vecino);
      if (dv ==  1) imp_conexion += getCosteEcoRAISE(terr_vecino);

      hijo.impacto += imp_conexion;
      Paso p{nf, nc, dv};
      hijo.camino.push_back(p);
      frontera.push(hijo);
    }
  }
}

return list<Paso>();
}

/**
 * @brief Comportamiento del ingeniero para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_4(Sensores sensores)
{
  if (!hayPlan) {
    list<Paso> plan = PlanificarRedTuberias(
      sensores.BelPosF, sensores.BelPosC,
      mapaResultado, mapaCotas, sensores.max_ecologico);
    
    if (!plan.empty()) {
      VisualizaRedTuberias(plan);
      cout << "Nivel 4: plan de " << plan.size() << " tramos" << endl;
    }
    hayPlan = true;
  }
  return IDLE;
}

/**
 * @brief Genera una ruta óptima de navegación desde la posición actual hasta un destino.
 * @param dest_f Fila de la casilla de destino.
 * @param dest_c Columna de la casilla de destino.
 * @param sensores Referencia a los sensores actuales del agente.
 * @note Utiliza el algoritmo de búsqueda en anchura (B_Anchura) y almacena el resultado en la variable global 'plan'.
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
 * @brief Extrae y ejecuta la siguiente acción del plan de navegación.
 * @param sensores Referencia a los sensores actuales del agente.
 * @return Acción a ejecutar (WALK, JUMP, TURN_SL, TURN_SR) o IDLE si hay riesgo de choque o no hay plan.
 * @note Gestiona las colisiones pausando el avance si el Técnico se encuentra en la trayectoria.
 */
Action ComportamientoIngeniero::AvanzarCasilla(const Sensores &sensores) {
    if (!hayPlan || plan.empty()) return IDLE;

    Action accion = plan.front();
    if (RiesgoChoqueTecnico(sensores, accion)) {
        return IDLE; // Pausa sin consumir acción del plan
    } 
    
    plan.pop_front();
    if (plan.empty()) hayPlan = false;
    
    return accion;
}

/**
 * @brief Ejecuta las operaciones de adecuación del terreno (DIG o RAISE).
 * @param tramo_actual Referencia al paso actual del plan de tuberías.
 * @return Acción a ejecutar, o IDLE si la adecuación ya se ha realizado.
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
 * @brief Calcula y ejecuta el giro necesario para mirar hacia una casilla ortogonal.
 * @param dest_f Fila de la casilla objetivo.
 * @param dest_c Columna de la casilla objetivo.
 * @param sensores Datos actuales de los sensores.
 * @return TURN_SR o TURN_SL si necesita girar, IDLE si ya está encarado.
 */
Action ComportamientoIngeniero::OrientarHacia(int dest_f, int dest_c, const Sensores &sensores) {
    Orientacion deseada;
    
    // Calcular orientación ortogonal (asumiendo adyacencia)
    if (dest_f < sensores.posF && dest_c == sensores.posC) deseada = norte;
    else if (dest_f > sensores.posF && dest_c == sensores.posC) deseada = sur;
    else if (dest_f == sensores.posF && dest_c > sensores.posC) deseada = este;
    else if (dest_f == sensores.posF && dest_c < sensores.posC) deseada = oeste;
    else return IDLE; 

    if (sensores.rumbo == deseada) return IDLE;
    
    // Seleccionar el giro más corto
    int diff = (deseada - sensores.rumbo + 8) % 8;
    if (diff <= 4) return TURN_SR;
    else return TURN_SL;
}

/**
 * @brief Comportamiento del ingeniero para el Nivel 5.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores) {
  Action accion = IDLE;
  cout << "[ING] Fase actual: " << fase 
     << " | Pos: (" << sensores.posF << ", " << sensores.posC << ")" << endl;
  switch (fase) {
    case 0: {
      // Planificación de la red
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
      
      if (sensores.posF == dest_f && sensores.posC == dest_c) {
        fase = 2;
        return IDLE;
      }
      
      if (!hayPlan) GenerarRuta(dest_f, dest_c, sensores);
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
      
      // Registrar coordenadas para la invocación del Técnico
      prev_f = sensores.posF;
      prev_c = sensores.posC;
      
      fase = 4;
      return COME; // Transmisión de GotoF y GotoC al Técnico[cite: 4]
    }

    case 4: {
      // Extracción del nodo completado y transición
      plan_tuberias.pop_front(); 
      fase = 5;
      return IDLE; // Consumo de instante para salto de fase lógico
    }

    case 5: {
      // Navegación hacia el siguiente nodo de la tubería
      int dest_f = plan_tuberias.front().fil;
      int dest_c = plan_tuberias.front().col;
      
      if (sensores.posF == dest_f && sensores.posC == dest_c) {
        fase = 6;
        return IDLE;
      }
      
      if (!hayPlan) GenerarRuta(dest_f, dest_c, sensores);
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
      
      if (accion == IDLE) {
        if (sensores.enfrente) {
          accion = INSTALL; // Ejecución simultánea[cite: 4]
          fase = 3;       // Retorno a la fase 3 para invocar al Técnico a este nuevo nodo
        }
      }
      break;
    }
  }

  return accion;
}

/**
 * @brief Comportamiento del ingeniero para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_6(Sensores sensores)
{
  return IDLE;
}

// =========================================================================
// FUNCIONES PROPORCIONADAS
// =========================================================================

/**
 * @brief Actualiza el mapaResultado y mapaCotas con la información de los sensores.
 * @param sensores Datos actuales de los sensores.
 */
void ComportamientoIngeniero::ActualizarMapa(Sensores sensores)
{
  mapaResultado[sensores.posF][sensores.posC] = sensores.superficie[0];
  mapaCotas[sensores.posF][sensores.posC] = sensores.cota[0];

  int pos = 1;
  switch (sensores.rumbo)
  {
  case norte:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
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
      for (int i = -j; i <= j; i++)
      {
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
      for (int i = -j; i <= j; i++)
      {
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
      for (int i = -j; i <= j; i++)
      {
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
bool ComportamientoIngeniero::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas)
{
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
bool ComportamientoIngeniero::EsAccesiblePorAltura(const ubicacion &actual, bool zap)
{
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
ubicacion ComportamientoIngeniero::Delante(const ubicacion &actual) const
{
  ubicacion delante = actual;
  switch (actual.brujula)
  {
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
void ComportamientoIngeniero::PintaPlan(const list<Action> &plan)
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
 * @brief Imprime las coordenadas y operaciones de un plan de tubería.
 *
 * @param plan  Lista de pasos (fila, columna, operación),
 *              donde operacion = -1 (DIG), operación = 1 (RAISE).
 */
void ComportamientoIngeniero::PintaPlan(const list<Paso> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
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

/**
 * @brief Convierte un plan de tubería en la lista de casillas usada
 *        por el sistema de visualización.
 *
 * @param st    Estado de partida (no utilizado directamente).
 * @param plan  Lista de pasos del plan de tubería.
 */
void ComportamientoIngeniero::VisualizaRedTuberias(const list<Paso> &plan)
{
  listaCanalizacionTuberias.clear();
  auto it = plan.begin();
  while (it != plan.end())
  {
    listaCanalizacionTuberias.push_back({it->fil, it->col, it->op});
    it++;
  }
}
