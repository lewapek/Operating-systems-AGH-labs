N=2000
STEP=2000
LIM=100000

rm *.points

while [ $N -le $LIM ]; do
	./processes_fork $N fork
	./processes_vfork $N vfork
	./processes_clone_as_fork $N clone_as_fork
	./processes_clone_as_vfork $N clone_as_vfork
	
	N=`expr $N + $STEP`
done

