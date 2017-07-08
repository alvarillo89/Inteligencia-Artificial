#include "../Comportamientos_Jugador/jugador.hpp"
#include <iostream>
#include <stdlib.h>
using namespace std;





Action ComportamientoJugador::think(Sensores sensores){

	Action accion = actIDLE;
	int index;

	/*-----------------------------------------------------------------*/
	// Couts informativos:
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
	cout << endl;

	/*------------------------------------------------------------------*/
	//Comprobar si se ha reiniciado el juego:
	if(sensores.reset)
		restart();
	/*------------------------------------------------------------------*/

	//Actualizar posión:
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
	/*------------------------------------------------------------------*/

	// Actualizar mapas informativos:
	AniadeAlMap(sensores);
	if (sensores.terreno[0]=='K' && !bien_situado){
		vfil = fil;
		vcol = col;
		fil = sensores.mensajeF;
		col= sensores.mensajeC;
		MergeMap();
		bien_situado = true;
	}

	/*------------------------------------------------------------------*/

	//Comprobar si hay un K cerca:
	if(!bien_situado){
		for (int i = 0; i < sensores.terreno.size(); i++) {
			if(sensores.terreno[i] == 'K' && i != 0 && i != 2 && i != 6 && i != 12){
				objetivo_fijado = true;
				pos_objetivo = i;
				FijaRuta(num_avances, Giro);
			}
		}
	}
	//Comportamiento Básico:
	if(ultimaAccion == actPICKUP){
		accion = actPUSH; // Si recogí un objeto lo guardo.
	}
	else if((sensores.terreno[2] == 'T' ||  sensores.terreno[2] == 'S' || sensores.terreno[2] == 'K' || sensores.terreno[2] == 'D') && sensores.superficie[2] == '_'){
		if(objetivo_fijado){ //Si tengo un objetivo fijado sigo la ruta:
			if(num_avances > 0){ accion = actFORWARD; num_avances--;}
			else{ accion = Giro; objetivo_fijado = false; }
		}
		else
			if(girar_en >0 ){accion = actFORWARD; girar_en--;}
			else if(rand()%2==0){ accion = actTURN_L; girar_en = 61;}
			else{ accion = actTURN_R; girar_en = 61;}
	}
	else if(sensores.superficie[2] == '0' || sensores.superficie[2] == '1' || sensores.superficie[2] == '2' || sensores.superficie[2] == '3'){
		//Si delante tengo un objeto.
		int index;
		switch (sensores.superficie[2]) {
			case '0': index = 0; break;
			case '1': index = 1; break;
			case '2': index = 2; break;
			case '3': index = 3; break;
		}
		if(!objetos[index]){ //Si no he recogido ese objeto.
			if(sensores.objetoActivo == '_'){ //Si no tengo nada en la mano.
				accion = actPICKUP; //Recojo el objeto.
				objetos[index] = true;
			}
			else //Tengo algo en la mano.
				accion = actPUSH; //Guardo en la mochila para coger el objeto.
		}
		else{ //Ya tengo un objeto de ese tipo. Lo evito:
			if(!girar_derecha){ accion = actTURN_L; objetivo_fijado = false;}
			else{ accion = actTURN_R; objetivo_fijado = false;}
		}
	}
	else if(ultimaAccion == actPOP){
		if(sensores.objetoActivo != objetoNecesario) //Si saqué un objeto de la mochila y no es lo que busco.
			accion = actPUSH; //Vuelvo a guardarlo.
		else
			accion = actIDLE;
	}
	else if(sensores.terreno[2] == 'A' && sensores.superficie[2] == '_'  && sensores.objetoActivo == '1'){
		//Si hay agua y tengo el bikini equipado:
		if(objetivo_fijado){ //Si tengo un objetivo fijado sigo la ruta:
			if(num_avances > 0){ accion = actFORWARD; num_avances--;}
			else{ accion = Giro; objetivo_fijado = false; }
		}
		else
			if(girar_en >0 ){accion = actFORWARD; girar_en--;}
			else if(rand()%2==0){ accion = actTURN_L; girar_en = 61;}
			else{ accion = actTURN_R; girar_en = 61;}
	}
	else if(sensores.terreno[2] == 'A'  && objetos[1]){
		if(sensores.objetoActivo == '_')
			accion = actPOP;	//Si hay agua y tengo el bikini en la mochila, lo busco.
		else
			accion = actPUSH; //Si tengo algo en la mano, guardo primero para sacar otra cosa.
		objetoNecesario = '1';
	}
	else if(sensores.terreno[2] == 'B' && sensores.superficie[2] == '_'  && sensores.objetoActivo == '2'){
		//Si hay bosque y tengo las zapatillas equipadas:
		if(objetivo_fijado){ //Si tengo un objetivo fijado sigo la ruta:
			if(num_avances > 0){ accion = actFORWARD; num_avances--;}
			else{ accion = Giro; objetivo_fijado = false; }
		}
		else
			if(girar_en >0 ){accion = actFORWARD; girar_en--;}
			else if(rand()%2==0){ accion = actTURN_L; girar_en = 61;}
			else{ accion = actTURN_R; girar_en = 61;}
	}
	else if(sensores.terreno[2] == 'B'  && objetos[2]){
		if(sensores.objetoActivo == '_')
			accion = actPOP;	//Si hay bosque y tengo las zapatillas en la mochila, las busco.
		else
			accion = actPUSH; //Si tengo algo en la mano, guardo primero para sacar otra cosa.
		objetoNecesario = '2';
	}
	else if(sensores.terreno[2] == 'D' && sensores.superficie[2] == 'd' && sensores.objetoActivo == '3'){
		accion = actGIVE; //Abro la puerta.
	}
	else if(sensores.terreno[2] == 'D' && sensores.superficie[2] == 'd' && objetos[3]){
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
	else if(sensores.superficie[2] == 'l' && objetos[0]){
		if(sensores.objetoActivo == '_')
			accion = actPOP;	//Si tengo delante un lobo y tengo un hueso en la mochila lo busco.
		else
			accion = actPUSH; //Si tengo algo en la mano, guardo primero para sacar otra cosa.
		objetoNecesario = '0';
	}
	else if(!girar_derecha){
		accion = actTURN_L;
		objetivo_fijado = false; // La ruta ya no es válida.
	}
	else{
		accion = actTURN_R;
		objetivo_fijado = false; // La ruta ya no es válida.
	}

	//Nos aseguramos que el hueso ha sido entregado con éxito:
	if(ultimaAccion == actGIVE){
		if(objetoDado == 0 && sensores.objetoActivo == '_')
			objetos[0] = false;
	}

	/*------------------------------------------------------------------*/

	//Return
	ultimaAccion = accion;
	return accion;
}

void ComportamientoJugador::restart(){
	num_avances = 0;
	girar_en = 61;
	objetivo_fijado = false;
	Giro = actIDLE;
	objetoDado = -1;
	pos_objetivo = -1;
	fil = 99;
	col = 99;
	brujula = 0;
	ultimaAccion = actIDLE;
	girar_derecha = false;
	bien_situado = false;
	for(int i = 0; i < 200 ; ++i)
		for(int j = 0; j < 200 ; ++j)
			memoriaTemp[i][j] = '?';

	for(int i = 0; i < 4; ++i)
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

void ComportamientoJugador::FijaRuta(int & num_avances, Action & Giro){
	if(1 == pos_objetivo || pos_objetivo == 3)
		num_avances = 1;
	else if (4 <= pos_objetivo && pos_objetivo <= 5 && 7 <= pos_objetivo && pos_objetivo <= 8)
		num_avances = 2;
	else if(pos_objetivo != 12)
		num_avances = 3;

	if(pos_objetivo == 1 || pos_objetivo == 4 || pos_objetivo == 5 ||
		 pos_objetivo == 9 || pos_objetivo == 10 || pos_objetivo == 11)
		Giro = actTURN_L;
	else
		Giro = actTURN_R;
}

int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
