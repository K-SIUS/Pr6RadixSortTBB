#include <iostream>
#include <vector>
#include <algorithm>

#include <tbb/tbb.h>
#include "oneapi/tbb/blocked_range.h"
#include "oneapi/tbb/parallel_for.h"

//Intento de Práctica de Ander Sarrión Martín basada en
//https://github.com/wisaaco/AA_Parallel_Programming_Lab/blob/main/6_Example_PackingProblem/main.cpp
using namespace std;
using namespace oneapi;


// DoMAP será el encargado de preparar los valores del array en formato binario,
// repetiremos x veces el proceso, x siendo el número de valores que haya en el array, de ahí que se utilice el map.
//tendremos que pasarle de entrada el vector de enteros, y el entero que haga la función del filtro.
vector<int> doMAP(int a,int x[], int n)
{
    //podemos reciclar este vector de salida y escribir en él los 1 de los índices
    vector<int> out(n);

    tbb::parallel_for(
        tbb::blocked_range<int>(0, n),

        // lambda function
        //nota: luego no era yo al que no le gustaba que se utilizaran variables 'caracter' que no expresan información.
        [&](tbb::blocked_range<int> r) {
            for (auto i = r.begin(); i != r.end(); i++) {
                // encontré que la forma de hacer el 'and' su valor binario era con el &
                // si es distinto de 0, se corresponde al valor donde está posicionado el 1 en la máscara
                if(x[i]&a != 0){
                    out[i] = 1;
                }else{
                    out[i]= 0
                }
            }
        }

    );
    return out;
}


//en principio el SCAN no se ha de modificar, hará el scan con el mapeado previo asignado en el array de out
int doSCAN(int out[], const int in[],int n){
    //no estoy muy seguro que es total_sum. Se supone que es un entero,
    //pero quiero que devuelva el array de índices del vector de entrada
    int total_sum = tbb::parallel_scan(
        tbb::blocked_range<int>(0, n), //range
        0, //id
        [&](tbb::blocked_range<int> r, int sum, bool is_final_scan){        
            int tmp = sum;
            for (int i = r.begin(); i < r.end(); ++i) {
                tmp = tmp + in[i];
                if (is_final_scan)
                    out[i] = tmp;
            }
            return tmp;
        },
        [&]( int left, int right ) {
            return left + right;
        }
    );
    return total_sum;
}

/*
hace un paralel for i dent

*/
//doMAPFilter(&out[0],&ix[0],&x[0],&filter_results[0], x.size())
void doMAPFilter(int bolMatch[], int ixMatch[],int x[], int out[], int n){
    tbb::parallel_for(
        tbb::blocked_range<int>(0, n),
        // lambda function
        [&](tbb::blocked_range<int> r) {
            for (auto i = r.begin(); i < r.end(); i++) {
                //aquí falta hacer algo, no puede ser que cambie mi condición del if en el MAP
                //pero en su filter no se modifique.
                //esto se lee como si el contenido del mapeado inicial es true? osea si no es 0?
                //al valor del array de filtrados (osea el resultado que devolverá), asignar el valor de mi entrada inicial
                //al valor anterior del array de indices. un array cuyo identificador es un array anidado es una obra de arte
                if (bolMatch[i]){
                    out[ixMatch[i]-1] = x[i];    
                }
                // y en caso de no ser true? es decir, ¿en caso de ser 0 se ignora?
                //aquí faltaría un else con la condición de que si la aplicación de la máscara tiene 0
                //si hay un 0 se tendría que guardar el valor inicial del vector cuyo indice será el mismo que en el valor resultante.
            }
        }
    );
}

int main(){
    /*
    Aquí se debe inicializar la máscara, los hilos y montar el radix.
    
    */

    //esto es mi máscara o condición
    //no sé qué valor dar inicial, pero sé que tiene que estar en bits por lo que la especificación vendrá encabezada
    //por 0b
    static int a = 0b00001;
    static vector<int> x{7,1,0,13,0,15,20};
    //{0b00111, 0b00001, 0b00000, 0b01101, 0b00000, 0b01111,0b10100}
    //n será = 7 para este array.
	tbb::tick_count t0 = tbb::tick_count::now();
 	
    //MAP operation
    //aquí se está llamando a la función de map. Por qué solo se hace una vez¿? es correcto?
    //me volverá un único valor así
    vector<int> bolMatch = doMAP(a,&x[0], x.size());
 	
    //y esto simplemente es un print.
    cout << "Map vector: "<< endl;
    for (int i: bolMatch){
        cout << i << ',';
    }
    cout << endl;

    //SCAN
    vector<int> ixMatch(x.size());
    int sum = doSCAN(&ixMatch[0], &bolMatch[0],  x.size()); //get index order


    cout << "Scan vector: " << sum << endl;
    for (int i: ixMatch){
        cout << i << ',';
    }
    cout << endl;
    //JOIN 
    vector<int> filtered_results(sum);
    doMAPFilter(&bolMatch[0],&ixMatch[0],&x[0],&filtered_results[0], x.size());

    //esto es el print del resultante ya ordenado vector ordenado 
    cout << "Filtered vector: " << endl;
    for (int i: filtered_results){
        cout << i << ',';
    }
    cout << endl;

 	cout << "\nTime: " << (tbb::tick_count::now()-t0).seconds() << "seconds" << endl;

 	return 0;

 }
