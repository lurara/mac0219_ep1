import subprocess
import csv
from subprocess import PIPE

def main():
    path_csv = input("Digite o nome do csv: ")
    n_proc = [2, 4, 8, 16]
    n_thread = [1, 2, 4, 8, 16, 32]
    lists = []
    path = "mpirun -np ./mandelbrot_mpi_pth -2.5 1.5 -2.0 2.0 4096"
    cpy_path = path
    output = cpy_path.split()

    output.insert(2, n_proc)
    output.append(n_thread)
    print(output)

    # Envia
    result = subprocess.run(output, stdout=PIPE)

    # Transforma seq. de bytes em lista de string
    str_b = (result.stdout).decode('ascii')
    inputs = list(map(str, str_b.split()))

    #print(inputs)

    for i in range(1):
        lists.append([])

    for i in range(1):
        lists[i].append(inputs)


    print(lists)

    # Escreve resultado no csv...
    """
    with open(csv, 'w', newline='') as file: 
        writer = csv.writer(file)
        writer.writerows(row_list)
    """

main()