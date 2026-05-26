#ifndef COMPORTAMIENTOINGENIERO_H
#define COMPORTAMIENTOINGENIERO_H

#include <chrono>
#include <climits>
#include <list>
#include <map>
#include <set>
#include <thread>
#include <time.h>

#include "comportamientos/comportamiento.hpp"

// Nivel 2
struct EstadoI {
  ubicacion site;
  bool zapatillas;
  bool operator==(const EstadoI &st) const {
    return site.f == st.site.f && site.c == st.site.c && site.brujula == st.site.brujula && zapatillas == st.zapatillas;
  }
};

struct NodoI {
  EstadoI estado;
  list<Action> secuencia;
  bool operator==(const NodoI &node) const {
    return estado == node.estado;
  }
};

// Nivel 4
struct NodoTuberia {
  int f, c;          // Fila y Columna
  int delta;         // Indica si si tiene modificación de altura {-1,0,+1}
  int longitud;      // Número de pasos hasta aquí
  int impacto;       // Impacto ecológico
  list<Paso> camino; // Secuencia de pasos de la tubería {f, c, op}
  bool agua = false;
};

// Comparador para la priority_queue:
// La prioridad mas alta es la (longitud, impacto) mas pequeña, asi que devolvemos true cuando
// 'a' debe ir DESPUES que 'b' (ya que la priority_queue ordena de mayor a menor).
struct ComparaTuberia {
  bool operator()(const NodoTuberia &a, const NodoTuberia &b) const {
    // Lex: primero longitud, luego impacto
    if (a.longitud != b.longitud)
      return a.longitud > b.longitud;
    return a.impacto > b.impacto;
  }
};

// Nivel 5

class ComportamientoIngeniero : public Comportamiento {
public:
  // =========================================================================
  // CONSTRUCTORES
  // =========================================================================

  /**
   * @brief Constructor para niveles 0, 1 y 6 (sin mapa completo)
   * @param size Tamaño del mapa (si es 0, se inicializa más tarde)
   */
  ComportamientoIngeniero(unsigned int size = 0) : Comportamiento(size) {
    // Inicializar Variables de Estado
    tiene_zapatillas = false; // Nada mas empezar el jugador no tiene zapatillas

    instante = 0;             // Comenzará en el instante 0
    vector<int> aux(size, 0); // Creamos un vector del tamaño el tamaño del mapa relleno de ceros
    for (int i = 0; i < size; i++)
      mtiempo.push_back(aux); // Hacemos un push back sobre el vector de vectores llamado mtiempo de manera que la matriz comience llena de ceros
  }

  /**
   * @brief Constructor para niveles 2, 3, 4 y 5 (con mapa completo conocido)
   * @param mapaR Mapa de terreno conocido
   * @param mapaC Mapa de cotas conocido
   */
  ComportamientoIngeniero(std::vector<std::vector<unsigned char>> mapaR,
                          std::vector<std::vector<unsigned char>> mapaC) : Comportamiento(mapaR, mapaC) {
    // Inicializar Variables de Estado
    tiene_zapatillas = false;
    hayPlan = false;
  }

  ComportamientoIngeniero(const ComportamientoIngeniero &comport)
      : Comportamiento(comport) {}
  ~ComportamientoIngeniero() {}

  /**
   * @brief Bucle principal de decisión del agente.
   * Estudia los sensores y decide la siguiente acción.
   *
   * EJEMPLO DE USO:
   * Action accion = think(sensores);
   * return accion; // El motor ejecutará esta acción
   */
  Action think(Sensores sensores);

  ComportamientoIngeniero *clone() {
    return new ComportamientoIngeniero(*this);
  }

  // =========================================================================
  // ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
  // =========================================================================

  // Funciones específicas para cada nivel (para ser implementadas por el alumno)

  /**
   * @brief Implementación del Nivel 0.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_0(Sensores sensores);

  /**
   * @brief Implementación del Nivel 1.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_1(Sensores sensores);

  /**
   * @brief Implementación del Nivel 2.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_2(Sensores sensores);

  /**
   * @brief Implementación del Nivel 3.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_3(Sensores sensores);

  /**
   * @brief Implementación del Nivel 4.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_4(Sensores sensores);

  /**
   * @brief Implementación del Nivel 5.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_5(Sensores sensores);

  /**
   * @brief Implementación del Nivel 6.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_6(Sensores sensores);

protected:
  // =========================================================================
  // FUNCIONES PROPORCIONADAS
  // =========================================================================

  /**
   * @brief Actualiza la información del mapa interno basándose en los sensores.
   * IMPORTANTE: Esta función ya está implementada. Actualiza mapaResultado y mapaCotas
   * con la información de los 16 sensores (casilla actual + 15 casillas alrededor).
   */
  void ActualizarMapa(Sensores sensores);

  /**
   * @brief Comprueba si una casilla es transitable.
   * @param f Fila de la casilla.
   * @param c Columna de la casilla.
   * @param tieneZapatillas Indica si el agente posee zapatillas.
   * @return true si la casilla es transitable (no es muro ni precipicio).
   */
  bool EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas);

  /**
   * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
   * REGLAS: Desnivel máximo 1 sin zapatillas, 2 con zapatillas.
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return true si el desnivel con la casilla de delante es admisible.
   */
  bool EsAccesiblePorAltura(const ubicacion &actual, bool zap);

  /**
   * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return Estado con la fila y columna de la casilla de enfrente.
   */
  ubicacion Delante(const ubicacion &actual) const;

  bool es_camino(unsigned char c) const;

  /**
   * @brief Imprime por consola la secuencia de acciones de un plan para un agente.
   * @param plan  Lista de acciones del plan.
   */
  void PintaPlan(const list<Action> &plan);

  /**
   * @brief Imprime las coordenadas y operaciones de un plan de tubería.
   * @param plan  Lista de pasos (fila, columna, operación).
   */
  void PintaPlan(const list<Paso> &plan);

  /**
   * @brief Convierte un plan de acciones en una lista de casillas para
   *        su visualización en el mapa gráfico.
   * @param st    Estado de partida.
   * @param plan  Lista de acciones del plan.
   */
  void VisualizaPlan(const ubicacion &st, const list<Action> &plan);

  /**
   * @brief Convierte un plan de tubería en la lista de casillas usada
   *        por el sistema de visualización.
   * @param st    Estado de partida (no utilizado directamente).
   * @param plan  Lista de pasos del plan de tubería.
   */
  void VisualizaRedTuberias(const list<Paso> &plan);

private:
  // =========================================================================
  // VARIABLES DE ESTADO (PUEDEN SER EXTENDIDAS POR EL ALUMNO)
  // =========================================================================
  // Como los sensores se borran en cada turno creamos variables que guarden estados concretos para que sean persistentes

  bool tiene_zapatillas;

  // NIVEL 0-1 (Reactivos)
  // "Mapa de Pulgarcito" (sugerencia del profesor)
  vector<vector<int>> mtiempo; // Se usará para que se tenga un mapa de valores que representará el recorrido, de manera que intente acceder a la posición que hace más tiempo que no accedió
  int instante;                // Variable que se incrementará representando el recorrido hecho por el jugador

  // NIVEL 2-6 (Deliberativos)
  bool hayPlan;      // Nos dirá si se ha planteado una ruta
  list<Action> plan; // Ruta que el agente ejecutará
  
  // NIVEL 4-5-6
  list<Paso> plan_tuberias; // Guarda el plan generado por PlanificarRedTuberias
  
  // Nivel 5
  int fase = 0;             // Controla el estado del Nivel 5 (0=Planificar, 1=Ir Belkanita, 2=Avanzar nodo, etc.)
  int prev_f = -1;          // Almacena la fila del nodo anterior
  int prev_c = -1;          // Almacena la columna del nodo anterior

  // Nivel 6
  bool exploracion_terminada = false;
  int cooldown = 0;  // Turnos restantes hasta reintentar PlanificarRedTuberias

  // =========================================================================
  // FUNCIONES
  // =========================================================================

  // Nivel 0
  char ViablePorAltura(char terreno, int dif_cota, bool zap) const;
  int SeleccionaCasillaPulgarcito0(char i, char c, char d, int t_i, int t_c, int t_d, bool zap) const;


  // Nivel 1
  char TransitableNivel1(char terreno, int dif_cota, bool zap) const;
  int SeleccionaCasillaPulgarcito1(char i, char c, char d, int t_i, int t_c, int t_d, bool zap) const;


  // Nivel 2
  bool esSuperficieValida(unsigned char superficie) const;
  bool CasillaAccesibleIngeniero(Action accion, const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) const;
  EstadoI applyI(Action accion, const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) const;
  list<Action> B_Anchura(const EstadoI &inicio, const EstadoI &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);
  bool RiesgoChoqueTecnico(const Sensores &sensores, Action accion) const;


  // Nivel 4
  bool DeltaValido(int f, int c, int delta, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) const;
  bool CasillaValidaTuberia(int f, int c, const vector<vector<unsigned char>> &terreno) const;
  int getCosteEco(unsigned char sup, Action accion) const;
  list<Paso> PlanificarRedTuberias(int fInicio, int cInicio, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura, int limiteAmbiental);

  
  // Nivel 5
  void GenerarRuta(int dest_f, int dest_c, const Sensores &sensores);
  Action AvanzarCasilla(const Sensores &sensores);
  Action AdecuarTerreno(Paso &tramo_actual);
  Action OrientarHacia(int dest_f, int dest_c, const Sensores &sensores);
};

#endif
