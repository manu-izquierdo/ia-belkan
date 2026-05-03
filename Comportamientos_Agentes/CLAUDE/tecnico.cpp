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

  switch (sensores.nivel) {
    case 0: accion = ComportamientoTecnicoNivel_0(sensores); break;
    case 1: accion = ComportamientoTecnicoNivel_1(sensores); break;
    case 2: accion = ComportamientoTecnicoNivel_2(sensores); break;
    case 3: accion = ComportamientoTecnicoNivel_3(sensores); break;
    case 4: accion = ComportamientoTecnicoNivel_4(sensores); break;
    case 5: accion = ComportamientoTecnicoNivel_5(sensores); break;
    case 6: accion = ComportamientoTecnicoNivel_6(sensores); break;
  }

  return accion;
}


char ViablePorAltura0T (char casilla, int dif) {
  if (abs(dif) <= 1){
    return casilla;
  } else {
    return 'P';
  }
}

int VeoCasillaPulgarcitoNivel0T (char i, char c, char d, int t_i, int t_c, int t_d) {
    if (c == 'U') return 2;
    else if (i == 'U') return 1;
    else if (d == 'U') return 3;

    if (i != 'C') t_i = 999999;
    if (c != 'C') t_c = 999999;
    if (d != 'C') t_d = 999999;

    if (t_i == 999999 && t_c == 999999 && t_d == 999999) return 0;

    int min_tiempo = min(t_i, min(t_c, t_d));

    if (min_tiempo == t_c) return 2;
    if (min_tiempo == t_i) return 1;
    if (min_tiempo == t_d) return 3;

    return 0;
}

Action ComportamientoTecnico::ComportamientoTecnicoNivel_0(Sensores sensores)
{
  Action accion = IDLE;
  
  ActualizarMapa(sensores);
  
  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  if(sensores.superficie[0]=='U')  return IDLE;

  char i = ViablePorAltura0T (sensores.superficie[1], sensores.cota[1]-sensores.cota[0]); 
  char c = ViablePorAltura0T (sensores.superficie[2], sensores.cota[2]-sensores.cota[0]);
  char d = ViablePorAltura0T (sensores.superficie[3], sensores.cota[3]-sensores.cota[0]);
  
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

  int t_i = (posI.f >= 0 && posI.f < (int)mtiempo.size() && posI.c >= 0 && posI.c < (int)mtiempo[0].size()) ? mtiempo[posI.f][posI.c] : 999999;
  int t_c = (posC.f >= 0 && posC.f < (int)mtiempo.size() && posC.c >= 0 && posC.c < (int)mtiempo[0].size()) ? mtiempo[posC.f][posC.c] : 999999;
  int t_d = (posD.f >= 0 && posD.f < (int)mtiempo.size() && posD.c >= 0 && posD.c < (int)mtiempo[0].size()) ? mtiempo[posD.f][posD.c] : 999999;

  int pos = VeoCasillaPulgarcitoNivel0T(i, c, d, t_i, t_c, t_d);

  switch (pos)
  {
  case 2: accion = WALK;    break;
  case 1: accion = TURN_SL; break;
  case 3: accion = TURN_SR; break;
  default: accion = TURN_SL; break;
  }

  instante++;
  mtiempo[sensores.posF][sensores.posC]=instante;
  
  last_action=accion;
  return accion;
}

bool ComportamientoTecnico::es_camino(unsigned char c) const {
  return (c == 'C' || c == 'D' || c == 'U');
}

char ViablePorAltura1T (char casilla, int dif, bool zap) {
  if (casilla == 'M' || casilla == 'A' || casilla == 'P') return 'P'; 
  if (casilla == 'B' && !zap) return 'P';
  if (abs(dif) <= 1){
    return casilla;
  } else {
    return 'P';
  }
}

int VeoCasillaPulgarcitoNivel1T (char i, char c, char d, int t_i, int t_c, int t_d, bool zap) {
  if (c == 'P') t_c = 999999;
  if (i == 'P') t_i = 999999;
  if (d == 'P') t_d = 999999;

  if (t_i == 999999 && t_c == 999999 && t_d == 999999) return 0;

  int min_tiempo = min(t_i, min(t_c, t_d));

  if (!zap) {
      if (c == 'D') return 2;
      if (i == 'D') return 1;
      if (d == 'D') return 3;
  }

  if (min_tiempo == 0) {
      if ((c == 'C' || c == 'S') && t_c == 0) return 2;
      if ((i == 'C' || i == 'S') && t_i == 0) return 1;
      if ((d == 'C' || d == 'S') && t_d == 0) return 3;
  }

  if (min_tiempo == t_c) return 2;
  if (min_tiempo == t_i) return 1;
  if (min_tiempo == t_d) return 3;

  return 0;
}

Action ComportamientoTecnico::ComportamientoTecnicoNivel_1(Sensores sensores)
{
  Action accion = IDLE;

  ActualizarMapa(sensores);

  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  char i = ViablePorAltura1T (sensores.superficie[1], sensores.cota[1]-sensores.cota[0], tiene_zapatillas);
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

  int t_i = (posI.f >= 0 && posI.f < (int)mtiempo.size() && posI.c >= 0 && posI.c < (int)mtiempo[0].size()) ? mtiempo[posI.f][posI.c] : 999999;
  int t_c = (posC.f >= 0 && posC.f < (int)mtiempo.size() && posC.c >= 0 && posC.c < (int)mtiempo[0].size()) ? mtiempo[posC.f][posC.c] : 999999;
  int t_d = (posD.f >= 0 && posD.f < (int)mtiempo.size() && posD.c >= 0 && posD.c < (int)mtiempo[0].size()) ? mtiempo[posD.f][posD.c] : 999999;

  int pos = VeoCasillaPulgarcitoNivel1T(i, c, d, t_i, t_c, t_d, tiene_zapatillas);

  switch (pos)
  {
  case 2: accion = WALK;    break;
  case 1: accion = TURN_SL; break;
  case 3: accion = TURN_SR; break;
  default: accion = TURN_SL; break;
  }

  instante++;
  mtiempo[sensores.posF][sensores.posC]=instante;
  
  last_action=accion;
  return accion;
}

// Parte 2

bool ComportamientoTecnico::esSuperficieValida(unsigned char superficie, unsigned char agente, const bool zap) const {
  if (superficie == 'M' || superficie == 'P' || agente != '_') return false;
  if (superficie == 'B') return zap;
  return true;
}

Action ComportamientoTecnico::ComportamientoTecnicoNivel_2(Sensores sensores) {
  int dif = sensores.cota[2] - sensores.cota[0];
  bool transitable = esSuperficieValida(sensores.superficie[2], sensores.agentes[2], tiene_zapatillas);

  if (transitable && abs(dif) <= 1) {
    return WALK;
  }
  
  return TURN_SR;
}

bool ComportamientoTecnico::CasillaAccesibleTecnico(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) {
    if (accion != WALK) return true;

    ubicacion casilla_delante = Delante(st.site);
    
    if (casilla_delante.f < 0 || casilla_delante.f >= (int)terreno.size() || casilla_delante.c < 0 || casilla_delante.c >= (int)terreno[0].size()) return false;
    if (!esSuperficieValida(terreno[casilla_delante.f][casilla_delante.c],'_',st.zapatillas)) return false;

    int dif = altura[casilla_delante.f][casilla_delante.c] - altura[st.site.f][st.site.c];
    return (abs(dif) <= 1);
}

EstadoT ComportamientoTecnico::applyT(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) {
  EstadoT next = st;
  switch(accion) {
    case WALK:    next.site = Delante(st.site); break;
    case TURN_SR: next.site.brujula = (Orientacion)((next.site.brujula + 1) % 8); break;
    case TURN_SL: next.site.brujula = (Orientacion)((next.site.brujula + 7) % 8); break;
  }
  if (terreno[next.site.f][next.site.c] == 'D') next.zapatillas = true;
  return next;
}

int ComportamientoTecnico::Heuristica(const EstadoT &actual, const EstadoT &objetivo) {
  return max(abs(actual.site.f - objetivo.site.f), abs(actual.site.c - objetivo.site.c));
}

int ComportamientoTecnico::CostoEnergiaTecnico(Action accion, const EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) {
  unsigned char sup = terreno[st.site.f][st.site.c];
  int costo = 0;
  
  if (accion == WALK) {
    ubicacion destino = Delante(st.site);
    int dif_altura = altura[destino.f][destino.c] - altura[st.site.f][st.site.c];
    
    if (sup == 'A') {
      costo = 60;
      if (dif_altura > 0) costo += 5;
      else if (dif_altura < 0) costo -= 2;
    } else if (sup == 'H') {
      costo = 6;
      if (dif_altura > 0) costo += 5;
      else if (dif_altura < 0) costo -= 2;
    } else if (sup == 'S') {
      costo = 3;
      if (dif_altura > 0) costo += 5;
      else if (dif_altura < 0) costo -= 2;
    } else {
      costo = 1;
    }
  } else if (accion == TURN_SL || accion == TURN_SR) {
    if (sup == 'A') costo = 5;
    else if (sup == 'H') costo = 2;
    else costo = 1;
  }
  return costo;
}
    
list<Action> ComportamientoTecnico::A_Estrella(const EstadoT &inicio, const EstadoT &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) {
  priority_queue<NodoT, vector<NodoT>, ComparaNodos> frontier;
  list<Action> camino_solucion;
  
  vector<vector<vector<vector<bool>>>> visitados(terreno.size(), 
      vector<vector<vector<bool>>>(terreno[0].size(), 
      vector<vector<bool>>(8, 
      vector<bool>(2, false))));
  
  NodoT n_inicial;
  n_inicial.estado = inicio;
  n_inicial.g = 0;
  n_inicial.h = Heuristica(inicio, final);
  frontier.push(n_inicial);
  
  while (!frontier.empty()) {
    NodoT nodo_actual = frontier.top();
    frontier.pop();

    int zapatillas = nodo_actual.estado.zapatillas ? 1 : 0;
    
    if (visitados[nodo_actual.estado.site.f][nodo_actual.estado.site.c][nodo_actual.estado.site.brujula][zapatillas]) continue;
    visitados[nodo_actual.estado.site.f][nodo_actual.estado.site.c][nodo_actual.estado.site.brujula][zapatillas] = true;
    
    if (nodo_actual.estado.site.f == final.site.f && nodo_actual.estado.site.c == final.site.c) {
      camino_solucion = nodo_actual.secuencia;
      break;
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
  return camino_solucion;
}

bool ComportamientoTecnico::RiesgoChoqueIngeniero(const Sensores &sensores, Action accion) {
    switch (accion) {
        case WALK:
            return (sensores.agentes[2] == 'i');
        default:
            return false;
    }
}
  
Action ComportamientoTecnico::ComportamientoTecnicoNivel_3(Sensores sensores)
{
  Action accion = IDLE;

  if (!hayPlan) {
      EstadoT inicio, destino;
      inicio.site.f = sensores.posF;
      inicio.site.c = sensores.posC;
      inicio.site.brujula = sensores.rumbo;
      inicio.zapatillas = tiene_zapatillas;

      destino.site.f = sensores.BelPosF;
      destino.site.c = sensores.BelPosC;
      destino.site.brujula = norte;
      destino.zapatillas = false;

      plan = A_Estrella(inicio, destino, mapaResultado, mapaCotas);
      VisualizaPlan(inicio.site,plan);
      hayPlan = plan.size()!=0;
  }

  if (hayPlan && plan.size() > 0) {
    accion = plan.front();
    if(RiesgoChoqueIngeniero(sensores, accion)){
      accion = IDLE;
    } else {
        plan.pop_front();
    }
  }

  if (plan.size() == 0) {
      hayPlan = false;
  }

  return accion;
}

Action ComportamientoTecnico::ComportamientoTecnicoNivel_4(Sensores sensores) {
  return IDLE;
}


// =========================================================================
// NIVEL 5 — HELPERS Y COMPORTAMIENTO PRINCIPAL
// =========================================================================

/**
 * @brief [NUEVO] Calcula la orientación cardinal para ir de (f1,c1) a (f2,c2).
 * Idéntica a la del Ingeniero; necesaria para orientarse sin TURN_SR ciego.
 */
Orientacion ComportamientoTecnico::OrientacionHacia(int f1, int c1, int f2, int c2) {
    if (f2 < f1) return norte;
    if (f2 > f1) return sur;
    if (c2 < c1) return oeste;
    return este;
}

/**
 * @brief [NUEVO] Devuelve el giro más corto de 'actual' a 'objetivo'.
 * Idéntica a la del Ingeniero; necesaria para EjecutarInstalacionOEspera.
 */
Action ComportamientoTecnico::GiroHacia(Orientacion actual, Orientacion objetivo) {
    if (actual == objetivo) return IDLE;
    int girosD = (objetivo - actual + 8) % 8;
    int girosI = (actual - objetivo + 8) % 8;
    return (girosD <= girosI) ? TURN_SR : TURN_SL;
}

/**
 * @brief Procesa la llamada COME del ingeniero.
 *
 * CAMBIOS respecto al código original:
 * - Se resetea ingExpF/ingExpC al recibir un nuevo destino, para que no
 *   usemos una posición del ingeniero que pertenece al tramo anterior.
 */
void ComportamientoTecnico::ProcesarLlamadaIngeniero(const Sensores &sensores) {
    if (sensores.venpaca) {
        destF = sensores.GotoF;
        destC = sensores.GotoC;
        tieneDestino = true;
        plan.clear();
        ingExpF = -1; // [CAMBIO] Resetear posición del ingeniero: nuevo tramo, nueva búsqueda
        ingExpC = -1;
    }
}

/**
 * @brief Navega hacia (destF, destC) con evasión de colisiones.
 *
 * CAMBIOS respecto al código original:
 * 1. En caso de colisión con el ingeniero, en vez de inyectar 'M' y
 *    re-lanzar A* en el mismo tick, simplemente limpiamos el plan y
 *    devolvemos IDLE. El siguiente tick A* se recalcula solo con una
 *    llamada, desde la posición real del agente.
 *
 * 2. [NUEVO] Cuando detectamos al ingeniero 1 paso al frente (agentes[2]=='i'),
 *    guardamos su posición absoluta en ingExpF/ingExpC.
 *    Así, cuando el técnico llegue a destF/destC, ya sabe dónde está el
 *    ingeniero y puede orientarse directamente con GiroHacia.
 */
Action ComportamientoTecnico::NavegacionSeguraTecnico(const Sensores &sensores) {
    // Rastrear al ingeniero si está 1 paso al frente durante la navegación.
    // Cuando lleguemos al destino, esta posición nos permitirá orientarnos
    // directamente en EjecutarInstalacionOEspera sin hacer TURN_SR ciego.
    if (sensores.agentes[2] == 'i') {
        ubicacion posIng = Delante({sensores.posF, sensores.posC, sensores.rumbo});
        ingExpF = posIng.f;
        ingExpC = posIng.c;
    }

    if (plan.empty()) {
        EstadoT ini, dest;
        ini.site       = {sensores.posF, sensores.posC, sensores.rumbo};
        ini.zapatillas = tiene_zapatillas;
        dest.site      = {destF, destC, norte};
        dest.zapatillas = false;
        plan = A_Estrella(ini, dest, mapaResultado, mapaCotas);
    }

    if (!plan.empty()) {
        Action sig = plan.front();

        if (RiesgoChoqueIngeniero(sensores, sig)) {
            // [CAMBIO] Espera 1 tick en vez de A* con marcaje 'M'.
            // El ingeniero se moverá y el plan se recalculará desde la
            // posición actualizada en el siguiente tick.
            plan.clear();
            return IDLE;
        }

        plan.pop_front();
        return sig;
    }

    return IDLE;
}

/**
 * @brief Ejecuta la instalación cuando el técnico ya está en destF/destC.
 *
 * CAMBIOS respecto al código original:
 * - En vez de TURN_SR ciego (hasta 7 ticks para encontrar al ingeniero),
 *   usamos ingExpF/ingExpC (registrado durante la navegación) para
 *   orientarnos directamente con GiroHacia: máximo 3 giros en lugar de 7.
 * - Si ingExpF == -1 (el ingeniero nunca estuvo en campo de visión frontal),
 *   se mantiene el fallback de TURN_SR como garantía de funcionamiento.
 */
Action ComportamientoTecnico::EjecutarInstalacionOEspera(const Sensores &sensores) {
    plan.clear(); // Limpiar plan residual por seguridad

    // ① Ingeniero justo enfrente: INSTALL inmediato
    if (sensores.enfrente && sensores.agentes[2] == 'i') {
        tieneDestino = false;
        ingExpF = -1;
        ingExpC = -1;
        return INSTALL;
    }

    // ② Actualizar posición del ingeniero si ahora está 1 paso al frente
    if (sensores.agentes[2] == 'i') {
        ubicacion posIng = Delante({sensores.posF, sensores.posC, sensores.rumbo});
        ingExpF = posIng.f;
        ingExpC = posIng.c;
    }

    // ③ Si conocemos la posición del ingeniero, orientarse directamente
    //    (GiroHacia elige el camino más corto: máx 3 giros vs 7 del spin)
    if (ingExpF != -1 && ingExpC != -1) {
        Orientacion orientDeseada = OrientacionHacia(
            sensores.posF, sensores.posC, ingExpF, ingExpC);
        if (sensores.rumbo != orientDeseada) {
            return GiroHacia(sensores.rumbo, orientDeseada);
        }
        // Orientados pero el ingeniero aún no detectado: esperar 1 tick
        return IDLE;
    }

    // ④ Fallback: spin clásico si el ingeniero no fue detectado durante la
    //    navegación (caso poco probable cuando el mapa es pequeño/medio)
    return TURN_SR;
}

/**
 * @brief Comportamiento del técnico para el Nivel 5.
 * La estructura general no cambia; los cambios están en las funciones
 * auxiliares llamadas desde aquí.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_5(Sensores sensores) {
    // 1. Siempre verificar si el ingeniero requiere nuestra presencia
    ProcesarLlamadaIngeniero(sensores);

    // 2. Sin tarea asignada: esperar
    if (!tieneDestino) return IDLE;

    // 3. Ya llegamos al destino: instalar o esperar al ingeniero
    if (sensores.posF == destF && sensores.posC == destC) {
        return EjecutarInstalacionOEspera(sensores);
    }

    // 4. En camino: navegar hacia el destino
    return NavegacionSeguraTecnico(sensores);
}

Action ComportamientoTecnico::ComportamientoTecnicoNivel_6(Sensores sensores) {
  return IDLE;
}


// =========================================================================
// FUNCIONES PROPORCIONADAS (sin cambios)
// =========================================================================

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

bool ComportamientoTecnico::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas) {
  if (f < 0 || f >= (int)mapaResultado.size() || c < 0 || c >= (int)mapaResultado[0].size()) return false;
  return es_camino(mapaResultado[f][c]);
}

bool ComportamientoTecnico::EsAccesiblePorAltura(const ubicacion &actual) {
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= (int)mapaCotas.size() || del.c < 0 || del.c >= (int)mapaCotas[0].size()) return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (desnivel > 1) return false;
  return true;
}

ubicacion ComportamientoTecnico::Delante(const ubicacion &actual) const {
  ubicacion delante = actual;
  switch (actual.brujula) {
    case 0: delante.f--;             break; // norte
    case 1: delante.f--; delante.c++; break; // noreste
    case 2: delante.c++;             break; // este
    case 3: delante.f++; delante.c++; break; // sureste
    case 4: delante.f++;             break; // sur
    case 5: delante.f++; delante.c--; break; // suroeste
    case 6: delante.c--;             break; // oeste
    case 7: delante.f--; delante.c--; break; // noroeste
  }
  return delante;
}

void ComportamientoTecnico::PintaPlan(const list<Action> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    if (*it == WALK)        cout << "W ";
    else if (*it == JUMP)   cout << "J ";
    else if (*it == TURN_SR) cout << "r ";
    else if (*it == TURN_SL) cout << "l ";
    else if (*it == COME)   cout << "C ";
    else if (*it == IDLE)   cout << "I ";
    else                    cout << "-_ ";
    it++;
  }
  cout << "( longitud " << plan.size() << ")" << endl;
}

void ComportamientoTecnico::VisualizaPlan(const ubicacion &st, const list<Action> &plan)
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
      case 0: cst.f--;             break;
      case 1: cst.f--; cst.c++;   break;
      case 2: cst.c++;             break;
      case 3: cst.f++; cst.c++;   break;
      case 4: cst.f++;             break;
      case 5: cst.f++; cst.c--;   break;
      case 6: cst.c--;             break;
      case 7: cst.f--; cst.c--;   break;
      }
      if (cst.f >= 0 && cst.f < (int)mapaResultado.size() &&
          cst.c >= 0 && cst.c < (int)mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, JUMP});
    case WALK:
      switch (cst.brujula)
      {
      case 0: cst.f--;             break;
      case 1: cst.f--; cst.c++;   break;
      case 2: cst.c++;             break;
      case 3: cst.f++; cst.c++;   break;
      case 4: cst.f++;             break;
      case 5: cst.f++; cst.c--;   break;
      case 6: cst.c--;             break;
      case 7: cst.f--; cst.c--;   break;
      }
      if (cst.f >= 0 && cst.f < (int)mapaResultado.size() &&
          cst.c >= 0 && cst.c < (int)mapaResultado[0].size())
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
