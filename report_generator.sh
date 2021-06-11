# export num threads for omp
export OMP_NUM_THREADS=8

# variables

PTH="Reports"
MAIN_RESULTS=general_results
CMP_FILE=compilation_stats
EXE_FILE=execution_stats
PROGRAM=mandelbrot

ETC=(""
     "8"
     ""
)

SFX=("seq"
     "pth"
     "omp"
)

VERSION=("Sequencial"
         "Pthreads"
	 "OpenMP"
)

INPUT_NAME=("Full_Picture"
       "Seahorse_Valley"
       "Elephant_Valley"
       "Triple_Spiral_Valley")

INPUT=("-2.5 1.5 -2.0 2.0 11500"
       "-0.8 -0.7 0.05 0.15 11500"
       "0.175 0.375 -0.1 0.1 11500"
       "-0.188 -0.012 0.554 0.754 11500")

# number of tests
if [ "$#" -eq 0 ] || [ $1 < 10 ]; then
    echo -e "O número mínimo de testes é 10\n"
    NUM_TESTS=10
else
    NUM_TESTS=$1
fi
echo -e "\nRealizando" $NUM_TESTS "testes por input.\n\n"

# verify if it is a new test
if ! mkdir $PTH 2>/dev/null; then
    rm -r -f $PTH
    mkdir $PTH
fi

touch $MAIN_RESULTS
          
# introduce the user the script
echo -e "Este script tem como função a realização de diversos testes a fim de avaliar o desempenho 
do algoritmo >> $PROGRAM << programado nas versões sequenciais, pthreads e OpenMP. 
Todos os resultados, seram encontrados na pasta '$PTH'"

echo -e "\nEntradas:\n"

echo -e '\t nome \t\t\t\t input'
echo '  ------------------------------------------------------------------------------------------'
i=0
while [ $i != ${#INPUT[@]}  ]; do
    if [ $i -eq 0 ]; then
        echo -e '\t' ${INPUT_NAME[$i]} '\t\t\t' ${INPUT[$i]}
    else
        echo -e '\t' ${INPUT_NAME[$i]} '\t\t' ${INPUT[$i]}
    fi
    let "i=i+1"
done

echo -e '\n >>>>> Os resultados gerais podem ser encontrados no arquivo: ' $MAIN_RESULTS '<<<<<'

# create the general tests

# add general information about the computer
echo "Sistema Operacional" > $MAIN_RESULTS
lsb_release -d >> $MAIN_RESULTS
echo "" >> $MAIN_RESULTS

echo "Processador" >> $MAIN_RESULTS
cat /proc/cpuinfo | grep "model name" | head -1 >> $MAIN_RESULTS
echo "" >> $MAIN_RESULTS

echo "RAM" >> $MAIN_RESULTS
cat /proc/meminfo | grep "MemTotal" | head -1 >> $MAIN_RESULTS
echo "" >> $MAIN_RESULTS

echo -e "Compilador: gcc" >> $MAIN_RESULTS
echo "Tempo de compilação gerado pelo comando: time" >> $MAIN_RESULTS
echo "Estatísticas de execução geradas pelo comando: perf" >> $MAIN_RESULTS
echo -e "\n" >> $MAIN_RESULTS

echo -e '  ------------------------------------------------------------------------------------------\n' >> $MAIN_RESULTS

echo -e '\t\t\t\t   ESTATÍSTICAS\n' >> $MAIN_RESULTS

# stats 
i=0
while [ $i != ${#VERSION[@]} ]; do
    echo -e "Versão: " ${VERSION[$i]} '\n' >> $MAIN_RESULTS
    /usr/bin/time -ao $MAIN_RESULTS -f "Compilation time: %U User time (seconds)" make ${SFX[$i]}
    echo "" >> $MAIN_RESULTS
    
    echo -n "Binary file size (bytes): " >> $MAIN_RESULTS
    stat -c %s "$PROGRAM"_"${SFX[$i]}" >> $MAIN_RESULTS
    echo -e "\n" >> $MAIN_RESULTS
  
    mkdir $PTH/${VERSION[$i]}

    j=0
    while [ $j != ${#INPUT[@]} ]; do
	mkdir $PTH/${VERSION[$i]}/${INPUT_NAME[$j]}
        echo "INPUT: "${INPUT_NAME[$j]} >> $MAIN_RESULTS
        perf stat --append -o $MAIN_RESULTS -e cpu-cycles,instructions,cache-misses,cache-references,page-faults -r $NUM_TESTS ./"$PROGRAM"_"${SFX[$i]}" ${INPUT[$j]} ${ETC[$i]} >> $PTH/${VERSION[$i]}/${INPUT_NAME[$j]}/$EXE_FILE
        echo "" >> $MAIN_RESULTS
        mv output.ppm $PTH/${VERSION[$i]}/${INPUT_NAME[$j]}/.
	let "j=j+1"
    done
    echo -e "\n" >> $MAIN_RESULTS
    
    let "i=i+1"
    make 'clean'
done

mv $MAIN_RESULTS $PTH/.

# add specific tests

i=0
DIR=""
while [ $i != ${#VERSION[@]} ]; do
    touch $CMP_FILE
    j=0
    while [ $j != $NUM_TESTS ]; do
        /usr/bin/time -ao $CMP_FILE -f "Compilation time: %U User time (seconds)" make ${SFX[$i]}
        echo -n "Binary file size (bytes): " >> $CMP_FILE
        stat -c %s "$PROGRAM"_"${SFX[$i]}" >> $CMP_FILE
        make clean
        echo "" >> $CMP_FILE
        let "j=j+1"
    done
    
    mv $CMP_FILE $PTH/${VERSION[$i]}/.

    let "i=i+1"
done

echo -e "\n\nFINISHED!"
