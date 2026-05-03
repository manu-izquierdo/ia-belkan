#ifndef COMPORTAMIENTOTECNICO_H
#define COMPORTAMIENTOTECNICO_H

#include <chrono>
#include <time.h>
#include <thread>
#include <list>

#include "comportamientos/comportamiento.hpp"

// =========================================================================
// DOCUMENTACIÓN PARA ESTUDIANTES
// =========================================================================

// Nivel 3
struct EstadoT {
  ubicacion site;
  bool zapatillas;
  bool operator==(const EstadoT &st) const {
    return site.f == st.site.f && site.c == st.site.c && site.brujula == st.site.brujula && zapatillas == st.zapatillas;
  }
};

struct NodoT {
  EstadoT estado;
  list<Action> secuencia;
  int g;
  int h;
  int f() const { return g + h; }
};

struct ComparaNodos {
  bool operator()(const NodoT& a, const NodoT& b) const {
    return a.f() > b.f();
  }
};

class ComportamientoTecnico : public Comportamiento {
public:
  // =========================================================================
  // CONSTRUCTORES
  // =========================================================================
  
  ComportamientoTecnico(unsigned int size = 0) : Comportamiento(size) {
    tiene_zapatillas = false;
    last_action = IDLE;
    instante=0;
    vector<int> aux(size,0);
    for(int i=0;i<size;i++)
      mtiempo.push_back(aux);
  }

  // [CAMBIO] Se inicializan correctamente todas las variables de Nivel 5.
  // Antes: tieneDestino, ingExpF, ingExpC quedaban sin inicializar (undefined behavior).
  ComportamientoTecnico(std::vector<std::vector<unsigned char>> mapaR, 
                       std::vector<std::vector<unsigned char>> mapaC): 
                       Comportamiento(mapaR, mapaC) {
    tiene_zapatillas = false;
    last_action = IDLE;
    hayPlan = false;
    installIdxT = 0;
    instante = 0;
    // Variables Nivel 5
    tieneDestino = false;
    destF = 0; destC = 0;
    ingExpF = -1; ingExpC = -1; // [NUEVO] -1 = ingeniero no localizado aún
  }

  ComportamientoTecnico(const ComportamientoTecnico &comport): Comportamiento(comport) {}
  ~ComportamientoTecnico() {}

  Action think(Sensores sensores);

  ComportamientoTecnico *clone() {
    return new ComportamientoTecnico(*this);
  }

  // =========================================================================
  // ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
  // =========================================================================
  
  Action ComportamientoTecnicoNivel_0(Sensores sensores);
  Action ComportamientoTecnicoNivel_1(Sensores sensores);
  Action ComportamientoTecnicoNivel_2(Sensores sensores);
  Action ComportamientoTecnicoNivel_3(Sensores sensores);
  Action ComportamientoTecnicoNivel_4(Sensores sensores);
  Action ComportamientoTecnicoNivel_5(Sensores sensores);
  Action ComportamientoTecnicoNivel_6(Sensores sensores);

protected:
  // =========================================================================
  // FUNCIONES PROPORCIONADAS
  // =========================================================================

  void ActualizarMapa(Sensores sensores);
  bool EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas);
  bool EsAccesiblePorAltura(const ubicacion &actual);
  ubicacion Delante(const ubicacion &actual) const;
  bool es_camino(unsigned char c) const;
  void PintaPlan(const list<Action> &plan);
  void PintaPlan(const list<Paso> &plan);
  void VisualizaPlan(const ubicacion &st, const list<Action> &plan);

private:
  // =========================================================================
  // VARIABLES DE ESTADO
  // =========================================================================

  bool tiene_zapatillas;
  Action last_action;
  vector<vector<int>> mtiempo;
  int instante;

  bool hayPlan;
  list<Action> plan;

  // Nivel 3
  bool esSuperficieValida(unsigned char superficie, unsigned char agente, const bool zap) const;
  int CostoEnergiaTecnico(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);
  bool CasillaAccesibleTecnico(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);
  EstadoT applyT(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);
  int Heuristica(const EstadoT &actual, const EstadoT &objetivo);
  bool RiesgoChoqueIngeniero(const Sensores &sensores, Action accion);
  list<Action> A_Estrella(const EstadoT &inicio, const EstadoT &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura);

  // Variable Nivel 5
  vector<Paso> planVec;
  int installIdxT;
  int estado_instalacion_t;
  list<Action> plan_actual_t;

  int destF, destC;       // Destino del último COME del Ingeniero (= tramoTec)
  bool tieneDestino;      // true si tenemos un destino activo

  // [NUEVO] Posición del ingeniero detectada durante la navegación.
  // Se usa en EjecutarInstalacionOEspera para orientarse directamente
  // hacia el ingeniero sin necesidad de hacer TURN_SR ciego.
  // Valor -1 indica "no detectado aún".
  int ingExpF, ingExpC;

  // Helpers de Nivel 5
  void ProcesarLlamadaIngeniero(const Sensores &sensores);
  Action NavegacionSeguraTecnico(const Sensores &sensores);
  Action EjecutarInstalacionOEspera(const Sensores &sensores);

  // [NUEVO] Utilidades de orientación (equivalentes a las del Ingeniero).
  // Necesarias para que EjecutarInstalacionOEspera haga GiroHacia directo.
  Orientacion OrientacionHacia(int f1, int c1, int f2, int c2);
  Action GiroHacia(Orientacion actual, Orientacion objetivo);
};

#endif
