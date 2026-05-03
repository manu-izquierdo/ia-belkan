#ifndef COMPORTAMIENTOINGENIERO_H
#define COMPORTAMIENTOINGENIERO_H

#include <chrono>
#include <list>
#include <map>
#include <set>
#include <thread>
#include <time.h>
#include <climits>

#include "comportamientos/comportamiento.hpp"

// Nivel 2
struct EstadoI {
  ubicacion site;
  bool zapatillas;
  bool operator==(const EstadoI &st) const{
    return site.f == st.site.f && site.c == st.site.c && site.brujula == st.site.brujula && zapatillas == st.zapatillas;
  }
};

struct NodoI{
  EstadoI estado;
  list<Action> secuencia;
  bool operator==(const NodoI &node) const{
    return estado == node.estado;
  }
};


// Nivel 4
struct NodoTuberia {
  int f, c; // Fila y Columna
  int delta; // Indica si si tiene modificación de altura {-1,0,+1}
  int longitud; // Número de pasos hasta aquí
  int impacto; // Impacto ecológico
  list<Paso> camino; // Secuencia de pasos de la tubería {f, c, op}
};

// Comparador para la priority_queue:
// La prioridad mas alta es la (longitud, impacto) mas pequeña, asi que devolvemos true cuando
// 'a' debe ir DESPUES que 'b' (ya que la priority_queue ordena de mayor a menor).
struct ComparaTuberia {
  bool operator()(const NodoTuberia& a, const NodoTuberia& b) const {
    // Lex: primero longitud, luego impacto
    if (a.longitud != b.longitud) return a.longitud > b.longitud;
    return a.impacto > b.impacto;
  }
};


class ComportamientoIngeniero : public Comportamiento {
public:
  // =========================================================================
  // CONSTRUCTORES
  // =========================================================================
  
  ComportamientoIngeniero(unsigned int size = 0) : Comportamiento(size) {
    tiene_zapatillas = false;
    last_action = IDLE;
    instante=0;
    vector<int> aux(size,0);
    for(int i=0;i<size;i++)
      mtiempo.push_back(aux);
  }

  ComportamientoIngeniero(std::vector<std::vector<unsigned char>> mapaR, 
                         std::vector<std::vector<unsigned char>> mapaC): 
                         Comportamiento(mapaR, mapaC) {
    tiene_zapatillas = false;
    last_action = IDLE;
    hayPlan = false;
    installIdx = 0;
    instante = 0;
  }

  ComportamientoIngeniero(const ComportamientoIngeniero &comport)
      : Comportamiento(comport) {}
  ~ComportamientoIngeniero() {}

  Action think(Sensores sensores);

  ComportamientoIngeniero *clone() {
    return new ComportamientoIngeniero(*this);
  }

  // =========================================================================
  // ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
  // =========================================================================

  Action ComportamientoIngenieroNivel_0(Sensores sensores);
  Action ComportamientoIngenieroNivel_1(Sensores sensores);
  Action ComportamientoIngenieroNivel_2(Sensores sensores);
  Action ComportamientoIngenieroNivel_3(Sensores sensores);
  Action ComportamientoIngenieroNivel_4(Sensores sensores);
  Action ComportamientoIngenieroNivel_5(Sensores sensores);
  Action ComportamientoIngenieroNivel_6(Sensores sensores);

protected:
  // =========================================================================
  // FUNCIONES PROPORCIONADAS
  // =========================================================================

  void ActualizarMapa(Sensores sensores);
  bool EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas);
  bool EsAccesiblePorAltura(const ubicacion &actual, bool zap);
  ubicacion Delante(const ubicacion &actual) const;
  bool es_camino(unsigned char c) const;
  void PintaPlan(const list<Action> &plan);
  void PintaPlan(const list<Paso> &plan);
  void VisualizaPlan(const ubicacion &st, const list<Action> &plan);
  void VisualizaRedTuberias(const list<Paso> &plan);

private:
  // =========================================================================
  // VARIABLES DE ESTADO
  // =========================================================================

  bool tiene_zapatillas;
  Action last_action;
  vector<vector<int>> mtiempo;
  int instante;

  // =========================================================================
  // VARIABLES DELIBERATIVAS (Niveles 2, 3, 4, 5, 6)
  // =========================================================================

  bool hayPlan;
  list<Action> plan;
  
  // Nivel 2
  bool esSuperficieValida(unsigned char superficie) const;  
  bool CasillaAccesibleIngeniero(Action accion, const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);
  EstadoI applyI(Action accion, const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);
  bool RiesgoChoqueTecnico(const Sensores &sensores, Action accion);
  list<Action> B_Anchura(const EstadoI &inicio, const EstadoI &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);
  
  // Nivel 4
  bool DeltaValido(int f, int c, int delta, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);
  bool CasillaValidaTuberia(int f, int c, const vector<vector<unsigned char>> &terreno);
  int getCosteEcoINSTALL(unsigned char terr);
  int getCosteEcoRAISE(unsigned char terr);
  int getCosteEcoDIG(unsigned char terr);
  list<Paso> PlanificarRedTuberias(int fInicio, int cInicio, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura, int limiteAmbiental);

  // Nivel 5
  vector<Paso> planVec;   // Plan como vector (acceso por índice)
  int installIdx;          // Tramo que el Ingeniero está instalando (empieza en 1)
  bool opDone;             // true si ya hicimos el RAISE/DIG de este tramo
  int faseN5;              // 0=POSICIONAR, 1=COME, 2=MOVER, 3=GIRAR, 4=ESPERAR

  Orientacion OrientacionHacia(int f1, int c1, int f2, int c2);
  Action GiroHacia(Orientacion actual, Orientacion objetivo);

  // [NUEVO] Encapsula la navegación con evasión de colisiones.
  // Calcula o reutiliza el plan hacia (destF, destC).
  // En caso de colisión con el técnico, espera 1 tick y limpia el plan
  // para recalcular en el siguiente tick desde la nueva posición.
  Action NavegacionIngenieroHacia(int destF, int destC, const Sensores &sensores);
};

#endif
