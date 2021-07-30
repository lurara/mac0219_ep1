import subprocess
import csv
from subprocess import PIPE

def main():
    path_csv = input("Digite o nome do csv: ")
    repeticoes = input("Digite o numero de repeticoes: ")
    n_proc = [2, 4, 8, 16]
    n_thread = [1, 2, 4, 8, 16, 32]
    lists = []
    dif_thread = []
    dif_proc = []
    path = "mpirun -np ./mandelbrot_mpi_pth"

    # Envia
    for proc in n_proc:
        for thread in n_thread:
            output = path.split()
            output.insert(2, str(proc))
            output.append(str(thread))
            #print(output)
            for itr in range(int(repeticoes)):
                result = subprocess.run(output, stdout=PIPE)
                # Transforma seq. de bytes em lista de string
                str_b = (result.stdout).decode('ascii')
                #inputs = list(map(str, str_b.split()))
                lists.append(str_b)

            dif_thread.append(lists)
        dif_proc.append(dif_thread)

    #print(dif_proc)

    # Escreve resultado no csv...
    rows = []
    fields = ['execution', 'n_threads', 'n_process', 'time']
    for i in range(len(n_proc)):
        for j in range(len(n_thread)):
            for q in range(int(repeticoes)):
                row = []
                row.append(str(q+1))
                row.append(str(n_thread[j]))
                row.append(str(n_proc[i]))
                row.append(str(dif_proc[i][j][q]))
                #print(row)
                rows.append(row)

    with open(path_csv, 'w', newline='') as file: 
        writer = csv.writer(file)
        writer.writerow(fields)
        writer.writerows(rows)
        
main()