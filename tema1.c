#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct incident{
    int id;
    char priority[7];
    char *description;
    char status[11];
}*incident;

typedef struct intervente{
    incident *incident;
    struct unit *unit;
}*intervention;

typedef struct listaInterventie{
    intervention interv;
    struct listaInterventie *next;
    struct listaInterventie *prev;
}*ListIntervention;

typedef struct stivaInterventie{
    ListIntervention elem; 
    struct stivaInterventie *next;
}*interventionStack;

typedef struct lista{
    incident inc;
    struct lista *next;
    struct lista *prev;
}*List;

typedef struct coada{
    List elem;
    struct coada * next;
}*waitingQueue; //pentru incidente

typedef struct{
    waitingQueue front, rear;
}waitingQueueBounds;

typedef struct unit{
    int id;
    char type;
    int availability;
}unit;

typedef struct coadaU{
    unit **elem; //pointer la vector de tip unit, ce contine echipajele de interventie
    struct coadaU *next;
}*unitQueue;

typedef struct{
    unitQueue front, rear;
}unitQueueBounds;

void adaugaIncidentInCoada(List nodLista, waitingQueueBounds *queue_high, waitingQueueBounds 
    *queue_low, waitingQueueBounds *queue_medium){
    waitingQueue nou = (waitingQueue)calloc(1, sizeof(struct coada));
    if(strcmp(nodLista->inc->priority,"low")==0){
        nou->elem = nodLista;
        nou->next=NULL;
        //verificam coada vida 
        if(queue_low->front == NULL){
            queue_low->front = queue_low->rear = nou;
        }else{
        queue_low->rear->next = nou;
        queue_low->rear=nou;
        }
    }else if(strcmp(nodLista->inc->priority,"medium")==0){
        nou->elem = nodLista;
        nou->next=NULL;
        //verificam coada vida 
        if(queue_medium->front == NULL){
            queue_medium->front = queue_medium->rear = nou;
        }else{
        queue_medium->rear->next = nou;
        queue_medium->rear=nou;
        }
    }else if(strcmp(nodLista->inc->priority,"high")==0){
        nou->elem = nodLista;
        nou->next=NULL;
        //verificam coada vida 
        if(queue_high->front == NULL){
            queue_high->front = queue_high->rear = nou;
        }else{
        queue_high->rear->next = nou;
        queue_high->rear=nou;
        }
    }
}

List adaugaNodListaIncidente(List listaIncidente, int id, char priority[7], char *descriere){
    //adaugare la finalul listei de incidente
    List nou;
    //alocam memorie pentru structuri, dar si pentru campuri din interior
    nou = (List)calloc(1, sizeof(struct lista));
    nou->inc = (incident)calloc(1, sizeof(struct incident));
    nou->inc->description = (char*)calloc(1, strlen(descriere)+1);

    nou->inc->id = id; //id
    strcpy(nou->inc->priority, priority); //prioritate
    strcpy(nou->inc->description, descriere); //descriere
    strcpy(nou->inc->status, "queued");  //NOUL STATUS "QUEUED"
    //printf("Adresa description: %p\n", (void*)nou->inc->description);
    
    nou->next=listaIncidente; 
    nou->prev=(listaIncidente->prev);
    listaIncidente->prev->next=nou;
    listaIncidente->prev=nou;

    return nou; //ultimul nod
}

ListIntervention adaugaNodListaInterventie(List nodIncidentExtras, unit *unitateExtrasa, ListIntervention listaInterventii){
     //adaugare la finalul listei de interventii
    ListIntervention nou;
    //alocam memorie pentru structuri, dar si pentru campuri din interior
    nou = (ListIntervention)calloc(1, sizeof(struct listaInterventie));
    nou->interv = (intervention)calloc(1, sizeof(struct intervente));

    nou->interv->unit = unitateExtrasa;
    nou->interv->incident = &nodIncidentExtras->inc;
    
    nou->next=listaInterventii; 
    nou->prev=(listaInterventii)->prev;
    (listaInterventii)->prev->next=nou;
    (listaInterventii)->prev=nou;

    return nou;
}

void adaugaNodInterventieInStiva(ListIntervention nod, interventionStack *stivaInterventii){
    //verificam stiva vida
    //adaugam nod de interventie in stiva si returnam pointer la ultimul nod
    interventionStack nou = malloc(sizeof(struct stivaInterventie));
    if(nou != NULL){
        nou->elem = nod;
        nou->next = *stivaInterventii;
        *stivaInterventii = nou;
    }
}

void adaugaUnitateInCoada(unitQueueBounds *queueUnit, unit *echipajeInterventie, int *lungimeCoadaUnitate){
        //adaugam echipajele disponibile in COADA DE DISPONIBILITATE
        unitQueue nou = calloc(1, sizeof(struct coadaU));
        (*lungimeCoadaUnitate)++;
        //aloc memorie ca sa stochez un pointer de tip unit*
        nou->elem = (unit**)calloc(1, sizeof(struct unit*));
        //nu mai aloc memorie pentru o structura de tip unit, ca o am deja alocata

        *(nou->elem) = &echipajeInterventie[(*lungimeCoadaUnitate)-1];
        nou->next = NULL;
        if(queueUnit->front == NULL)
            queueUnit->front = queueUnit->rear = nou;
        else{
            queueUnit->rear->next = nou;
            queueUnit->rear=nou;
        }
}

List extrageIncidentDinCoada(waitingQueueBounds *queue){
    //printf("queue_high->front: %p %p\n", queue->front, queue->rear);
    List nodExtras = queue->front->elem;
    waitingQueue aux = queue->front;
    //verific coada cu un singur element 
    if(queue->front == queue->rear){
        queue->front = queue->rear = NULL;
    }else{
        queue->front=queue->front->next;
    }
    free(aux);
    return nodExtras;
}

unit* extrageUnitateDinCoada(unitQueueBounds *queue, unit *echipajeInterventie, int *lungimeCoadaUnitate){
    //pointer la element din vectorul de unitati 
    unit *echipaj = *queue->front->elem;
    unitQueue aux = queue->front;
    //verific coada cu un singur element
    if(queue->front == queue->rear)
        queue->front = queue->rear = NULL;
    else{
        queue->front=queue->front->next;
    }
    free(aux->elem);
    free(aux);
    (*lungimeCoadaUnitate)--;
    return echipaj; //un element din vector 
}

void dequeueWaitingQueue(waitingQueueBounds *queue){
    //pentru eliberarea memoriei
    waitingQueue first = queue->front;
    waitingQueue second = first->next;
    while(second != NULL){
        //eliberam memoria alocata pentru nodul de coada
        free(first);
        first=second;
        second=second->next;
    }
    free(queue->rear);
    queue->rear = queue->front = NULL;
}

void dequeueUnitQueue(unitQueueBounds *queue){
    //pentru eliberarea memoriei
    unitQueue first = queue->front;
    unitQueue second = first->next;
    while(second != NULL){
        //eliberam vectorul de pointeri
        free(first->elem);
        //eliberam nodul din coada
        free(first);
        first=second;
        second=second->next;
    }
    free(queue->rear->elem);
    free(queue->rear);
    queue->rear = queue->front = NULL;
}

void popInterventie(interventionStack *stivaInterventie){
    //pentru UNDO_LAST_DESPATCH
    interventionStack s = *stivaInterventie;
    if(s->next != NULL){
        *stivaInterventie = (*stivaInterventie)->next;
        //eliminam nodul din lista de interen
        s->elem->prev->next = s->elem->next;
        s->elem->next->prev = s->elem->prev;
        free(s);
    }
}

int main(){
    FILE *f=fopen("tema1.in", "r");
    unit *echipajeInterventie = malloc(100*sizeof(struct unit));
    char linie[500], aux2;
    char *cuvant = NULL;
    char *descriere = NULL;
    char priority[7];
    int i, nrEchipaje, nrOperatii;
    int id;

    //initializare lista INCIDENTE
    List listaIncidente;
    listaIncidente = (List)calloc(1, sizeof(struct lista));
    listaIncidente->next=listaIncidente->prev=listaIncidente;
    
    List ultimNod = NULL;

    //initializare lista INTERVENTII
    ListIntervention listaInterventii;
    listaInterventii = (ListIntervention)calloc(1, sizeof(struct listaInterventie));
    listaInterventii->next=listaInterventii->prev=listaInterventii;

    //initializare STIVA INTERVENTII-contine noduri din lista interventii
    interventionStack stivaInterventii;
    stivaInterventii = calloc(1, sizeof(struct stivaInterventie));
    stivaInterventii->next = NULL;

    //
    List nodIncidentExtras = NULL;
    unit *unitateExtrasa = NULL;
    ListIntervention nodInterventie = NULL;

    //initiliazare cozi de asteptare pt LISTA CU INCIDENTE 
    waitingQueueBounds queue_high, queue_low, queue_medium;
    queue_high.front = queue_high.rear = NULL;
    queue_medium.front = queue_medium.rear = NULL;
    queue_low.front = queue_low.rear = NULL;

    fscanf(f, "%d", &nrEchipaje);
    for(i=0; i<nrEchipaje; i++){
        fscanf(f, "%d %c", &echipajeInterventie[i].id, &echipajeInterventie[i].type);
        echipajeInterventie[i].availability = 1;
    }

    //realocam exact pentru nrEchipaje
    echipajeInterventie = realloc(echipajeInterventie, nrEchipaje*sizeof(struct unit));
    //coada pentru echipajele disponibile
    unitQueueBounds queue_available_units;
    queue_available_units.rear = queue_available_units.front = NULL;

    //adaugam echipajele disponibile in coada 
    i=0;
    int lungimeCoadaUnitate=0;
    while(i<nrEchipaje){
        if(echipajeInterventie[lungimeCoadaUnitate].availability == 1){
                adaugaUnitateInCoada(&queue_available_units, echipajeInterventie, &lungimeCoadaUnitate);
        }
            i++;
        }

    fscanf(f, "%d", &nrOperatii);
    fgetc(f); //pentru \n

    waitingQueue nod = NULL;
    unitQueue nou = NULL; //pentru SOLVED_INCIDENT
    unitQueue nouU = NULL; //pentru UNDO_

    for(int j=0; j<nrOperatii; j++){
        fgets(linie, 900, f);
        cuvant = strtok(linie, " \n"); 
        if(cuvant == NULL)
                break;
        if(strcmp(cuvant, "ADD_INCIDENT")==0){
            cuvant = strtok(NULL, " \n"); //iau urmatorul cuvant 
            id = atoi(cuvant); //id
            cuvant = strtok(NULL, " \n");
            strcpy(priority, cuvant); //priority
            cuvant = strtok(NULL, "\n");
            descriere = calloc(1, (strlen(cuvant)+1));//descriere
            strcpy(descriere, cuvant);//descriere
            //adaugam nod in listaIncidente 
            ultimNod=adaugaNodListaIncidente(listaIncidente, id, priority, descriere);
            adaugaIncidentInCoada(ultimNod, &queue_high, &queue_low, &queue_medium);
            free(descriere);
        }
        if(strcmp(cuvant, "CHECK_UNITS_AVAILABILITY")==0){
            printf("Number of available units: %d\n", lungimeCoadaUnitate);
        }
        if(strcmp(cuvant, "DISPATCH") == 0){
            //demarare intervetie
            //daca nu am incident in coada || nu am unitate in coada || nu am nicio interventie creata deja 
            if((queue_high.front == NULL && queue_low.front == NULL  && queue_medium.front == NULL) || 
            queue_available_units.front == NULL || listaInterventii->next == NULL){
                printf("INVALID OPERATION! ERROR 404\n");
            }
            if(queue_high.front != NULL){//STATUS - HIGH
                if(lungimeCoadaUnitate != 0){
                    nodIncidentExtras = extrageIncidentDinCoada(&queue_high);
                    unitateExtrasa = extrageUnitateDinCoada(&queue_available_units, echipajeInterventie, &lungimeCoadaUnitate);
                    //modificam parametri
                    strcpy(nodIncidentExtras->inc->status, "intervened");
                    unitateExtrasa->availability = 0;
                    //adauga nod la lista de interventii
                    //nu mai aloc iar memorie, ca e DEJA alocata
                    nodInterventie = adaugaNodListaInterventie(nodIncidentExtras, unitateExtrasa, listaInterventii);
                    adaugaNodInterventieInStiva(nodInterventie, &stivaInterventii);
                }
            }else if(queue_medium.front != NULL){//STATUS MEDIUM
                if(lungimeCoadaUnitate != 0){
                    nodIncidentExtras = extrageIncidentDinCoada(&queue_medium);
                    unitateExtrasa = extrageUnitateDinCoada(&queue_available_units, echipajeInterventie, &lungimeCoadaUnitate);
                    //modificam parametri
                    strcpy(nodIncidentExtras->inc->status, "intervened");
                    unitateExtrasa->availability = 0;
                    //adauga nod la lista de interventii
                    //nu mai aloc iar memorie, ca e DEJA alocata
                    nodInterventie = adaugaNodListaInterventie(nodIncidentExtras, unitateExtrasa, listaInterventii);
                    adaugaNodInterventieInStiva(nodInterventie, &stivaInterventii);
                }
            }else if(queue_low.front != NULL){//STATUS - LOW
                if(lungimeCoadaUnitate != 0){
                    nodIncidentExtras = extrageIncidentDinCoada(&queue_low);
                    unitateExtrasa = extrageUnitateDinCoada(&queue_available_units, echipajeInterventie, &lungimeCoadaUnitate);
                    //modificam parametri
                    strcpy(nodIncidentExtras->inc->status, "intervened");
                    unitateExtrasa->availability = 0;
                    //adauga nod la lista de interventii
                    //nu mai aloc iar memorie, ca e DEJA alocata
                    nodInterventie = adaugaNodListaInterventie(nodIncidentExtras, unitateExtrasa, listaInterventii);
                    adaugaNodInterventieInStiva(nodInterventie, &stivaInterventii);
                }
            }
        }
        if(strcmp(cuvant, "SOLVED_INCIDENT") == 0){
            cuvant=strtok(NULL, " \n");
            int id1 = atoi(cuvant); //id incident
            int ok1=0;
            //parcurg lista si gasesc incidentul cu id-ul <id> si il modific 
            //obtin mai intai pointer catre structura si apoi accesez campul id
            
            if(listaInterventii != NULL){
                //variabile pt adaugare unitate la coada
                interventionStack s = stivaInterventii;
                
                if(stivaInterventii == NULL){
                    //adica nu exista acel id 
                    printf("INVALID OPERATION! ERROR 404\n");
                }
                // //daca listaInterventii != NULL, asta nu inseamna oblig. ca am alocat memorie si pentru campul interventie
                // //el poate fi in continuare NULL
                     //verificam Conditii de EROARE
                        while(s != NULL){
                            if(s->elem != NULL){
                                if(s->elem->interv != NULL){
                                //printf("id stiva in while este:%d\n",(*s->elem->interv->incident)->id);
                                    if(strcmp((*s->elem->interv->incident)->status, "intervened") != 0){
                                        printf("INVALID OPERATION! ERROR 404\n");
                                        ok1=-1; //ca sa nu intre si pe cazul 3 de eroare, ca daca n am eroare, 
                                        //coada de unitati va fi zero, deci intra si acolo 
                                        //daca am incident in stiva, dar nu unitate, intra aici
                                        break;
                                    }
                                    if( id1 == (*s->elem->interv->incident)->id){   
                                        ok1 = 1; //am gasit nod!
                                            break;
                                    }  
                                } 
                            }
                            s=s->next;
                        }
                    if(ok1==1){//am gasit nodul cu acel id 
                    //modificam parametriii lui p
                    strcpy((*s->elem->interv->incident)->status, "solved");
                    (*s->elem->interv->unit).availability = 1; //este valabil

                    //adaugam unitatea la finalul cozii de disponibilitate:
                    nou = calloc(1, sizeof(struct coadaU));
                    nou->elem = (unit**)calloc(1, sizeof(struct unit*));
                    *nou->elem = s->elem->interv->unit;
                    nou->next = NULL;
                    if(queue_available_units.front == NULL)
                        queue_available_units.front = queue_available_units.rear = nou;
                    else{
                        queue_available_units.rear->next = nou;
                        queue_available_units.rear=nou;
                    }
                    lungimeCoadaUnitate++;
                    //adaugaUnitateInCoada(&queue_available_units, p->interv->unit, &lungimeCoadaUnitate);
                    }
                    if(ok1==0 || queue_available_units.front == NULL){
                        //adica nu exista acel id sau nu avem unitate disponibila pt interventie(dar aveam incident de rezolvat)
                        printf("INVALID OPERATION! ERROR 404\n");
                    }
                }
        }
        if(strcmp(cuvant, "SHOW_UNIT") == 0){
            cuvant = strtok(NULL, " \n");
            int idUnit = atoi(cuvant), i;
            char s[13]; //sir auxiliar pentru a stoca "available"/"unavailable"
            int ok2=0; //n-am gasit nodul cautat; semafor pentru a verifica eroarea
            //cautam direct in vectorul de echipaje
            if(queue_available_units.front != NULL){
                for(i=0; i<nrEchipaje; i++){
                        if(echipajeInterventie[i].id == idUnit){
                            if(echipajeInterventie[i].availability == 1)
                                strcpy(s,"available");
                            else if(echipajeInterventie[i].availability == 0)
                                    strcpy(s,"unavailable");
                            printf("Unit %d is type %c and is %s\n", echipajeInterventie[i].id, echipajeInterventie[i].type, s);
                            ok2=1;
                            break;
                        } 
                }
            }
            //cazuri de EROARE
            if(ok2==0){
                printf("INVALID OPERATION! ERROR 404");
            }
        }
        if(strcmp(cuvant, "SHOW_INCIDENT") == 0){
            //caut in lista de incidente
            //pt ca se poate sa nu se fi facut un "dispatch" adica sa nu gasim nodul in l. de interventii 
            cuvant = strtok(NULL, " \n");
            int idIncident = atoi(cuvant);
            int ok3=0;
            List q = listaIncidente->next;
                while(q != listaIncidente){
                    //caut nodul cu acel id
                    if(q->inc->id == idIncident){
                        printf("Incident %d has %s priority, the following description: %s and is %s\n",  q->inc->id, q->inc->priority, q->inc->description, q->inc->status);
                        ok3=1;
                        break;
                    }else{
                        q=q->next;
                    }
                }
                if(ok3 == 0)
                    printf("INVALID OPERATION! ERROR 404\n");
        }
        if(strcmp(cuvant, "SHOW_INTERVENTIONS")==0){
            //conditii de EROARE
            ListIntervention s = listaInterventii->next;
            if(s == listaInterventii){
                //nu avem nicio interventie demarata, deci stiva cu interventii e goala
                printf("No intervention has been initiated\n");
            }else{
                //nu afisam cele pt care am fct UNDO, adica interventii cu status de "queued"
                while(s != listaInterventii){
                        printf("Incident %d was assigned to unit %d, and has the following status: \"%s\"\n", (*s->interv->incident)->id, (*s->interv->unit).id, (*s->interv->incident)->status);
                    s=s->next;
                }
            }   
        }
        if(strcmp(cuvant, "UNDO_LAST_DISPATCH")==0){
            //se amana remedierea unui incident 
            nod = malloc(sizeof(struct coada));
            nod->elem = malloc(sizeof(struct lista));
            
            interventionStack s = stivaInterventii;
            int ok4=0; //pp ca nu contine interventii neinitializate
            int nr=0;

            if(s->next==NULL){ 
                printf("INVALID OPERATION! ERROR 404\n");
            }else{
                while(s->next !=NULL){  
                    if(nr==1)
                        break; //la prima gasita, ne oprim
                    //extragem pana gasim o interventie in desfasurare
                    if(strcmp((*s->elem->interv->incident)->status, "intervened") == 0){
                        nr++;
                        //facem UNDO
                        strcpy((*s->elem->interv->incident)->status,"queued");
                        //plasam la inceputul cozii de asteptare specifice 
                        nod->elem->inc = *s->elem->interv->incident;
                        if(strcmp((*s->elem->interv->incident)->priority,"medium") == 0){
                            nod->next = queue_medium.front;
                            queue_medium.front = nod;
                        }
                        //marcam echipajul ca fiind available
                         (*s->elem->interv->unit).availability = 1;
                        // //extrage unitate din coada
                        
                        // //adaugam echipajul la finalul cozii de disponibilitate
                        // //TO DO FUNCTIE:
                        unitQueue nouU = calloc(1, sizeof(struct coadaU));
                        nouU->elem = (unit**)calloc(1, sizeof(struct unit*));
                        *nouU->elem = s->elem->interv->unit;
                        nouU->next = NULL;
                        if(queue_available_units.front == NULL)
                            queue_available_units.front = queue_available_units.rear = nou;
                        else{
                            queue_available_units.rear->next = nouU;
                            queue_available_units.rear=nouU;
                        }
                        lungimeCoadaUnitate++;
                        ok4=1;
                    }
                    s=s->next;
                    //POP INTERVENTIE + scoatem si din lista de interventii:
                    popInterventie(&stivaInterventii);
                }
                if(ok4==0){
                    //nu avem interventii nefinalizate
                    printf("INVALID OPERATION! ERROR 404\n");
                }
            }
        }
    }
    
    //ca sa ajung sa eliberez memorie din interiorul structurii, verific exteriorul != NULL
    if(listaIncidente != NULL){ //adica a reusit alocarea 
        if(descriere!=NULL){
            List nodCurent = listaIncidente->next;
            List nodUrmator = nodCurent->next;
            if(nodUrmator == listaIncidente){ //lista cu un singur element
                free(nodCurent->inc->description);
                free(nodCurent->inc);
                free(nodCurent);
            }else{
                while(nodUrmator != listaIncidente){
                        free(nodCurent->inc->description);
                        free(nodCurent->inc);
                        free(nodCurent);
                        nodCurent = nodUrmator;
                        nodUrmator = nodUrmator->next;
                }
                free(nodCurent->inc->description);
                free(nodCurent->inc);
                free(nodCurent);
            }
        }
        free(listaIncidente);
        listaIncidente=NULL;
    }

    //eliberez memoria pentru lista de INTERVENTII
    if(listaInterventii != NULL){
        ListIntervention nodCurentInterv = listaInterventii->next;
        ListIntervention nodUrmatorInterv = nodCurentInterv->next; 
        if(nodIncidentExtras!=NULL && unitateExtrasa!=NULL){
        // //lista cu un singur element
            if(nodCurentInterv->next == listaInterventii){
                free(nodCurentInterv->interv);
                free(nodCurentInterv);
            }else{
                while(nodUrmatorInterv != listaInterventii){
                    free(nodCurentInterv->interv);
                    free(nodCurentInterv);
                    nodCurentInterv = nodUrmatorInterv;
                    nodUrmatorInterv = nodUrmatorInterv->next;
                }
                //eliberez memoria pt ultimul nod 
                free(nodCurentInterv->interv);
                free(nodCurentInterv);
                free(listaInterventii->interv);
                free(listaInterventii);
            }
        }
        listaInterventii = NULL;
    }

    //memoria pentru lista de INCIDENTE e eliberata, deci e suficient sa eliberez memoria folosita pt 
    //nodurile de coada
    if(stivaInterventii!=NULL){
         interventionStack s = stivaInterventii, p;
         if(s->next == NULL){
            //un sg element in stiva
            free(stivaInterventii);
            stivaInterventii=NULL;
        }else if(stivaInterventii->elem != NULL){
             while(s!=NULL){
                    p=s;
                    s=s->next;
                    free(p);
            }
        }
    }

    if(queue_high.front != NULL)
        dequeueWaitingQueue(&queue_high);

    if(queue_medium.front != NULL)
        dequeueWaitingQueue(&queue_medium);

    if(queue_low.front != NULL)
        dequeueWaitingQueue(&queue_low);

    // //eliberez memorie pentru coada cu pointeri la vectorul de interventie
    if(queue_available_units.front != NULL)
        dequeueUnitQueue(&queue_available_units);
    
    if(nod!=NULL){
        free(nod->elem);
        free(nod);
    }

    if(echipajeInterventie != NULL){
        free(echipajeInterventie);
    }
    ultimNod = NULL;
}
