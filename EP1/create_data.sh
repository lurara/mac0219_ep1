if [ "$#" -ne 1 ]; then
	echo "Por favor, digite um único argumento, na forma:
	        .\create_data.sh <num_testes>"
	exit
fi

if [ $1 -lt 10 ]; then
	echo "O numero minimo de testes é 10"
	NUM_TESTS=10
else
	NUM_TESTS=$1
fi

SIZE=("16"
      "32"
      "64"
      "128"
      "256"
      "512"
      "1024"
      "2048"
      "4096"
      "8192"
)

N_THREADS=("1"
           "2"
	   "4"
	   "8"
	   "16"
	   "32"
)

for S in $SIZE; do
	for T in $N_THREADS; do
		./report_generator.sh $NUM_TESTS $T $S
		mv Reports Data/Reports_T"$T"_S"$S"
	done
done
