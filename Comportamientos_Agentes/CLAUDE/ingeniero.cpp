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


char ViablePorAltura0I (char casilla, int dif, bool zap) {
  if (abs(dif) <= 1 or (zap and abs(dif) <=2)){
    return casilla;
  } else {
    return 'P';
  }
}

int VeoCasillaPulgarcitoNivel0I (char i, char c, char d, int t_i, int t_c, int t_d, bool zap) {
    if (c == 'U') return 2;
    else if (i == 'U') return 1;
    else if (d == 'U') return 3;

    if (!zap) {
        if (c == 'D') return 2;
        else if (i == 'D') return 1;
        else if (d == 'D') return 3;
    }

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

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_0(Sensores sensores)
{
  Action accion = IDLE;
  
  ActualizarMapa(sensores);
  
  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  if(sensores.superficie[0]=='U')  return IDLE;

  char i = ViablePorAltura0I (sensores.superficie[1], sensores.cota[1]-sensores.cota[0], tiene_zapatillas); 
  char c = ViablePorAltura0I (sensores.superficie[2], sensores.cota[2]-sensores.cota[0], tiene_zapatillas);
  char d = ViablePorAltura0I (sensores.superficie[3], sensores.cota[3]-sensores.cota[0], tiene_zapatillas);
  
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

  int pos = VeoCasillaPulgarcitoNivel0I(i, c, d, t_i, t_c, t_d, tiene_zapatillas);

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

bool ComportamientoIngeniero::es_camino(unsigned char c) const
{
  return (c == 'C' || c == 'D' || c == 'U');
}

char ViablePorAltura1I (char casilla, int dif, bool zap) {
  if (casilla == 'M' || casilla == 'A' || casilla == 'P' || casilla == 'B') return 'P'; 
  if (abs(dif) <= 1 || (zap && abs(dif) <= 2)){
    return casilla;
  } else {
    return 'P';
  }
}

int VeoCasillaPulgarcitoNivel1I (char i, char c, char d, int t_i, int t_c, int t_d, bool zap) {
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

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_1(Sensores sensores)
{
  Action accion = IDLE;

  ActualizarMapa(sensores);

  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  char i = ViablePorAltura1I (sensores.superficie[1], sensores.cota[1]-sensores.cota[0], tiene_zapatillas);
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

  int t_i = (posI.f >= 0 && posI.f < (int)mtiempo.size() && posI.c >= 0 && posI.c < (int)mtiempo[0].size()) ? mtiempo[posI.f][posI.c] : 999999;
  int t_c = (posC.f >= 0 && posC.f < (int)mtiempo.size() && posC.c >= 0 && posC.c < (int)mtiempo[0].size()) ? mtiempo[posC.f][posC.c] : 999999;
  int t_d = (posD.f >= 0 && posD.f < (int)mtiempo.size() && posD.c >= 0 && posD.c < (int)mtiempo[0].size()) ? mtiempo[posD.f][posD.c] : 999999;

  int pos = VeoCasillaPulgarcitoNivel1I(i, c, d, t_i, t_c, t_d, tiene_zapatillas);

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

bool ComportamientoIngeniero::esSuperficieValida(unsigned char superficie) const {
  if (superficie == 'M' || superficie == 'P' || superficie == 'B') {
    return false;
  } else {
    return true;
  }
}

bool ComportamientoIngeniero::CasillaAccesibleIngeniero(Action accion, const EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  if (accion!=WALK && accion!=JUMP) return true;

  ubicacion casilla_delante = Delante(st.site);

  if (accion == WALK) {
    if (casilla_delante.f < 0 || casilla_delante.f >= (int)terreno.size() || casilla_delante.c < 0 || casilla_delante.c >= (int)terreno[0].size()) return false;
    if (!esSuperficieValida(terreno[casilla_delante.f][casilla_delante.c])) return false;

    int dif_altura = altura[casilla_delante.f][casilla_delante.c] - altura[st.site.f][st.site.c];
    return (abs(dif_altura) <= 1 || (st.zapatillas && abs(dif_altura) <= 2));
  }

  else if (accion == JUMP) {
    ubicacion casilla_jump = Delante(casilla_delante);

    if (casilla_delante.f < 0 || casilla_delante.f >= (int)terreno.size() || casilla_delante.c < 0 || casilla_delante.c >= (int)terreno[0].size()) return false;
    if (!esSuperficieValida(terreno[casilla_delante.f][casilla_delante.c])) return false;

    if (casilla_jump.f < 0 || casilla_jump.f >= (int)terreno.size() || casilla_jump.c < 0 || casilla_jump.c >= (int)terreno[0].size()) return false;
    if (!esSuperficieValida(terreno[casilla_jump.f][casilla_jump.c])) return false;
    
    int dif_altura = altura[casilla_jump.f][casilla_jump.c] - altura[st.site.f][st.site.c];
    return (abs(dif_altura) <= 1 || (st.zapatillas && abs(dif_altura) <= 2));
  }
  return false;
}

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
  if (terreno[nuevo_st.site.f][nuevo_st.site.c] == 'D') nuevo_st.zapatillas = true;
  return nuevo_st;
}

list<Action> ComportamientoIngeniero::B_Anchura(const EstadoI &inicio, const EstadoI &final, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura) {
  NodoI nodo_actual;
  list<NodoI> frontier;
  list<Action> camino_solucion;

  vector<vector<vector<vector<bool>>>> visitados(terreno.size(),
      vector<vector<vector<bool>>>(terreno[0].size(),
      vector<vector<bool>>(8,
      vector<bool>(2, false))));
  
  nodo_actual.estado = inicio;
  frontier.push_back(nodo_actual);

  bool solucion_encontrada = (nodo_actual.estado.site.f == final.site.f and nodo_actual.estado.site.c == final.site.c);

  while (!solucion_encontrada and !frontier.empty()){
    nodo_actual = frontier.front();
    frontier.pop_front();

    int zapatillas = nodo_actual.estado.zapatillas ? 1 : 0;
    if (visitados[nodo_actual.estado.site.f][nodo_actual.estado.site.c][nodo_actual.estado.site.brujula][zapatillas]) continue;
    visitados[nodo_actual.estado.site.f][nodo_actual.estado.site.c][nodo_actual.estado.site.brujula][zapatillas] = true;

    if (nodo_actual.estado.site.f == final.site.f && nodo_actual.estado.site.c == final.site.c) {
      solucion_encontrada = true;
      camino_solucion = nodo_actual.secuencia;
    break;
    }

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

bool ComportamientoIngeniero::RiesgoChoqueTecnico(const Sensores &sensores, Action accion) {
    switch (accion) {
        case WALK:
            return (sensores.agentes[2] == 't');
        case JUMP:
            return (sensores.agentes[2] == 't' || sensores.agentes[6] == 't');
        default:
            return false;
    }
}

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_2(Sensores sensores)
{
  Action accion = IDLE;

  if (!hayPlan) {
      EstadoI inicio, destino;
      inicio.site.f = sensores.posF;
      inicio.site.c = sensores.posC;
      inicio.site.brujula = sensores.rumbo;
      inicio.zapatillas = tiene_zapatillas;

      destino.site.f = sensores.BelPosF;
      destino.site.c = sensores.BelPosC;
      destino.site.brujula = norte;
      destino.zapatillas = false;

      plan = B_Anchura(inicio, destino, mapaResultado, mapaCotas);
      VisualizaPlan(inicio.site,plan);
      hayPlan = plan.size()!=0;
  }

  if (hayPlan && plan.size() > 0) {
    accion = plan.front();
    if(RiesgoChoqueTecnico(sensores, accion)){
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

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_3(Sensores sensores)
{
  return IDLE;
}

bool ComportamientoIngeniero::DeltaValido(int f, int c, int delta, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  unsigned char terr = terreno[f][c];
  int h = altura[f][c];
  
  if (terr == 'A' && delta != 0) return false;
  if (delta == 1 && h >= 9)     return false;
  if (delta == -1 && h <= 1)    return false;
  return true;
}

bool ComportamientoIngeniero::CasillaValidaTuberia(int f, int c, const vector<vector<unsigned char>> &terreno) {
  unsigned char t = terreno[f][c];
  if (t == 'M' || t == 'P' || t == 'B' || t == '?') return false;
  return true;
}

int ComportamientoIngeniero::getCosteEcoINSTALL(unsigned char terr) {
  if (terr == 'A') return 50;
  if (terr == 'H') return 45;
  if (terr == 'S') return 25;
  if (terr == 'C' || terr == 'U') return 15;
  return 30;
}

int ComportamientoIngeniero::getCosteEcoRAISE(unsigned char terr) {
  if (terr == 'H') return 55;
  if (terr == 'S') return 30;
  if (terr == 'C' || terr == 'U') return 10;
  return 40;
}

int ComportamientoIngeniero::getCosteEcoDIG(unsigned char terr) {
  if (terr == 'H') return 65;
  if (terr == 'S') return 40;
  if (terr == 'C' || terr == 'U') return 25;
  return 50;
}

list<Paso> ComportamientoIngeniero::PlanificarRedTuberias(int fInicio, int cInicio, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura, int limiteAmbiental){
priority_queue<NodoTuberia, vector<NodoTuberia>, ComparaTuberia> frontera;
vector<vector<vector<int>>> min_imp(
    terreno.size(),
    vector<vector<int>>(terreno[0].size(),
    vector<int>(3, INT_MAX)));

for (int delta = -1; delta <= 1; delta++) {
  if (DeltaValido(fInicio, cInicio, delta, terreno, altura)) {
    NodoTuberia inicio;
    inicio.f = fInicio;
    inicio.c = cInicio;
    inicio.delta = delta;
    inicio.longitud = 1;
    int imp = 0;
    if (delta == -1) imp = getCosteEcoDIG(terreno[fInicio][cInicio]);
    if (delta ==  1) imp = getCosteEcoRAISE(terreno[fInicio][cInicio]);
    inicio.impacto = imp;
    Paso p0{fInicio, cInicio, delta};
    inicio.camino.push_back(p0);
    frontera.push(inicio);
  }
}

while (!frontera.empty()) {
  NodoTuberia actual = frontera.top();
  frontera.pop();
  
  int &mi = min_imp[actual.f][actual.c][actual.delta + 1];
  if (mi <= actual.impacto) continue;

  if (terreno[actual.f][actual.c] == 'U') {
    if (actual.impacto <= limiteAmbiental) {
      return actual.camino;
    }
    continue;
  }

  mi = actual.impacto;

  int h_actual = altura[actual.f][actual.c] + actual.delta;

  const int df[] = {-1, 1, 0, 0};
  const int dc[] = { 0, 0,-1, 1};

  for (int dir = 0; dir < 4; dir++) {
    int nf = actual.f + df[dir];
    int nc = actual.c + dc[dir];

    if (!CasillaValidaTuberia(nf, nc, terreno)) continue;

    for (int dv = -1; dv <= 1; dv++) {
      if (!DeltaValido(nf, nc, dv, terreno, altura)) continue;

      int h_v = altura[nf][nc] + dv;
      int diff = h_actual - h_v;
      if (diff != 0 && diff != 1) continue;

      NodoTuberia hijo = actual;
      hijo.f = nf; hijo.c = nc; hijo.delta = dv;
      hijo.longitud++;

      unsigned char terr_actual = terreno[actual.f][actual.c];
      unsigned char terr_vecino = terreno[nf][nc];

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


// =========================================================================
// NIVEL 5 — HELPERS Y COMPORTAMIENTO PRINCIPAL
// =========================================================================

Orientacion ComportamientoIngeniero::OrientacionHacia(int f1, int c1, int f2, int c2) {
    if (f2 < f1) return norte;
    if (f2 > f1) return sur;
    if (c2 < c1) return oeste;
    return este;
}

Action ComportamientoIngeniero::GiroHacia(Orientacion actual, Orientacion objetivo) {
    if (actual == objetivo) return IDLE;
    int girosD = (objetivo - actual + 8) % 8;
    int girosI = (actual - objetivo + 8) % 8;
    return (girosD <= girosI) ? TURN_SR : TURN_SL;
}

/**
 * @brief [NUEVO] Navega hacia (destF, destC) reutilizando el plan existente.
 *
 * CAMBIOS respecto al código original:
 * - La lógica duplicada de las Fases 0 y 2 ahora vive aquí.
 * - En caso de colisión con el técnico, en vez de inyectar 'M' y
 *   re-lanzar B_Anchura en el mismo tick (muy caro en CPU), simplemente
 *   limpiamos el plan y devolvemos IDLE. El siguiente tick, si el técnico
 *   ya se movió, el plan se recalcula desde la nueva posición correcta.
 *   Un solo tick de espera es suficiente porque ambos agentes se mueven
 *   simultáneamente.
 */
Action ComportamientoIngeniero::NavegacionIngenieroHacia(int destF, int destC, const Sensores &sensores) {
    // Calcular plan solo cuando esté vacío (se reutiliza entre ticks)
    if (plan.empty()) {
        EstadoI ini, dest;
        ini.site       = {sensores.posF, sensores.posC, sensores.rumbo};
        ini.zapatillas = tiene_zapatillas;
        dest.site      = {destF, destC, norte};
        dest.zapatillas = false;
        plan = B_Anchura(ini, dest, mapaResultado, mapaCotas);
    }

    if (!plan.empty()) {
        Action sig = plan.front();

        if (RiesgoChoqueTecnico(sensores, sig)) {
            // Espera 1 tick. El técnico se moverá y el plan se recalculará
            // desde la posición actualizada en el siguiente tick.
            plan.clear();
            return IDLE;
        }

        plan.pop_front();
        return sig;
    }

    return IDLE;
}

/**
 * @brief Comportamiento del ingeniero para el Nivel 5.
 *
 * CAMBIOS respecto al código original:
 *
 * 1. La navegación duplicada de Fases 0 y 2 se ha extraído a
 *    NavegacionIngenieroHacia(), eliminando ~40 líneas repetidas.
 *
 * 2. Fase 0→1: se añade [[fallthrough]] al final de la Fase 0, de forma
 *    que al llegar a tramoTec no se desperdicia 1 tick devolviendo IDLE;
 *    en su lugar se entra directamente a la Fase 1 (orientación + COME).
 *
 * 3. Fase 2→3: ídem. Al llegar a tramoIng sin operación pendiente, se
 *    entra directamente a la Fase 3 en el mismo tick.
 *
 * 4. La colisión con el técnico ya no inyecta 'M' ni re-lanza B_Anchura
 *    en el mismo tick; ver NavegacionIngenieroHacia().
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores) {

  // ── PLANIFICACIÓN (solo se ejecuta una vez) ──────────────────────────────
  if (!hayPlan) {
    list<Paso> plan5 = PlanificarRedTuberias(
        sensores.BelPosF, sensores.BelPosC,
        mapaResultado, mapaCotas, sensores.max_ecologico);

    if (!plan5.empty()) {
      VisualizaRedTuberias(plan5);
      planVec    = vector<Paso>(plan5.begin(), plan5.end());
      installIdx = 1;
      opDone     = false;
      faseN5     = 0;
    }
    hayPlan = true;
    return IDLE; // Único IDLE obligatorio: esperar a que el motor procese la planificación
  }

  if (planVec.empty() || installIdx >= (int)planVec.size()) return IDLE;

  // Referencias al tramo actual
  const Paso &tramoTec = planVec[installIdx - 1]; // casilla donde irá el Técnico
  const Paso &tramoIng = planVec[installIdx];     // casilla donde irá el Ingeniero

  switch (faseN5) {

    // ── FASE 0: Ir a la casilla del Técnico (plan[k-1]) ──────────────────
    case 0: {
      if (sensores.posF != tramoTec.fil || sensores.posC != tramoTec.col) {
        return NavegacionIngenieroHacia(tramoTec.fil, tramoTec.col, sensores);
      }

      // Llegamos a tramoTec
      plan.clear();

      // RAISE/DIG del nodo origen (solo en el primer tramo)
      if (installIdx == 1 && tramoTec.op != 0 && !opDone) {
        opDone = true;
        return (tramoTec.op == 1) ? RAISE : DIG;
      }

      faseN5 = 1;
      [[fallthrough]]; // [CAMBIO] Evitar 1 IDLE: entrar a Fase 1 en este mismo tick
    }

    // ── FASE 1: Orientarse hacia tramoIng y enviar COME ──────────────────
    case 1: {
      opDone = false; // Reset para la evaluación de la cota de tramoIng en Fase 2

      Orientacion orientDeseada = OrientacionHacia(
          tramoTec.fil, tramoTec.col, tramoIng.fil, tramoIng.col);

      if (sensores.rumbo != orientDeseada) {
        return GiroHacia(sensores.rumbo, orientDeseada);
      }

      faseN5 = 2;
      return COME;
    }

    // ── FASE 2: Moverse a tramoIng y hacer RAISE/DIG ─────────────────────
    case 2: {
      if (sensores.posF != tramoIng.fil || sensores.posC != tramoIng.col) {
        return NavegacionIngenieroHacia(tramoIng.fil, tramoIng.col, sensores);
      }

      // Llegamos a tramoIng
      plan.clear();

      if (tramoIng.op != 0 && !opDone) {
        opDone = true;
        return (tramoIng.op == 1) ? RAISE : DIG;
      }

      faseN5 = 3;
      [[fallthrough]]; // [CAMBIO] Evitar 1 IDLE: entrar a Fase 3 en este mismo tick
    }

    // ── FASE 3: Girarse mirando hacia tramoTec ───────────────────────────
    case 3: {
      Orientacion orientDeseada = OrientacionHacia(
          tramoIng.fil, tramoIng.col, tramoTec.fil, tramoTec.col);

      if (sensores.rumbo != orientDeseada) {
        return GiroHacia(sensores.rumbo, orientDeseada);
      }

      faseN5 = 4;
      [[fallthrough]]; // Si ya estamos orientados, comprobar directamente si el técnico llegó
    }

    // ── FASE 4: Sincronizar INSTALL ───────────────────────────────────────
    case 4: {
      if (sensores.enfrente && sensores.agentes[2] == 't') {
        installIdx++;
        opDone = false;
        faseN5 = 0;
        return INSTALL;
      }
      return IDLE; // Esperar a que el técnico llegue a tramoTec
    }
  }

  return IDLE;
}

Action ComportamientoIngeniero::ComportamientoIngenieroNivel_6(Sensores sensores)
{
  return IDLE;
}


// =========================================================================
// FUNCIONES PROPORCIONADAS (sin cambios)
// =========================================================================

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

bool ComportamientoIngeniero::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas)
{
  if (f < 0 || f >= (int)mapaResultado.size() || c < 0 || c >= (int)mapaResultado[0].size())
    return false;
  return es_camino(mapaResultado[f][c]);
}

bool ComportamientoIngeniero::EsAccesiblePorAltura(const ubicacion &actual, bool zap)
{
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= (int)mapaCotas.size() || del.c < 0 || del.c >= (int)mapaCotas[0].size())
    return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (zap && desnivel > 2)  return false;
  if (!zap && desnivel > 1) return false;
  return true;
}

ubicacion ComportamientoIngeniero::Delante(const ubicacion &actual) const
{
  ubicacion delante = actual;
  switch (actual.brujula)
  {
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

void ComportamientoIngeniero::PintaPlan(const list<Action> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    if (*it == WALK)       cout << "W ";
    else if (*it == JUMP)  cout << "J ";
    else if (*it == TURN_SR) cout << "r ";
    else if (*it == TURN_SL) cout << "l ";
    else if (*it == COME)  cout << "C ";
    else if (*it == IDLE)  cout << "I ";
    else                   cout << "-_ ";
    it++;
  }
  cout << "( longitud " << plan.size() << ")" << endl;
}

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

void ComportamientoIngeniero::VisualizaPlan(const ubicacion &st, const list<Action> &plan)
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
