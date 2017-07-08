#include "../Comportamientos_Jugador/jugador.hpp"
#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
using namespace std;

Action ComportamientoJugador::think(Sensores sensores){

	Action accion = actIDLE;
	int index;

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Impresión de estado en la terminal:
	// Descomentar para más información.
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	cout << "Terreno: ";
	for (int i=0; i<sensores.terreno.size(); i++)
		cout << sensores.terreno[i];
	cout << endl;
	cout << "Superficie: ";
	for (int i=0; i<sensores.superficie.size(); i++)
		cout << sensores.superficie[i];
	cout << endl;
	cout << "Colisión: " << sensores.colision << endl;
	cout << "Mochila: " << sensores.mochila << endl;
	cout << "Reset: " << sensores.reset << endl;
	cout << "Vida: " << sensores.vida << endl;
	cout << "Fila: " << sensores.mensajeF << endl;
	cout << "Columna: " << sensores.mensajeC << endl;
	cout << "objetoActivo: " << sensores.objetoActivo << endl;
	cout << "Objetos en Mochila: ";
	for(int i = 0; i<5; ++i) cout << objetos[i] << " ";
	cout << endl;
	cout << "Deliberar en: " << deliberar_en << endl;
	cout << "Deliberando: " << deliberando << endl;
	cout << "Error plan: " << error_plan << endl;
	cout << "Regalos sin recoger: " << sensores.regalos.size() << endl;
	cout << endl;
	*/
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Comprobar si se ha reiniciado el juego:
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	if(sensores.reset)
		restart();

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Actualizar posición:
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	switch (ultimaAccion) {
		case actFORWARD:
			if(!sensores.colision){
				if(brujula == 0) fil--; //N
				else if(brujula == 1) col++; //E
				else if(brujula == 2) fil++; //S
				else col--; //O
			}
			break;
		case actTURN_L:
			brujula = (brujula+3)%4;
			if (rand()%2==0) girar_derecha = true;
			else girar_derecha = false;
			break;
		case actTURN_R:
			brujula = (brujula+1)%4;
			if (rand()%2==0) girar_derecha = true;
			else girar_derecha = false;
			break;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Actualizar memoria: Mapas y reyes:
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	AniadeAlMap(sensores);
	if (sensores.terreno[0]=='K' && !bien_situado){
		vfil = fil;
		vcol = col;
		fil = sensores.mensajeF;
		col= sensores.mensajeC;
		MergeMap();
		bien_situado = true;
	}

	//Si hay un rey cerca, guardo su posición:
	if(bien_situado)
		guardaRey(sensores);

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Elaboración y comprobación de planes Deliberativos: Planificación.
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Si el plan ha fallado dejo de deliberar:
	if(error_plan){
		deliberar_en = 90;
		deliberando = false;
	}

	//Comprobar si el plan va a fallar:
	if(deliberando && plan.front() == actFORWARD && ObstaculoDelante(sensores.terreno, sensores.superficie)){
		if(sensores.superficie[2] == 'a' || sensores.superficie[2] == 'l'){
			plan.push_front(actIDLE);
			error_plan = false;
		}
		else if(sensores.superficie[2] == '0' || sensores.superficie[2] == '1' || sensores.superficie[2] == '2' || sensores.superficie[2] == '3' || sensores.superficie[2] == '4'){
			pair<bool,int> puedo = PuedoRecoger(sensores.superficie[2]);
			if(puedo.first && sensores.terreno[0] != 'A' && sensores.terreno[0] != 'B')
				error_plan = false;
			else
				error_plan = true;
		}
		else if(sensores.terreno[2] == 'A'  && objetos[1] && sensores.terreno[0] != 'B')
			error_plan = false;
		else if(sensores.terreno[2] == 'B'  && objetos[2] && sensores.terreno[0] != 'A')
			error_plan = false;
		else if(sensores.terreno[2] == 'D' && sensores.superficie[2] == 'd' && objetos[3] && sensores.terreno[0] != 'A' && sensores.terreno[0] != 'B')
			error_plan = false;
		else
			error_plan = true;
	}
	else
		error_plan = false;

	// Fijar plan para ir hacia un punto PK:
	if(!bien_situado && !deliberando){
		bool continua = true;
		for (int i = 0; i < sensores.terreno.size() && continua; i++) {
			if(sensores.terreno[i] == 'K'){
				deliberando = true;
				pathFinding_simple(i, plan);
				continua = false;
			}
		}
	}

	//Fijar plan para ir a por un regalo:
	if(bien_situado && !objetos[4] && !deliberando && deliberar_en <= 0 && sensores.regalos.size()>0){
		estado origen;
		origen.fila = fil;
		origen.columna = col;
		origen.orientacion = brujula;
		estado destino;
		destino.fila = sensores.regalos[0].first;
		destino.columna = sensores.regalos[0].second;
		deliberando = pathFinding(origen, destino, plan);
	}

	//Fijar un plan para ir hasta el rey si tengo regalo:
	if(bien_situado && objetos[4] && !deliberando && deliberar_en <= 0 && reyes.size()>0){
		//Buscar el rey más cercano:
		int pos_min = 0;
		int distancia = abs(fil - reyes[0].first) + abs(col - reyes[0].second);
		for(int i = 1; i<reyes.size(); ++i)
		 	if(abs(fil - reyes[i].first) + abs(col - reyes[i].second) < distancia){
				distancia = abs(fil - reyes[i].first) + abs(col - reyes[i].second);
				pos_min = i;
			}
		//Planificar:
		estado origen;
		origen.fila = fil;
		origen.columna = col;
		origen.orientacion = brujula;
		estado destino;
		destino.fila = reyes[pos_min].first;
		destino.columna = reyes[pos_min].second;
		deliberando = pathFinding(origen, destino, plan);
		if(!plan.empty())
			plan.pop_back(); //Eliminar el último avance, ya que no puede situarse sobre el rey.
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Acciones que NO implican movimiento:
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Nos aseguramos que el hueso y el regalo han sido entregado con éxito:
	if(ultimaAccion == actGIVE){
		if(objetoDado == 0 && sensores.objetoActivo == '_')
			objetos[0] = false;
		else if(objetoDado == 4 && sensores.objetoActivo == '_')
			objetos[4] = false;
	}
	else if(ultimaAccion == actPICKUP){
		accion = actPUSH; // Si recogí un objeto lo guardo.
	}
	else if(sensores.superficie[2] == '0' || sensores.superficie[2] == '1' || sensores.superficie[2] == '2' || sensores.superficie[2] == '3' || sensores.superficie[2] == '4'){
		//Si delante tengo un objeto.
		pair<bool,int> puedo_recoger = PuedoRecoger(sensores.superficie[2]);
		if(puedo_recoger.first && sensores.objetoActivo == '_'){ ////Si puedo recoger ese objeto y no tengo nada en la mano.
				accion = actPICKUP; //Recojo el objeto.
				objetos[puedo_recoger.second] = true;
		}
		else if(puedo_recoger.first && sensores.objetoActivo != '_'  && sensores.terreno[0] != 'A' && sensores.terreno[0] != 'B')
			accion = actPUSH; //Tengo algo en la mano. Guardo en la mochila para coger el objeto.
		else{ //Ya tengo un objeto de ese tipo o no puedo cogerlo. Lo evito:
			if(!girar_derecha)
				accion = actTURN_L;
			else
				accion = actTURN_R;
		}
	}
	else if(ultimaAccion == actPOP){
		if(sensores.objetoActivo != objetoNecesario) //Si saqué un objeto de la mochila y no es lo que busco.
			accion = actPUSH; //Vuelvo a guardarlo.
		else
			accion = actIDLE;
	}
	else if(sensores.terreno[2] == 'A'  && objetos[1] && sensores.objetoActivo != '1' && sensores.terreno[0] != 'B'){
		if(sensores.objetoActivo == '_')
			accion = actPOP;	//Si hay agua y tengo el bikini en la mochila, lo busco.
		else
			accion = actPUSH; //Si tengo algo en la mano, guardo primero para sacar otra cosa.
		objetoNecesario = '1';
	}
	else if(sensores.terreno[2] == 'B'  && objetos[2] && sensores.objetoActivo != '2' && sensores.terreno[0] != 'A'){
		if(sensores.objetoActivo == '_')
			accion = actPOP;	//Si hay bosque y tengo las zapatillas en la mochila, las busco.
		else
			accion = actPUSH; //Si tengo algo en la mano, guardo primero para sacar otra cosa.
		objetoNecesario = '2';
	}
	else if(sensores.terreno[2] == 'D' && sensores.superficie[2] == 'd' && sensores.objetoActivo == '3'){
		accion = actGIVE; //Abro la puerta.
	}
	else if(sensores.terreno[2] == 'D' && sensores.superficie[2] == 'd' && objetos[3] && sensores.objetoActivo != '3' && sensores.terreno[0] != 'A' && sensores.terreno[0] != 'B'){
		if(sensores.objetoActivo == '_')
			accion = actPOP;	//Si tengo una puerta cerrada delante y tengo las llaves, las busco.
		else
			accion = actPUSH; //Si tengo algo en la mano, guardo primero para sacar otra cosa.
		objetoNecesario = '3';
	}
	else if(sensores.superficie[2] == 'l' && sensores.objetoActivo == '0'){
		accion = actGIVE; //Si tengo un lobo delante y un hueso en la mano le doy el hueso.
		objetoDado = 0;
	}
	else if(sensores.superficie[2] == 'l' && objetos[0] && sensores.objetoActivo != '0' && sensores.terreno[0] != 'A' && sensores.terreno[0] != 'B'){
		if(sensores.objetoActivo == '_')
			accion = actPOP;	//Si tengo delante un lobo y tengo un hueso en la mochila lo busco.
		else
			accion = actPUSH; //Si tengo algo en la mano, guardo primero para sacar otra cosa.
		objetoNecesario = '0';
	}
	else if(sensores.superficie[2] == 'r' && sensores.objetoActivo == '4'){
		accion = actGIVE; //Si tengo un rey delante y un regalo en la mano le doy el regalo.
		objetoDado = 4;
	}
	else if(sensores.superficie[2] == 'r' && objetos[4] && sensores.objetoActivo != '4' && sensores.terreno[0] != 'A' && sensores.terreno[0] != 'B'){
		if(sensores.objetoActivo == '_')
			accion = actPOP;	//Si tengo delante un rey y tengo un regalo en la mochila lo busco.
		else
			accion = actPUSH; //Si tengo algo en la mano, guardo primero para sacar otra cosa.
		objetoNecesario = '4';
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Acciones que implican movimiento:
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	else if(deliberando && !error_plan){
		accion = plan.front();
		if(!plan.empty())
	 		plan.erase(plan.begin());
		//ImprimePlan(plan); //Descomentar para imprimir el plan
		if(plan.size() == 0)
			deliberando = false;
	}
	else if((sensores.terreno[2] == 'T' ||  sensores.terreno[2] == 'S' || sensores.terreno[2] == 'K' || sensores.terreno[2] == 'D') && sensores.superficie[2] == '_'){
		if(girar_en >0 ){accion = actFORWARD; girar_en--;}
		else if(rand()%2==0){ accion = actTURN_L; girar_en = 61;}
		else{ accion = actTURN_R; girar_en = 61;}
	}
	else if(sensores.terreno[2] == 'A' && sensores.superficie[2] == '_'  && sensores.objetoActivo == '1'){
		//Si hay agua y tengo el bikini equipado:
		if(girar_en >0 ){accion = actFORWARD; girar_en--;}
		else if(rand()%2==0){ accion = actTURN_L; girar_en = 61;}
		else{ accion = actTURN_R; girar_en = 61;}
	}
	else if(sensores.terreno[2] == 'B' && sensores.superficie[2] == '_'  && sensores.objetoActivo == '2'){
		//Si hay bosque y tengo las zapatillas equipadas:
		if(girar_en >0 ){accion = actFORWARD; girar_en--;}
		else if(rand()%2==0){ accion = actTURN_L; girar_en = 61;}
		else{ accion = actTURN_R; girar_en = 61;}
	}
	else if(!girar_derecha)
		accion = actTURN_L;
	else
		accion = actTURN_R;

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Ajustes finales:
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Disminuir el contador para comenzar a deliberar:
	if(!deliberando)
		deliberar_en--;

	//Return
	ultimaAccion = accion;
	return accion;
}

void ComportamientoJugador::restart(){
	girar_en = 61;
	deliberando = false;
	error_plan = false;
	deliberar_en = 0;
	objetoDado = -1;
	fil = 99;
	col = 99;
	brujula = 0;
	ultimaAccion = actIDLE;
	girar_derecha = false;
	bien_situado = false;
	for(int i = 0; i < 200 ; ++i)
		for(int j = 0; j < 200 ; ++j)
			memoriaTemp[i][j] = '?';

	for(int i = 0; i < 5; ++i)
		objetos[i] = false;

	objetoNecesario = '5';
}

void ComportamientoJugador::MergeMap(){
	int dfil = fil - vfil;
	int dcol = col - vcol;
	for(int i = 0; i < 200 ; ++i)
		for(int j = 0; j < 200 ; ++j)
			if(memoriaTemp[i][j] != '?')
				mapaResultado[i+dfil][j+dcol] = memoriaTemp[i][j];
}

void ComportamientoJugador::AniadeAlMap(Sensores sensores){
	int k = 0, z = 0;
	switch(brujula){
		case 0:
			for(int i = fil; i>=fil-3; --i){
				for(int j = -k; j <= k; ++j){
					if(bien_situado)
						mapaResultado[i][col+j]=sensores.terreno[z++];
					else
						memoriaTemp[i][col+j] = sensores.terreno[z++];
				}
				k++;
			} break;
		case 2:
			for(int i = fil; i<=fil+3; ++i){
				for(int j = k; j >= -k; --j){
					if(bien_situado)
						mapaResultado[i][col+j]=sensores.terreno[z++];
					else
						memoriaTemp[i][col+j] = sensores.terreno[z++];
				}
				k++;
			} break;
		case 1:
			for(int i = col; i<=col+3; ++i){
				for(int j = -k; j <= k; ++j){
					if(bien_situado)
						mapaResultado[fil+j][i]=sensores.terreno[z++];
					else
						memoriaTemp[fil+j][i] = sensores.terreno[z++];
				}
				k++;
			} break;
		case 3:
			for(int i = col; i>=col-3; --i){
				for(int j = k; j >= -k; --j){
					if(bien_situado)
						mapaResultado[fil+j][i]=sensores.terreno[z++];
					else
						memoriaTemp[fil+j][i] = sensores.terreno[z++];
				}
				k++;
			} break;
		}
}

pair<bool,int> ComportamientoJugador::PuedoRecoger(unsigned char obj){
	pair<bool,int> sal;
	switch (obj) {
		case '0': sal.second = 0; break;
		case '1': sal.second = 1; break;
		case '2': sal.second = 2; break;
		case '3':	sal.second = 3; break;
		case '4': sal.second = 4; break;
	}
	//Las llaves y los huesos se consideran objetos menos importantes, por eso solo guarda uno de los dos.
	if(sal.second == 0 || sal.second == 3){
		sal.first = (!objetos[0] * !objetos[3]);
		return sal;
	}
	else{
		sal.first = !objetos[sal.second];
		return sal;
	}
}

void ComportamientoJugador::pathFinding_simple(int pos, list<Action> &plan){
	int num_avances;

	plan.clear();
	if(1 <= pos && pos <= 3)
		num_avances = 1;
	else if (4 <= pos && pos <= 8)
		num_avances = 2;
	else
		num_avances = 3;

	for(int i = 0; i < num_avances; ++i)
		plan.push_back(actFORWARD);

	if(pos == 1 || pos == 4 || pos == 5 || pos == 9 || pos == 10 || pos == 11)
		plan.push_back(actTURN_L);
	else if(pos != 2 && pos != 6 && pos != 12)
		plan.push_back(actTURN_R);

	num_avances = 0;
	if(pos == 1 || pos == 3 || pos == 5 || pos == 7 || pos == 11 || pos == 13)
		num_avances = 1;
	else if(pos == 4 || pos == 8 || pos == 10 || pos == 14)
		num_avances = 2;
	else if(pos == 9 || pos == 15)
		num_avances = 3;

	for(int i = 0; i < num_avances; ++i)
		plan.push_back(actFORWARD);
}

int ComportamientoJugador::calculaHeuristica(const nodo& nodo, const estado &destino){

	int difF = nodo.status.fila - destino.fila;
	int difC = nodo.status.columna - destino.columna;

	//La heurística tiene en cuenta la distancia en términos de filas y columnas:
	int heuristica = abs(difF) + abs(difC);

	//También tiene en cuenta la orientación:
	if(difC < 0){ //Tengo que ir al Este, si no apunta al este aumento su heuristica:
		//Caso especial: el norte está un poco más cerca del este: hacemos que este tenga
		//mejor heuristica para evitar mínimos locales. Se repite en los demás casos
		if(nodo.status.orientacion == 0) heuristica++;
		else if(nodo.status.orientacion != 1) heuristica += 2;
	}
	else if(difC > 0){ //Tengo que ir al oeste:
		if(nodo.status.orientacion == 0) heuristica++;
		else if(nodo.status.orientacion != 3) heuristica += 2;
	}

	if(difF < 0){ //Tengo que ir al sur:
		if(nodo.status.orientacion == 1) heuristica++;
		else if(nodo.status.orientacion != 2) heuristica += 2;
	}
	else if(difF > 0){ //Tengo que ir al norte
		if(nodo.status.orientacion == 1) heuristica++;
		else if(nodo.status.orientacion != 0) heuristica += 2;

	}
	return heuristica;
}

bool ComportamientoJugador::pathFinding(const estado &origen, const estado &destino, list<Action> &plan){
	//Borro la lista
	plan.clear();

	int mejor_h, index_mh;
	vector<nodo> hijos;

	nodo act;
	act.status = origen;
	act.heuristica = calculaHeuristica(act, destino);

	//Como mucho explorará 200 niveles del árbol:
	for(int j = 0; j<200; ++j){
		//Obtener hijos del nodo:
		for(int i = 0; i<3; ++i)
			hijos.push_back(expandeNodo(act, i, destino));

		//Obtener el nodo hijo con mejor heurística:
		mejor_h = hijos[0].heuristica;
		index_mh = 0;
		for(int i = 1; i<3; ++i){
			if(hijos[i].heuristica < mejor_h){
				index_mh = i;
				mejor_h = hijos[i].heuristica;
			}
		}
		//Comprobar si mejora al padre:
		if(hijos[index_mh].heuristica < act.heuristica){
			act = hijos[index_mh];
			hijos.clear();
		}
		else{
			plan = act.acciones;
			return true;
		}
	}
	return false;
}

ComportamientoJugador::nodo ComportamientoJugador::expandeNodo(nodo node, int action, const estado &destino){
	//Actualizar estado del nodo:
	switch (action) {
		case 0: //actFORWARD
			if(node.status.orientacion == 0) node.status.fila--; //N
			else if(node.status.orientacion == 1) node.status.columna++; //E
			else if(node.status.orientacion == 2) node.status.fila++; //S
			else node.status.columna--; //O
			node.acciones.push_back(actFORWARD);
		break;
		case 1: //actTURN_R
			node.status.orientacion = (node.status.orientacion+1)%4;
			node.acciones.push_back(actTURN_R);
		break;
		case 2: //actTURN_L
			node.status.orientacion = (node.status.orientacion+3)%4;
			node.acciones.push_back(actTURN_L);
		break;
	}

	//Darle heuristica:
	node.heuristica = calculaHeuristica(node, destino);
	return node;
}

bool ComportamientoJugador::ObstaculoDelante(const vector<unsigned char> & terreno, const vector<unsigned char> & superficie){
	if((terreno[2] == 'T' || terreno[2] == 'S' || terreno[2] == 'K') && superficie[2] == '_')
		return false;

	return true;
}

void ComportamientoJugador::ImprimePlan(list<Action> plan){
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A";
		}
		else if (*it == actTURN_R){
			cout << "D";
		}
		else if (*it == actTURN_L){
			cout << "I";
		}
		else {
			cout << "-";
		}
		it++;
	}
	cout << endl;
}

void ComportamientoJugador::guardaRey(Sensores sensores){
	int pos, avances = 0, desplazamientos = 0;
	bool izq = false;

	//Ver si hay un rey a la vista:
	bool hay_rey = false;
	for (int i = 0; i < sensores.terreno.size() && !hay_rey; i++)
		if(sensores.superficie[i] == 'r'){
			hay_rey = true;
			pos = i;
		}

	//Calcular su posición en función de donde lo hemos visto:
	if(hay_rey){
		int f_r = fil, c_r = col;

		if(1 <= pos && pos <= 3)
			avances = 1;
		else if (4 <= pos && pos <= 8)
			avances = 2;
		else
			avances = 3;

		if(pos == 1 || pos == 3 || pos == 5 || pos == 7 || pos == 11 || pos == 13)
			desplazamientos = 1;
		else if(pos == 4 || pos == 8 || pos == 10 || pos == 14)
			desplazamientos = 2;
		else if(pos == 9 || pos == 15)
			desplazamientos = 3;

		if(pos == 1 || (4 <= pos && pos <=5) || (9 <= pos && pos <= 11))
			izq = true;

		for(int i=0; i<avances; ++i){
			if(brujula == 0) f_r--;
			else if(brujula == 1) c_r++;
			else if(brujula == 2) f_r++;
			else c_r--;
		}

		for(int i=0; i<desplazamientos; ++i){
			if(brujula == 0){ izq? c_r-- : c_r++; }
			else if(brujula == 1){ izq? f_r-- : f_r++; }
			else if(brujula == 2){ izq? c_r++ : c_r--; }
			else if(brujula == 3){ izq? f_r++ : f_r--; }
		}

		pair<int, int> pos_r;
		pos_r.first = f_r;
		pos_r.second = c_r;

		//Buscar si el rey ya está en el vector:
		bool encontrado = false;
		for(int i = 0; i<reyes.size() && !encontrado; ++i)
			if(reyes[i] == pos_r) encontrado = true;

		if(!encontrado)
			reyes.push_back(pos_r);
	}
}

int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
