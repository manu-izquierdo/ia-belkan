#ifndef COMPORTAMIENTOTECNICO_H
#define COMPORTAMIENTOTECNICO_H

#include <chrono>
#include <climits>
#include <list>
#include <thread>
#include <time.h>

#include "comportamientos/comportamiento.hpp"

// =========================================================================
// DOCUMENTACIÓN PARA ESTUDIANTES
// =========================================================================
/*
 * CLASE: ComportamientoTecnico
 *
 * DESCRIPCIÓN:
 * Esta clase implementa el comportamiento del agente Técnico en el mundo Belkan.
 * El técnico colabora con el ingeniero para resolver el problema de instalación de tuberías
 */

// Nivel 3
struct EstadoT {
  ubicacion site;
  bool zapatillas;
  bool operator==(const EstadoT &st) const {
    return site.f == st.site.f && site.c == st.site.c && site.brujula == st.site.brujula && zapatillas == st.zapatillas && meahogo == st.meahogo;
  }
  int meahogo = 0;
};

struct NodoT {
  EstadoT estado;
  list<Action> secuencia;
  int g; // Coste acumulado
  int h; // Heurística
  int f() const {
    return g + h;
  }
};

// Comparador para la cola de prioridad
struct ComparaNodos {
  bool operator()(const NodoT &a, const NodoT &b) const {
    return a.f() > b.f(); // Mayor f() tiene menor prioridad
  }
};

class ComportamientoTecnico : public Comportamiento {
public:
  // =========================================================================
  // CONSTRUCTORES
  // =========================================================================

  /**
   * @brief Constructor para niveles 0, 1 y 6 (sin mapa completo)
   * @param size Tamaño del mapa (si es 0, se inicializa más tarde)
   */
  ComportamientoTecnico(unsigned int size = 0) : Comportamiento(size) {
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
  ComportamientoTecnico(std::vector<std::vector<unsigned char>> mapaR,
                        std::vector<std::vector<unsigned char>> mapaC) : Comportamiento(mapaR, mapaC) {
    // Inicializar Variables de Estado
    tiene_zapatillas = false;
    hayPlan = false;
  }

  ComportamientoTecnico(const ComportamientoTecnico &comport) : Comportamiento(comport) {}
  ~ComportamientoTecnico() {}

  /**
   * @brief Bucle principal de decisión del técnico.
   * Estudia los sensores y decide la siguiente acción.
   *
   * EJEMPLO DE USO:
   * Action accion = think(sensores);
   * return accion; // El motor ejecutará esta acción
   */
  Action think(Sensores sensores);

  ComportamientoTecnico *clone() {
    return new ComportamientoTecnico(*this);
  }

  // =========================================================================
  // ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
  // =========================================================================

  /**
   * @brief Comportamiento del técnico para el Nivel 0.
   * @param sensores Datos actuales de los sensores.
   * @return Acción a realizar.
   */
  Action ComportamientoTecnicoNivel_0(Sensores sensores);

  /**
   * @brief Comportamiento del técnico para el Nivel 1.
   * @param sensores Datos actuales de los sensores.
   * @return Acción a realizar.
   */
  Action ComportamientoTecnicoNivel_1(Sensores sensores);

  // Parte 2

  /**
   * @brief Comportamiento del técnico para el Nivel 2.
   * @param sensores Datos actuales de los sensores.
   * @return Acción a realizar.
   */
  Action ComportamientoTecnicoNivel_2(Sensores sensores);

  /**
   * @brief Comportamiento del técnico para el Nivel 3.
   * @param sensores Datos actuales de los sensores.
   * @return Acción a realizar.
   */
  Action ComportamientoTecnicoNivel_3(Sensores sensores);

  /**
   * @brief Comportamiento del técnico para el Nivel 4.
   * @param sensores Datos actuales de los sensores.
   * @return Acción a realizar.
   */
  Action ComportamientoTecnicoNivel_4(Sensores sensores);

  /**
   * @brief Comportamiento del técnico para el Nivel 5.
   * @param sensores Datos actuales de los sensores.
   * @return Acción a realizar.
   */
  Action ComportamientoTecnicoNivel_5(Sensores sensores);

  /**
   * @brief Comportamiento del técnico para el Nivel 6.
   * @param sensores Datos actuales de los sensores.
   * @return Acción a realizar.
   */
  Action ComportamientoTecnicoNivel_6(Sensores sensores);

protected:
  // =========================================================================
  // FUNCIONES PROPORCIONADAS
  // =========================================================================

  /**
   * @brief Actualiza el mapaResultado y mapaCotas con la información de los sensores.
   * IMPORTANTE: Esta función ya está implementada. Actualiza mapaResultado y mapaCotas
   * con la información de los 16 sensores.
   */
  void ActualizarMapa(Sensores sensores);

  /**
   * @brief Determina si una casilla es transitable para el técnico.
   * NOTA: El técnico puede tener reglas de transitabilidad diferentes al ingeniero.
   * @param f Fila de la casilla.
   * @param c Columna de la casilla.
   * @param tieneZapatillas Indica si el agente posee las zapatillas.
   * @return true si la casilla es transitable.
   */
  bool EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas);

  /**
   * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
   * REGLA PARA TÉCNICO: Desnivel máximo siempre 1 (independiente de zapatillas).
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return true si el desnivel con la casilla de delante es admisible.
   */
  bool EsAccesiblePorAltura(const ubicacion &actual);

  /**
   * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return Estado con la fila y columna de la casilla de enfrente.
   */
  ubicacion Delante(const ubicacion &actual) const;

  /**
   * @brief Comprueba si una celda es de tipo transitable por defecto.
   * @param c Carácter que representa el tipo de superficie.
   * @return true si es camino ('C'), zapatillas ('D') o meta ('U').
   */
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
  
  // Nivel 5
  int fase = 0;
  int destino_f = -1;
  int destino_c = -1;

  // =========================================================================
  // FUNCIONES
  // =========================================================================

  // Nivel 0
  char ViablePorAltura(char terreno, int dif_cota) const;
  int SeleccionaCasillaPulgarcito0(char i, char c, char d, int t_i, int t_c, int t_d) const;


  // Nivel 1
  char TransitableNivel1(char terreno, int dif_cota, bool zap) const;
  int SeleccionaCasillaPulgarcito1(char i, char c, char d, int t_i, int t_c, int t_d, bool zap) const;
  

  // Nivel 2
  bool esSuperficieValida(unsigned char superficie, bool zap) const;
  bool RiesgoChoqueIngeniero(const Sensores &sensores, Action accion) const;


  // Nivel 3
  int CostoEnergiaTecnico(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) const;
  bool CasillaAccesibleTecnico(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) const;
  EstadoT applyT(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) const;
  int Heuristica(const EstadoT &actual, const EstadoT &objetivo) const;
  list<Action> A_Estrella(const EstadoT &inicio, const EstadoT &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);


  // Nivel 5
  void GenerarRuta(int dest_f, int dest_c, const Sensores &sensores);
  Action AvanzarCasilla(const Sensores &sensores);

  // Nivel 6
  Action ExploracionDirigida(int dest_f, int dest_c, const Sensores &sensores);
};

#endif
