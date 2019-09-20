// Copio 2 SJF que saque de la deep web para anaizarlos y ver como estan hechos


// SJF 1

int main(int n, char **args) {
	printf("SJF: SHORTEST JOB FIRST");
	// cada proceso contiene dos elementos:
	//  - Posición 0 el tiempo del trabajo
	// 	- Posición 1 la posición original del proceso.
	int np=11, procesos[10][2];
	double tf = 0, tp;// tiempo promedio.
	while (np > 10 || np <= 0) {
		printf("\nNumero de procesos: ");
		scanf("%d", &np);
	}
	//para i=0, mientras i<np, hacer:...
	// pedimos el tamaño de cada proceso.
	for(int i=0; i<np; i++) {
		printf("\nInserte el proceso %d :", i+1);
		scanf("%d", &procesos[i][0]);
		procesos[i][1] = i+1;
	}
	// Algoritmo SJF
	// ordenamos de menor a mayor
	for (int i=0; i<np-1; i++) {
		for(int j=i+1; j<np; j++) {
			if (procesos[j][0]<procesos[i][0]) {
				int aux[2] = {procesos[j][0], procesos[j][1]};
				procesos[j][0] = procesos[i][0];
				procesos[j][1] = procesos[i][1];
				procesos[i][0] = aux[0];
				procesos[i][1] = aux[1];
			}
		}
	}
	for (int i=0; i<np; i++) {
		tf += procesos[i][0];
		tp = tp + tf;
		printf("\nProceso %d, concluye en %2.1f", procesos[i][1], tf);
	}
	printf("\n-------------------------------");
	printf("\nLa suma de los procesos %2.1f", tp);
	tp = tp / np;
	printf("\n\nTiempo promedio en SJF fue de: %2.2f:", tp);
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
// SJF 2 (Este esta en c++, es mas largo y mas dificil de entender)

int mat[10][6];

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

void arrangeArrival(int num, int mat[][6])
{
    for(int i=0; i<num; i++)
    {
        for(int j=0; j<num-i-1; j++)
        {
            if(mat[j][1] > mat[j+1][1])
            {
                for(int k=0; k<5; k++)
                {
                    swap(mat[j][k], mat[j+1][k]);
                }
            }
        }
    }
}

void completionTime(int num, int mat[][6])
{
    int temp, val;
    mat[0][3] = mat[0][1] + mat[0][2];
    mat[0][5] = mat[0][3] - mat[0][1];
    mat[0][4] = mat[0][5] - mat[0][2];

    for(int i=1; i<num; i++)
    {
        temp = mat[i-1][3];
        int low = mat[i][2];
        for(int j=i; j<num; j++)
        {
            if(temp >= mat[j][1] && low >= mat[j][2])
            {
                low = mat[j][2];
                val = j;
            }
        }
        mat[val][3] = temp + mat[val][2];
        mat[val][5] = mat[val][3] - mat[val][1];
        mat[val][4] = mat[val][5] - mat[val][2];
        for(int k=0; k<6; k++)
        {
            swap(mat[val][k], mat[i][k]);
        }
    }
}

int main()
{
    int num, temp;

    cout<<"Enter number of Process: ";
    cin>>num;

    cout<<"...Enter the process ID...\n";
    for(int i=0; i<num; i++)
    {
        cout<<"...Process "<<i+1<<"...\n";
        cout<<"Enter Process Id: ";
        cin>>mat[i][0];
        cout<<"Enter Arrival Time: ";
        cin>>mat[i][1];
        cout<<"Enter Burst Time: ";
        cin>>mat[i][2];
    }

    cout<<"Before Arrange...\n";
    cout<<"Process ID\tArrival Time\tBurst Time\n";
    for(int i=0; i<num; i++)
    {
        cout<<mat[i][0]<<"\t\t"<<mat[i][1]<<"\t\t"<<mat[i][2]<<"\n";
    }

    arrangeArrival(num, mat);
    completionTime(num, mat);
    cout<<"Final Result...\n";
    cout<<"Process ID\tArrival Time\tBurst Time\tWaiting Time\tTurnaround Time\n";
    for(int i=0; i<num; i++)
    {
        cout<<mat[i][0]<<"\t\t"<<mat[i][1]<<"\t\t"<<mat[i][2]<<"\t\t"<<mat[i][4]<<"\t\t"<<mat[i][5]<<"\n";
    }
}
