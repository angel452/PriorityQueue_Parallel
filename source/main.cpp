#include <bits/stdc++.h>
#include <mutex>
#include <stdlib.h>
#include <time.h>
#include <thread>
#include <iostream>
using namespace std;

template <class T>
class Node
{
public:
    T data;

    //-----  for Parallel -----------
    bool deletedThread;
    bool addedThread;

    Node **next;

    Node(int _data, int _lvl){
        data = _data;
        next = new Node*[_lvl+1]; //asignamos memoria
        memset(next, 0, sizeof(Node*)*(_lvl+1)); // Llenamos con ceros

        deletedThread = false;  // TRUE: significa que otro thread ya lo borro
                                // FALSE: Sigue activo y en la cola

        addedThread = false;   // TRUE: indica que el nodo ya fue insertado
                               // FALSE: indica que el nodo esta en proceso de insertado
    }
};

const unsigned int MxLVL = 3; // Maximo nivel dado por el usuario


template <class T>
class PriorityQueue
{
private:
    //int MxLVL;
    int lvl; // Maximo nivel que se obtiene aleatoriamente por nodo
    float P;

    mutex candadoBefore;
    mutex candadoAfter;

    Node<T> *header;
    Node<T> *tail;

public:
    // ######################## CONSTRUCTOR ######################################
    PriorityQueue(float _semilla)
    {
        //MxLVL = _maxLevel;
        P = _semilla;
        lvl = 0;

        // creamos un header y tail para establecer limites.
        // Header -> valores -1
        // Tail -> valores -2
        header = new Node<T>(-1, MxLVL);
        tail = new Node<T>(-2, MxLVL);

        // Apuntamostodo lo del header al tail
        for(int i = 0; i < MxLVL; i++){
            header->next[i] = tail;
        }
    }

    // ################## FUNCIONES ADICIONALES #################################
    int randomLvl()
    {
        float r = (float)rand()/RAND_MAX;
        int aux_lvl = 0;
        //cout << "Aleatorio: " << r << endl;
        while (r < P && aux_lvl < MxLVL) // crea aleatoriamente el nivel de cada nodo
        {
            aux_lvl++;
            r = (float)rand()/RAND_MAX;
            //cout << "Aleatorio2: " << r << endl;
        }

        //cout << "EL NIVEL SERA: " << aux_lvl << endl;
        return aux_lvl;
    }

    Node<T>* createNode(int key, int level)
    {
        Node<T> *n = new Node<T>(key, level);
        return n;
    }

    // ################## FUNCIONES DEL PQ #################################
    bool onlyFind(int numero){
        cout << "############################ Buscando " << numero << " ##############################" << endl;
        Node<T> *current = header;

        // ------------- BUSQUEDA -------------------------------
        // FOR -> Busqueda por nivel
        for (int i = lvl; i >= 0; i--)  // Busqueda en maximo nivel
        {
            // i -> nivel en donde se esta buscando
            if(current->next[i]->data == numero){
                cout << "El Elemento " << numero << " ya esta insertado. Esta en el nivel" << i << endl;
                return true;
            }

            //WHILE -> busqueda por enlaces entre nodos
            while (current->next[i]->data != -2 && current->next[i]->data > numero){
                current = current->next[i];

                if(current->next[i]->data == numero){
                    cout << "El Elemento " << numero << " ya esta insertado. Esta en el nivel" << i << endl;
                    return true;
                }
            }
        }

        cout << "El elemento " << numero << " no esta en la cola para insertar. Si se puede insertar" << endl;
        return false;
    }

    bool findInsert(int numero, Node<T> *current, Node<T> *beforeList[], Node<T> *afterList[]){

        candadoBefore.lock();
        candadoAfter.lock();

        cout << "############### Buscando " << numero << " para insertar ##################" << endl;

        current = header;

        // ------------- BUSQUEDA -------------------------------
        // FOR -> Busqueda por nivel
        for (int i = lvl; i >= 0; i--)  // Busqueda en maximo nivel
        {
            //cout << "Buscando en el nivel: " << i << endl;
            if(current->next[i]->data == numero){
                cout << "El Elemento " << numero << " ya esta insertado. Esta en el nivel" << i << endl;

                candadoBefore.unlock();
                candadoAfter.unlock();

                return true;
            }

            //WHILE -> busqueda por enlaces entre nodos
            while (current->next[i]->data != -2 && current->next[i]->data > numero){
                current = current->next[i];
                //cout << "actual: " << current->data << endl;

                if(current->next[i]->data == numero){
                    cout << "El Elemento " << numero << " ya esta insertado. Esta en el nivel" << i << endl;

                    candadoBefore.unlock();
                    candadoAfter.unlock();

                    return true;
                }
            }

            // Guardamos los futuros enlaces en estos 2 arrays
            beforeList[i] = current;
            afterList[i] = current->next[i];
        }

        //cout << "-------------------------------------------------------------" << endl;
        cout << "El elemento " << numero << " no esta en la cola para insertar. Si se puede insertar" << endl;

        current = current->next[0];
        return false;
    }

    void insert(int k)
    {
        Node<T> *current = header;

        Node<T> *beforeList[MxLVL+1];
        memset(beforeList, 0, sizeof(Node<T>*)*(MxLVL+1));

        Node<T> *afterList[MxLVL+1];
        memset(afterList, 0, sizeof(Node<T>*)*(MxLVL+1));

        bool isInQueue;
        isInQueue = findInsert(k, current, beforeList, afterList); // Retorna los 2 arrays: beforeList y afterList

        if(current->data == -2 || !isInQueue)
        {
            // creacion del nivel aleatorio de un nodo
            int rlevel = randomLvl();
            Node<T>* n = createNode(k, rlevel);

            // rlevel -> Es el nivel del nodo
            // lvl -> es el maximo nivel que un nodo ALCANZO al crearlo
            // MxLVL -> es el maximo nivel que un nodo PUEDE alcanzar

            if (rlevel > lvl) // El nivel del nuevo nodo es mayor al maximo actual en el queue?
            {
                //Actualizamos before y after pues los niveles adicionales no estan llenos
                for (int j = lvl+1 ; j < rlevel+1 ; j++){
                    //Las redireccionamos directamente con header y tail pues sabemos con certeza que es un nuevo enlace. y no hay nadie a sus costados
                    beforeList[j] = header;
                    afterList[j] = tail;
                }

                //actualizamos el mayor nivel del random
                lvl = rlevel;
            }

            // Linkeamos before -> nuevo nodo -> After
            for (int z = 0; z <= rlevel ; z++)
            {
                n->next[z] = afterList[z];
                beforeList[z]->next[z] = n;
            }
            cout << "Se inserto el elemento: " << k << "\n";
            candadoAfter.unlock();
            candadoBefore.unlock();
        }
        else{
            //cout << "El elemento ya se encuentra en la cola" << endl;
        }
    }

    bool findDelete(int numero, Node<T> *current, Node<T> *beforeList[], Node<T> *afterList[]){

        candadoBefore.lock();
        candadoAfter.lock();

        cout << "################## Buscando " << numero << " para eliminar ####################" << endl;

        current = header;

        // ------------- BUSQUEDA -------------------------------
        // FOR -> Busqueda por nivel
        for (int i = lvl; i >= 0; i--)  // Busqueda en maximo nivel
        {
            //WHILE -> busqueda por enlaces entre nodos
            while (current->next[i]->data != -2 && current->next[i]->data > numero){
                current = current->next[i];
            }
            beforeList[i] = current;

            if(current->next[i]->data == -2){
                afterList[i] = current->next[i];
            } else{
                afterList[i] = current->next[i]->next[i];
            }
        }

        current = current->next[0];

        if(current->data == numero){
            cout << "Si se encontro el elemento " << numero <<  " para eliminar" << endl;
            return true;
        } else{
            cout << "No se encontro el elemento " << numero <<  "  para eliminar" << endl;

            candadoBefore.unlock();
            candadoAfter.unlock();

            return false;
        }
    }

    void deleteData(int k)
    {
        Node<T> *current = header;

        Node<T> *beforeList[MxLVL+1];
        memset(beforeList, 0, sizeof(Node<T>*)*(MxLVL+1));

        Node<T> *afterList[MxLVL+1];
        memset(afterList, 0, sizeof(Node<T>*)*(MxLVL+1));

        bool isInQueue;
        isInQueue = findDelete(k, current, beforeList, afterList); // Retorna los 2 arrays: beforeList y afterList

        if(current->data == -2 || isInQueue)
        {
            // Verificamos si el nodo al que eliminamos, contenia al maximo nivel.
            for(int i = lvl; i >= 0; i--){
                //cout << "lvl es: " << lvl << endl;
                Node<T> *aux1 = beforeList[i];
                Node<T> *aux2 = afterList[i];
                if( aux1->data != -1 || aux2->data != -2 ){
                    break;
                }
                // Bajamos el nivel
                lvl--;
            }

            // Linkeamos before -> nuevo nodo -> After
            for (int z = 0; z <= lvl ; z++)
            {
                beforeList[z]->next[z] = afterList[z];
            }

            cout << "Se elimino el elemento: " << k << "\n";

            candadoBefore.unlock();
            candadoAfter.unlock();
        }
        else{
            //cout << "El elemento ya se encuentra en la cola" << endl;
        }
    }

    void displaySkipList()
    {
        cout << endl;
        cout<<" -------- Skip  List View -----------  " << endl;
        for (int i = 0; i <= lvl; i++)
        {
            Node<T> *node = header->next[i];
            cout << "Level " << i << ": ";
            while (node->data != -2)
            {
                cout << node->data<<" ";
                node = node->next[i];
            }
            cout << endl;
        }
    }

    void displayPriorityQueue()
    {
        cout << endl;
        cout<<" -------- Priority Queue View -----------  " << endl;
        Node<T> *aux = header->next[0];
        while (aux->data != -2)
        {
            cout << aux->data<<" ";
            aux = aux->next[0];
        }
        cout << endl;
    }

    void getPQ(){
        cout << "-----------------------------------------" << endl;
        cout << "Dato con mas prioridad: " << header->next[0]->data << endl;
        cout << "-----------------------------------------" << endl;
    }
};

template <class T>
struct Add
{
    PriorityQueue<T> *queue;
    int valMin, valMax;
    Add(PriorityQueue<T> &_queue,int _valMin,int _valMax)
    {
        queue = &_queue;
        valMin = _valMin;
        valMax = _valMax;
    }

    void operator()(int n_operations)
    {
        for(int i = 0; i < n_operations; i++)
        {
            int number = valMin+rand()%(valMax);
            //cout << "Añadiendo: " << number << endl;
            queue->insert(number);
        }
    }
};

template <class T>
struct Delete
{
    PriorityQueue<T> *queue;
    int valMin, valMax;
    Delete(PriorityQueue <T> &_queue,int _valMin,int _valMax)
    {
        queue = &_queue;
        valMin = _valMin;
        valMax = _valMax;
    }

    void operator()(int n_operations)
    {
        for(int i=0; i < n_operations; i++)
        {
            int number = valMin+rand()%(valMax);
            //cout << "Eliminando: " << number << endl;
            queue->deleteData(number);
        }
    }
};

int main()
{
    // Seed random number generator
    srand((unsigned)time(0));

    cout << "----- Usando Threads: -------- " << endl;

    thread* threads[2];
    //PriorityQueue queue(3, 0.5);
    PriorityQueue<int> queue(0.5);

    Add opt1(queue, 1,1000);
    thread first(opt1,1000);

    Delete opt2(queue, 1, 1000);
    thread second(opt2, 1000);

    threads[0] = &first;
    threads[1] = &second;

    threads[0]->join();
    threads[1]->join();

    queue.displaySkipList();
    queue.displayPriorityQueue();
    queue.getPQ();

    return 0;
}