#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H
#include <list>
#include "comportamientos/comportamiento.hpp"
using namespace std;

class ComportamientoJugador : public Comportamiento{

  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size){
      restart();
    }

    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);

    int interact(Action accion, int valor);

    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

  private:

    // Representación del estado.
    struct estado{
    	int fila;
    	int columna;
    	int orientacion;
    };

    //Representación de un nodo del árbol utilizado para la búsqueda.
    struct nodo{
      estado status;
      int heuristica;
      list<Action> acciones;
    };

    // Variables necesarias:
    int fil, col, brujula;
    int vfil, vcol;
    int girar_en;
    Action ultimaAccion;
    int objetoDado;
    bool girar_derecha, bien_situado;
    char memoriaTemp[200][200];
    char objetoNecesario;

    //Mochila auxiliar para conocer los objetos recogidos.
    bool objetos[5];

    //Vector que almacena las posiciones de los reyes encontrados:
    vector<pair<int,int>> reyes;

    //Elementos necesarias cuando se fija un Plan:
    int deliberar_en;
    bool deliberando;
    bool error_plan;
    list <Action> plan;

    //Funciones auxiliares y privadas:
    //Reiniciar player:
    void restart();

    //Juntar memoria con matriz resultado:
    void MergeMap();

    //Añadir el cono de visión al mapa:
    void AniadeAlMap(Sensores sensores);

    //Comprueba si hay un obstáculo delante:
    bool ObstaculoDelante(const vector<unsigned char> & terreno, const vector<unsigned char> & superficie);

    //Método para establecer una búsqueda simple (el objetivo está a la vista del jugador)
    void pathFinding_simple(int pos, list<Action> &plan);

    //Método de búsqueda para fijar rutas complejas:
    //Método de escalada por máxima pendiente:
    bool pathFinding(const estado &origen, const estado &destino, list<Action> &plan);

    //Método para imprimir el plan establecido:
    void ImprimePlan(list<Action> plan);

    //Método para comprobar si se puede recoger o no un objeto:
    pair<bool,int> PuedoRecoger(unsigned char obj);

    //Método para calcular el valor de la heuristica de un nodo.
    int calculaHeuristica(const nodo& hijo, const estado &destino);

    //Método para generar los hijos de un nodo:
    nodo expandeNodo(nodo node, int action, const estado &destino);

    //Método para ir guardando las posiciones de los reyes:
    void guardaRey(Sensores sensores);
};


#endif
